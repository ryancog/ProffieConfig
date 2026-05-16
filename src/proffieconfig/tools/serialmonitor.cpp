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
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#elif defined(_WIN32)
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
            .rsn_ = Error::Code::Unknown,
            .code_ = errno,
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
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (not isOpen()) {
        return {
            rsn_ = Error::Code::Unknown;
            code_ = GetLastError();
        }
        return err;
    }

    DCB dcbSerialParameters = {};
    dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);

    dcbSerialParameters.BaudRate = CBR_115200;
    dcbSerialParameters.ByteSize = 8;
    dcbSerialParameters.StopBits = ONESTOPBIT;
    dcbSerialParameters.Parity = NOPARITY;
    dcbSerialParameters.fRtsControl = RTS_CONTROL_ENABLE;
    dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

    SetCommState(mSerialHandle, &dcbSerialParameters);
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
    std::lock_guard scopeLock(mMutex);

    if (not isOpen())
        return;

    if (mDevThread.joinable()) {
        mSmphr.release();

        mDevThread.join();
    }

#   if defined(__APPLE__) or defined(__linux__)
    fsync(mFd);
    ::close(mFd);
    mFd = -1;
#   elif defined(_WIN32)
    CloseHandle(mHandle);
    mHandle = INVALID_HANDLE_VALUE;
#   endif

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
        DWORD bytesWritten{};
        auto res{WriteFile(
            mHandle,
            NEWLINE.data(),
            NEWLINE.length(),
            &bytesWritten,
            nullptr
        )};
        if (not res) {
            close();
            return {
                .rsn_=Error::Code::Unknown,
                .code_=GetLastError(),
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

    if (not isOpen()) {
        return {
            .rsn_=Error::Code::Disconnected,
        };
    }

#   if defined(__APPLE__) or defined(__linux__)
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
#   elif defined(_WIN32)
    DWORD bytesRead{};
    auto res{ReadFile(
        mHandle,
        &chr,
        1,
        &bytesRead,
        nullptr
    )};
    if (not res) {
        close();
        return {
            .rsn_=Error::Code::Unknown,
            .code_=GetLastError(),
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

