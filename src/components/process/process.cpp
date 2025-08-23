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

#if defined(__APPLE__) or defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#elif defined(__WINDOWS__)
#endif

namespace {

struct InternalData {
    std::promise<Process::Result> promise;
#   if defined(__APPLE__) or defined(__linux__)
    int parentFromChild[2];
    int childFromParent[2];
    __pid_t pid{-1};
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
    auto& data{internalDatas.emplace_back()};
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
#   elif defined(__WINDOWS__)

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
#   elif defined(__WINDOWS__)
#   endif
}

bool Process::write(const string_view& str) {
    assert(mRef);
    InternalData& data{*reinterpret_cast<InternalData *>(mRef)};

#   if defined(__APPLE__) or defined(__linux__)
    return -1 != ::write(data.childFromParent[1], str.data(), str.size());
#   elif defined(__WINDOWS__)
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
                data.promise.set_value({.err=Process::Result::CRASHED});
            }

            close(data.childFromParent[1]);
            close(data.parentFromChild[0]);
        }
        dataLock.unlock();
    }
}

#endif
} // namespace

