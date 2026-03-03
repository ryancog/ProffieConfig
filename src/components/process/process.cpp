#include "process.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/process/process.cpp
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
#include <cstring>
#include <future>
#include <list>

#if defined(__APPLE__) or defined(__linux__)
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <sys/poll.h>
#elif defined(_WIN32)
#include <fileapi.h>
#include <handleapi.h>
#include <namedpipeapi.h>
#include <processthreadsapi.h>
#include <windows.h>
#include <winnt.h>
#endif

namespace {

#if defined(__APPLE__)
using PidType = pid_t;
#elif defined(__linux__)
using PidType = __pid_t;
#endif

struct InternalData {
    std::promise<Process::Result> promise_;
#   if defined(__APPLE__) or defined(__linux__)
    int parentFromChild_[2];
    int childFromParent_[2];
    PidType pid_{-1};
#   elif defined(_WIN32)
    HANDLE parentFromChild_[2];
    HANDLE childFromParent_[2];
    DWORD id_;
#   endif
};

std::mutex dataLock;
std::list<InternalData> internalDatas;

#if defined(__APPLE__) or defined(__linux__)
bool sigHandled{false};
void onChildExit(int);
#endif

} // namespace

Process::~Process() {
    if (not mRef) return;

    dataLock.lock();
    for (auto iter{internalDatas.begin()}; iter != internalDatas.end(); ++iter) {
        if (&*iter == mRef) {
            internalDatas.erase(iter);
            break;
        }
    }
    dataLock.unlock();
}

void Process::create(std::string exec, std::span<std::string> args) {
    assert(not mRef);
    dataLock.lock();
    auto& data{internalDatas.emplace_back()};
    dataLock.unlock();
    mRef = &data;

#   if defined(__APPLE__) or defined(__linux__)
    if (not sigHandled) {
        sigHandled = true;
        struct sigaction act{};
        act.sa_flags = SA_NOCLDSTOP;
        act.sa_handler = onChildExit;
        sigaction(SIGCHLD, &act, nullptr);
    }

    if (
            pipe(data.childFromParent_) == -1 or
            pipe(data.parentFromChild_) == -1
       ) {
        data.promise_.set_value({.err_=Result::eConnection_Failed});
        return;
    }

    auto pid{fork()};
    if (pid == -1) {
        data.promise_.set_value({.err_=Result::eCreation_Failed});
        return;
    }

    if (pid == 0) {
        dup2(data.childFromParent_[0], STDIN_FILENO);
        dup2(data.parentFromChild_[1], STDOUT_FILENO);
        dup2(data.parentFromChild_[1], STDERR_FILENO);

        close(data.childFromParent_[1]);
        close(data.parentFromChild_[0]);

        char **argv{new char *[args.size() + 2]};
        argv[0] = new char[exec.length() + 1];
        memcpy(argv[0], exec.data(), exec.length());
        argv[0][exec.length()] = 0;

        for (auto idx{0}; idx < args.size(); ++idx) {
            auto *& argPtr{argv[idx + 1]};
            argPtr = new char[args[idx].length() + 1];
            memcpy(argPtr, args[idx].data(), args[idx].length());
            argPtr[args[idx].length()] = 0;
        }

        argv[args.size() + 1] = nullptr;

        execvp(exec.c_str(), argv);
        data.promise_.set_value({.err_=Result::eExecution_Failed});
        exit(1);
    } 

    data.pid_ = pid;
    close(data.childFromParent_[0]);
    close(data.parentFromChild_[1]);
#   elif defined(_WIN32)
    SECURITY_ATTRIBUTES pipeAttributes;
    pipeAttributes.nLength = sizeof pipeAttributes;
    pipeAttributes.bInheritHandle = true;
    pipeAttributes.lpSecurityDescriptor = nullptr;

    bool pipeSuccess{true};
    pipeSuccess &= CreatePipe(
        &data.parentFromChild_[0],
        &data.parentFromChild_[1],
        &pipeAttributes,
        0
    );
    pipeSuccess &= CreatePipe(
        &data.childFromParent_[0],
        &data.childFromParent_[1],
        &pipeAttributes,
        0
    );

    if (not pipeSuccess) {
        data.promise_.set_value({.err_=Result::eConnection_Failed});
        return;
    }

    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION procInfo;
    memset(&startupInfo, 0, sizeof startupInfo);
    startupInfo.cb = sizeof startupInfo;
    startupInfo.hStdError = data.parentFromChild_[1];
    startupInfo.hStdOutput = data.parentFromChild_[1];
    startupInfo.hStdInput = data.childFromParent_[0];
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    string execBuffer{exec};
    for (const auto& arg : args) {
        execBuffer += ' ';
        execBuffer += arg;
    }

    auto procSuccess{CreateProcessA(
        nullptr,
        execBuffer.data(),
        nullptr,
        nullptr,
        true,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startupInfo,
        &procInfo
    )};
    if (not procSuccess) {
        data.promise_.set_value({
            .err_=Result::eCreation_Failed,
            .systemResult_=GetLastError()
        });
        return;
    }

    data.id_ = procInfo.dwProcessId;
    CloseHandle(data.parentFromChild_[1]);
    CloseHandle(data.childFromParent_[0]);
    CloseHandle(procInfo.hThread);

    std::thread{[&data, id=procInfo.dwProcessId, procHandle=procInfo.hProcess]() {
        WaitForSingleObject(procHandle, INFINITE);
        DWORD exitCode{};
        GetExitCodeProcess(procHandle, &exitCode);

        // Needs to lock so data remains valid
        dataLock.lock();
        if (exitCode == 0) {
            data.promise_.set_value({.err_=Result::eSuccess});
        } else if (exitCode >= ERROR_SEVERITY_ERROR) {
            data.promise_.set_value({
                .err_=Result::eCrashed,
                .systemResult_=exitCode
            });
        } else {
            data.promise_.set_value({
                .err_=Result::eExited_With_Failure,
                .systemResult_=exitCode
            });
        }

        CloseHandle(data.childFromParent_[1]);
        CloseHandle(data.parentFromChild_[0]);
        CloseHandle(procHandle);
        dataLock.unlock();
    }}.detach();
#   else
#   error Unsupported
#   endif
}

std::optional<std::string> Process::read() {
    assert(mRef);
    InternalData& data{*reinterpret_cast<InternalData *>(mRef)};

#   if defined(__APPLE__) or defined(__linux__)
    std::string ret;
    ret.resize(2048);

    while (not false) {
        auto res{::read(data.parentFromChild_[0], ret.data(), ret.size())};
        if (res == -1 and errno == EINTR) continue;
        if (res == -1 or res == 0) return std::nullopt;

        ret.resize(res);
        break;
    }

    return ret;
#   elif defined(_WIN32)
    DWORD numBytes{};
    auto peekResult{PeekNamedPipe(
        data.parentFromChild_[0],
        nullptr,
        0,
        nullptr,
        &numBytes,
        nullptr
    )};
    if (not peekResult) {
        return nullopt;
    }

    std::string ret;
    ret.resize(numBytes);
    auto readResult{ReadFile(
        data.parentFromChild_[0],
        ret.data(),
        ret.size(),
        nullptr,
        nullptr
    )};
    if (not readResult) {
        return nullopt;
    }

    return ret;
#   endif
}

bool Process::write(const std::string_view& str) {
    assert(mRef);
    InternalData& data{*reinterpret_cast<InternalData *>(mRef)};

#   if defined(__APPLE__) or defined(__linux__)
    return -1 != ::write(data.childFromParent_[1], str.data(), str.size());
#   elif defined(_WIN32)
    return WriteFile(
        data.childFromParent_[1],
        str.data(),
        str.size(),
        nullptr,
        nullptr
    );
#   endif
}

Process::Result Process::finish() {
    assert(mRef);
    auto ret{reinterpret_cast<InternalData *>(mRef)->promise_.get_future().get()};
    dataLock.lock();
    for (auto iter{internalDatas.begin()}; iter != internalDatas.end(); ++iter) {
        if (&*iter == mRef) {
            internalDatas.erase(iter);
            break;
        }
    }
    dataLock.unlock();
    return ret;
}

#ifdef _WIN32
Process::Result Process::elevatedProcess(
    cstring exec, const std::span<std::string>& args
) {
    std::string argBuffer{};
    for (auto& arg : args) {
        if (not argBuffer.empty()) argBuffer += ' ';
        argBuffer += arg;
    }

    SHELLEXECUTEINFOA execInfo;
    memset(&execInfo, 0, sizeof execInfo);
    execInfo.cbSize = sizeof execInfo;
    execInfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    execInfo.lpVerb = "runas";
    execInfo.lpFile = exec;
    execInfo.lpParameters = argBuffer.c_str();
    execInfo.nShow = SW_SHOW;

    Result ret;

    if (ShellExecuteExA(&execInfo)) {
        WaitForSingleObject(execInfo.hProcess, INFINITE);
        DWORD exitCode{};
        GetExitCodeProcess(execInfo.hProcess, &exitCode);

        if (exitCode == 0) {
            ret = {.err_=Result::eSuccess};
        } else if (exitCode >= ERROR_SEVERITY_ERROR) {
            ret = {.err_=Result::eCrashed, .systemResult_=exitCode};
        } else {
            ret = {.err_=Result_::eExited_With_Failure, .systemResult_=exitCode};
        }

        CloseHandle(execInfo.hProcess);
    } else {
        ret = {.err_=Result::eCreation_Failed, .systemResult_=GetLastError()};
    }

    return ret;
}
#endif

namespace {
#if defined(__APPLE__) or defined(__linux__)
    
void onChildExit(int sig) {
    assert(sig == SIGCHLD);
    int status{};
    PidType pid{};
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Ugly macro stuff false-positive
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        if (not WIFEXITED(status) and not WIFSIGNALED(status)) continue;

        dataLock.lock();
        for (auto& data : internalDatas) {
            if (data.pid_ != pid) continue;

            if (WIFEXITED(status)) {
                auto exitStatus{WEXITSTATUS(status)};
                if (exitStatus == 0) {
                    data.promise_.set_value({.err_=Process::Result::eSuccess});
                } else {
                    data.promise_.set_value({
                        .err_=Process::Result::eExited_With_Failure,
                        .systemResult_=exitStatus
                    });
                }
            } else {
                auto signal{WTERMSIG(status)};
                data.promise_.set_value({
                    .err_=Process::Result::eCrashed,
                    .systemResult_=signal
                });
            }

            close(data.childFromParent_[1]);
            close(data.parentFromChild_[0]);
        }
        dataLock.unlock();
    }
}

#endif
} // namespace

