#include "serialmonitor.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/tools/serialmonitor.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <mutex>

#if defined(__APPLE__) or defined(__linux__)
#include <cstring>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#else
#error Unsupported
#endif

SerialMonitor::SerialMonitor() = default;

SerialMonitor::~SerialMonitor() {
    close();
}

bool SerialMonitor::isOpen() const {
    std::lock_guard scopeLock(mMutex);
#if defined(__APPLE__) or defined(__linux__)
    return mFd >= 0;
#elif defined(_WIN32)
    return mHandle != INVALID_HANDLE_VALUE;
#   endif
}

void SerialMonitor::setOnDisconnect(std::function<void()> func) {
    std::lock_guard scopeLock(mMutex);
    // Can only be set prior to opening.
    assert(not isOpen());
    mOnDisconnect = std::move(func);
}
Error SerialMonitor::open(const std::string& path) {
    std::lock_guard scopeLock(mMutex);
    assert(not isOpen());

#   if defined(__APPLE__) or defined(__linux__)
    struct termios newtio;

    mFd = ::open(path.c_str(), O_RDWR | O_NOCTTY);
    if (not isOpen()) {
        return {
            .rsn_=Error::Code::Unknown,
            .code_=errno,
        };
    }

    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = (tcflag_t) NULL;
    newtio.c_lflag &= ~ICANON; /* unset canonical */
    newtio.c_cc[VTIME] = 0; // No wait for multiple bytes
    newtio.c_cc[VMIN] = 1; // Only wait for one byte

    tcflush(mFd, TCIFLUSH);
    tcsetattr(mFd, TCSANOW, &newtio);
#   elif defined(_WIN32)
    // See https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file#win32-device-namespaces
    const auto safeBoardPath{R"(\\.\)" + path};
    mHandle = CreateFileA(
        safeBoardPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        nullptr
    );
    if (not isOpen()) {
        return {
            .rsn_=Error::Code::Unknown,
            .code_=static_cast<int32>(GetLastError()),
        };
    }

    DCB dcbSerialParameters = {};
    dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);

    dcbSerialParameters.BaudRate = CBR_115200;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = NOPARITY;
    dcbSerialParameters.fRtsControl = RTS_CONTROL_ENABLE;
    dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

    SetCommState(mHandle, &dcbSerialParameters);

    // Read starts signaled for synchronization
    mReadEvent = CreateEventA(nullptr, true, true, nullptr);
    if (mReadEvent == nullptr) {
        CloseHandle(mHandle);
        mHandle = INVALID_HANDLE_VALUE;

        return {
            .rsn_=Error::Code::Unknown,
            .code_=static_cast<int32>(GetLastError()),
        };
    }

    mWriteEvent = CreateEventA(nullptr, true, false, nullptr);
    if (mWriteEvent == nullptr) {
        CloseHandle(mReadEvent);
        CloseHandle(mHandle);
        mHandle = INVALID_HANDLE_VALUE;

        return {
            .rsn_=Error::Code::Unknown,
            .code_=static_cast<int32>(GetLastError()),
        };
    }
#   endif

    // Only bother setting up this loop if a handler is registered, otherwise
    // problems will be caught by read/write.
    if (mOnDisconnect) {
        mDevThread = std::thread{[this] {
            devLoop();
        }};
    }

    return {};
}

void SerialMonitor::close() {
    { std::lock_guard scopeLock(mMutex);

        if (not isOpen())
            return;

        if (mDevThread.joinable()) {
            mSmphr.release();

            // If close is being called from the dev thread, it'll acquire the
            // semaphore once returning from here and should exit just fine.
            //
            // Can't join() here otherwise it'll deadlock, but joinable()
            // needs to become false, so detach().
            if (std::this_thread::get_id() == mDevThread.get_id())
                mDevThread.detach();
            else
                mDevThread.join();
        }

#       if __APPLE__ or __linux__
        fsync(mFd);
        ::close(mFd);
        mFd = -1;
#       elif _WIN32
        CloseHandle(mReadEvent);
        CloseHandle(mWriteEvent);

        CloseHandle(mHandle);
        mHandle = INVALID_HANDLE_VALUE;
#       endif
    }

    // Call outside of lock
    if (mOnDisconnect)
        mOnDisconnect();
}

Error SerialMonitor::write(std::string_view msg) {
    std::lock_guard scopeLock(mMutex);

    if (not isOpen()) {
        return {
            .rsn_=Error::Code::Disconnected,
        };
    }

    const auto doWrite{[this](std::string_view str) -> Error {
#       if defined(__APPLE__) or defined(__linux__)
        auto res{::write(
            mFd,
            str.data(),
            str.length()
        )};
        if (res == -1) {
            close();
            return {
                .rsn_=Error::Code::Unknown,
                .code_=errno,
            };
        }
#       elif defined(_WIN32)
        BOOL res{};
        DWORD bytesWritten{};
        OVERLAPPED overlapped;
        memset(&overlapped, 0, sizeof overlapped);
        overlapped.hEvent = mWriteEvent;

        res = WriteFile(
            mHandle,
            str.data(),
            str.length(),
            &bytesWritten,
            &overlapped
        );
        if (res)
            // Completed
            return {};

        auto err{GetLastError()};
        if (err != ERROR_IO_PENDING) {
            close();
            return {
                .rsn_=Error::Code::Unknown,
                .code_=static_cast<int32>(err),
            };
        }

        res = GetOverlappedResult(
            mHandle,
            &overlapped,
            &bytesWritten,
            true
        );

        if (not res) {
            return {
                .rsn_=Error::Code::Unknown,
                .code_=static_cast<int32>(GetLastError()),
            };
        }
#       endif

        return {};
    }};

    constexpr std::string_view NEWLINE{"\r\n"};

    if (auto err{doWrite(NEWLINE)})
        return err;

    if (auto err{doWrite(msg)})
        return err;

    if (auto err{doWrite(NEWLINE)})
        return err;

    return {};
}

Error SerialMonitor::read(char& chr) {
    // Don't lock here.

#   if _WIN32
    // On Linux it's kind of okay to access mHandle outside of the lock, since
    // if it's invalid (because closed or explicitly set invalid), the read()
    // will fail.
    //
    // Here though, the mHandle is read twice, so I want to make sure the value
    // is stored. In both cases accessing the HANDLE or FD runs the risk of it
    // being closed then re-opened (especially on Linux) afaik...
    HANDLE handle{};
    DWORD bytesRead{};
    OVERLAPPED overlapped;
    BOOL res{};

    while (not false) {
        std::lock_guard scopeLock(mMutex);
        if (not isOpen()) {
            return {
                .rsn_=Error::Code::Disconnected,
            };
        }

        // Wait for any previous read to have finished.
        auto wait{WaitForSingleObject(mReadEvent, 50)};
        if (wait != WAIT_OBJECT_0)
            continue;

        ResetEvent(mReadEvent);

        // Start Read in lock to ensure there's not a race after the above
        // wait.
        memset(&overlapped, 0, sizeof overlapped);
        overlapped.hEvent = mReadEvent;
        res = ReadFile(
            mHandle,
            &chr,
            1,
            &bytesRead,
            &overlapped
        );
        if (res)
            // Completed
            return {};

        auto err{GetLastError()};
        if (err != ERROR_IO_PENDING) {
            close();
            return {
                .rsn_=Error::Code::Unknown,
                .code_=static_cast<int32>(err),
            };
        }

        handle = mHandle;
        break;
    }

    res = GetOverlappedResult(
        mHandle,
        &overlapped,
        &bytesRead,
        true
    );

    if (not res) {
        return {
            .rsn_=Error::Code::Unknown,
            .code_=static_cast<int32>(GetLastError()),
        };
    }
#   elif __APPLE__ or __linux__
    if (not isOpen()) {
        return {
            .rsn_=Error::Code::Disconnected,
        };
    }

    pollfd pfd{.fd=mFd, .events=POLLIN};
    while (not false) {
        // This needs to be a poll, specifically with a timeout... I don't
        // really understand why.
        //
        // Just read() or a poll w/ timeout=-1 will cause things to hang for
        // at least some amount of time.
        auto res{poll(&pfd, 1, 50)};
        if (res == -1) {
            return {
                .rsn_=Error::Code::Unknown,
                    // .code_=errno,
            };
        }

        if (res == 1)
            break;
    };

    if (pfd.revents & (POLLHUP | POLLERR)) {
        close();
        return {
            .rsn_=Error::Code::Unknown,
            // .code_=errno,
        };
    }

    auto res{::read(mFd, &chr, 1)};
    if (res == -1) {
        close();
        return {
            .rsn_=Error::Code::Unknown,
            .code_=errno
        };
    }
#   endif

    return {};
}

void SerialMonitor::devLoop() {
    while (not false) {
#       if defined(__APPLE__) or defined(__linux__)
        pollfd pfd{
            .fd=mFd,
            .events=POLLERR | POLLHUP,
        };

        if (0 < poll(&pfd, 1, 0)) {
#       elif defined(_WIN32)
        if (not ClearCommError(mHandle, nullptr, nullptr)) {
#       endif
            close();
        }

        if (mSmphr.try_acquire_for(std::chrono::seconds(1)))
            break;
    }
}

