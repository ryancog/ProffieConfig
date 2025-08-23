#include "process.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
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
#include <windows.h>
#include <winnt.h>

#if defined(__APPLE__) or defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#elif defined(_WIN32)
#include <namedpipeapi.h>
#include <processthreadsapi.h>
#endif

namespace {

struct InternalData {
    std::promise<Process::Result> promise;
#   if defined(__APPLE__) or defined(__linux__)
    int parentFromChild[2];
    int childFromParent[2];
    __pid_t pid{-1};
#   elif defined(_WIN32)
    HANDLE parentFromChild[2];
    HANDLE childFromParent[2];
    DWORD id;
#   endif
};

std::mutex dataLock;
list<InternalData> internalDatas;

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

void Process::create(const string_view& executable, const span<string>& args) {
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
            pipe(data.childFromParent) == -1 or
            pipe(data.parentFromChild)
       ) {
        data.promise.set_value({.err=Result::CONNECTION_FAILED});
        return;
    }

    auto pid{fork()};
    if (pid == -1) {
        data.promise.set_value({.err=Result::CREATION_FAILED});
        return;
    }

    if (pid == 0) {
        dup2(data.childFromParent[0], STDIN_FILENO);
        dup2(data.parentFromChild[1], STDOUT_FILENO);
        dup2(data.parentFromChild[1], STDERR_FILENO);

        close(data.childFromParent[0]);
        close(data.childFromParent[1]);
        close(data.parentFromChild[0]);
        close(data.parentFromChild[1]);

        char **argv{new char *[args.size() + 2]};
        argv[0] = new char[executable.length() + 1];
        memcpy(argv[0], executable.data(), executable.length());
        argv[0][executable.length()] = 0;

        for (auto idx{0}; idx < args.size(); ++idx) {
            argv[idx + 1] = new char[args[idx].length() + 1];
            memcpy(argv[idx + 1], args[idx].data(), args[idx].length());
            argv[idx + 1][args[idx].length()] = 0;
        }

        argv[args.size() + 1] = nullptr;

        char *exec{new char[executable.length() + 1]};
        memcpy(exec, executable.data(), executable.length());
        exec[executable.length()] = 0;

        execvp(exec, argv);
        data.promise.set_value({.err=Result::EXECUTION_FAILED});
        dataLock.lock();
        for (auto iter{internalDatas.begin()}; iter != internalDatas.end(); ++iter) {
            if (&*iter == mRef) {
                internalDatas.erase(iter);
                break;
            }
        }
        dataLock.unlock();
        exit(1);
    } 

    data.pid = pid;
    close(data.childFromParent[1]);
    close(data.parentFromChild[0]);
#   elif defined(_WIN32)
    SECURITY_ATTRIBUTES pipeAttributes;
    pipeAttributes.nLength = sizeof pipeAttributes;
    pipeAttributes.bInheritHandle = true;
    pipeAttributes.lpSecurityDescriptor = nullptr;

    bool pipeSuccess{true};
    pipeSuccess &= CreatePipe(
        &data.parentFromChild[0],
        &data.parentFromChild[1],
        &pipeAttributes,
        0
    );
    pipeSuccess &= CreatePipe(
        &data.childFromParent[0],
        &data.childFromParent[1],
        &pipeAttributes,
        0
    );

    if (not pipeSuccess) {
        data.promise.set_value({.err=Result::CONNECTION_FAILED});
        return;
    }

    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION procInfo;
    memset(&startupInfo, 0, sizeof startupInfo);
    startupInfo.cb = sizeof startupInfo;
    startupInfo.hStdError = data.parentFromChild[1];
    startupInfo.hStdOutput = data.parentFromChild[1];
    startupInfo.hStdInput = data.childFromParent[0];
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    string execBuffer{executable};
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
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &procInfo
    )};
    if (not procSuccess) {
        data.promise.set_value({.err=Result::CREATION_FAILED});
        return;
    }

    data.id = procInfo.dwProcessId;
    CloseHandle(data.parentFromChild[1]);
    CloseHandle(data.childFromParent[0]);
    CloseHandle(procInfo.hThread);

    std::thread{[id=procInfo.dwProcessId, procHandle=procInfo.hProcess]() {
        WaitForSingleObject(procHandle, INFINITE);
        DWORD exitCode{};
        GetExitCodeProcess(procHandle, &exitCode);

        dataLock.lock();
        for (auto& data : internalDatas) {
            if (data.id != id) continue;

            if (exitCode == 0) {
                data.promise.set_value({.err=Process::Result::SUCCESS});
            } else if (exitCode >= ERROR_SEVERITY_ERROR) {
                data.promise.set_value({.err=Process::Result::CRASHED, .systemResult=exitCode});
            } else {
                data.promise.set_value({.err=Process::Result::EXITED_WITH_FAILURE, .systemResult=exitCode});
            }

            CloseHandle(data.childFromParent[1]);
            CloseHandle(data.parentFromChild[0]);
        }
        dataLock.unlock();
    }}.detach();
#   else
#   error Unsupported
#   endif
}

optional<string> Process::read() {
    assert(mRef);
    InternalData& data{*reinterpret_cast<InternalData *>(mRef)};
#   if defined(__APPLE__) or defined(__linux__)
    int numBytes{};
    if (ioctl(data.parentFromChild[0], FIONREAD, &numBytes) == -1) {
        return nullopt;
    }

    string ret;
    ret.resize(numBytes);
    if (::read(data.parentFromChild[0], ret.data(), ret.size()) == -1) {
        return nullopt;
    }
    return ret;
#   elif defined(_WIN32)
    DWORD numBytes{};
    auto peekResult{PeekNamedPipe(
        data.parentFromChild[0],
        nullptr,
        0,
        nullptr,
        &numBytes,
        nullptr
    )};
    if (not peekResult) {
        return nullopt;
    }

    string ret;
    ret.resize(numBytes);
    auto readResult{ReadFile(
        data.parentFromChild[0],
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

bool Process::write(const string_view& str) {
    assert(mRef);
    InternalData& data{*reinterpret_cast<InternalData *>(mRef)};

#   if defined(__APPLE__) or defined(__linux__)
    return -1 != ::write(data.childFromParent[1], str.data(), str.size());
#   elif defined(_WIN32)
    return WriteFile(
        data.childFromParent[1],
        str.data(),
        str.size(),
        nullptr,
        nullptr
    );
#   endif
}

Process::Result Process::finish() {
    assert(mRef);
    return reinterpret_cast<InternalData *>(mRef)->promise.get_future().get();
}

namespace {
#if defined(__APPLE__) or defined(__linux__)
    
void onChildExit(int sig) {
    assert(sig == SIGCHLD);
    int status{};
    __pid_t pid{};
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (not WIFEXITED(status) and not WIFSIGNALED(status)) continue;

        dataLock.lock();
        for (auto& data : internalDatas) {
            if (data.pid != pid) continue;

            if (WIFEXITED(status)) {
                auto exitStatus{WEXITSTATUS(status)};
                if (exitStatus == 0) {
                    data.promise.set_value({.err=Process::Result::SUCCESS});
                } else {
                    data.promise.set_value({.err=Process::Result::EXITED_WITH_FAILURE, .systemResult=exitStatus});
                }
            } else {
                auto signal{WTERMSIG(status)};
                data.promise.set_value({.err=Process::Result::CRASHED, .systemResult=signal});
            }

            close(data.childFromParent[1]);
            close(data.parentFromChild[0]);
        }
        dataLock.unlock();
    }
}

#endif
} // namespace

