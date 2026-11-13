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

#if __APPLE__ or __linux__
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <sys/poll.h>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <namedpipeapi.h>
#include <shellapi.h>
#include <winnt.h>
#endif

namespace {

#if __APPLE__
using PidType = pid_t;
#elif __linux__
using PidType = __pid_t;
#endif

struct InternalData {
    std::promise<Process::Result> promise_;
#   if __APPLE__ or __linux__
    int parent_from_child_[2];
    int child_from_parent_[2];
    PidType pid_{-1};
#   elif _WIN32
    HANDLE parent_from_child_[2];
    HANDLE child_from_parent_[2];
    HANDLE handle_;
    DWORD id_;
#   endif
};

std::mutex data_lock;
std::list<InternalData> internal_datas;

#if __APPLE__ or __linux__
bool sig_handled{false};
void on_child_exit(int);
#endif

#if _WIN32
std::string create_quoted(std::string);
#endif

} // namespace

Process::~Process() {
    if (not m_ref)
        return;

    std::lock_guard lk(data_lock);
    for (
            auto iter{internal_datas.begin()};
            iter != internal_datas.end();
            ++iter
        ) {
        if (&*iter == m_ref) {
            internal_datas.erase(iter);
            break;
        }
    }
}

void Process::create(std::string exec, std::span<const std::string> args) {
    assert(not m_ref);

    { std::lock_guard lk(data_lock);
        m_ref = &internal_datas.emplace_back();
    }
    auto& data{*reinterpret_cast<InternalData *>(m_ref)};

#   if __APPLE__ or __linux__
    if (not sig_handled) {
        sig_handled = true;
        struct sigaction act{};
        act.sa_flags = SA_NOCLDSTOP;
        act.sa_handler = on_child_exit;
        sigaction(SIGCHLD, &act, nullptr);
    }

    if (
            pipe(data.child_from_parent_) == -1 or
            pipe(data.parent_from_child_) == -1
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
        dup2(data.child_from_parent_[0], STDIN_FILENO);
        dup2(data.parent_from_child_[1], STDOUT_FILENO);
        dup2(data.parent_from_child_[1], STDERR_FILENO);

        close(data.child_from_parent_[1]);
        close(data.parent_from_child_[0]);

        char **argv{new char *[args.size() + 2]};
        argv[0] = new char[exec.length() + 1];
        memcpy(argv[0], exec.data(), exec.length());
        argv[0][exec.length()] = 0;

        for (auto idx{0}; idx < args.size(); ++idx) {
            auto *& arg_ptr{argv[idx + 1]};
            arg_ptr = new char[args[idx].length() + 1];
            memcpy(arg_ptr, args[idx].data(), args[idx].length());
            arg_ptr[args[idx].length()] = 0;
        }

        argv[args.size() + 1] = nullptr;

        execvp(exec.c_str(), argv);

        // This doesn't make it back to the parent, the child has its own
        // memory space, duh.
        // data.promise_.set_value({.err_=Result::eExecution_Failed});

        // Don't do normal cleanup, there's all sorts of stuff from the parent
        // polluting things.
        _exit(1);
    } 

    data.pid_ = pid;
    close(data.child_from_parent_[0]);
    close(data.parent_from_child_[1]);
#   elif _WIN32
    SECURITY_ATTRIBUTES pipe_attributes;
    pipe_attributes.nLength = sizeof pipe_attributes;
    pipe_attributes.bInheritHandle = true;
    pipe_attributes.lpSecurityDescriptor = nullptr;

    bool pipe_success{true};
    pipe_success &= static_cast<bool>(CreatePipe(
        &data.parent_from_child_[0],
        &data.parent_from_child_[1],
        &pipe_attributes,
        0
    ));
    pipe_success &= static_cast<bool>(CreatePipe(
        &data.child_from_parent_[0],
        &data.child_from_parent_[1],
        &pipe_attributes,
        0
    ));

    if (not pipe_success) {
        data.promise_.set_value({.err_=Result::eConnection_Failed});
        return;
    }

    STARTUPINFOA startup_info;
    PROCESS_INFORMATION proc_info;
    memset(&startup_info, 0, sizeof startup_info);
    startup_info.cb = sizeof startup_info;
    startup_info.hStdError = data.parent_from_child_[1];
    startup_info.hStdOutput = data.parent_from_child_[1];
    startup_info.hStdInput = data.child_from_parent_[0];
    startup_info.dwFlags |= STARTF_USESTDHANDLES;

    std::string command_line{};
    command_line = create_quoted(std::move(exec));
    for (const auto& arg : args) {
        command_line += ' ';
        command_line += create_quoted(arg);
    }

    auto proc_success{CreateProcessA(
        nullptr,
        command_line.data(),
        nullptr,
        nullptr,
        true,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startup_info,
        &proc_info
    )};
    if (not proc_success) {
        data.promise_.set_value({
            .err_=Result::eCreation_Failed,
            .system_result_=GetLastError()
        });
        return;
    }

    data.id_ = proc_info.dwProcessId;
    data.handle_ = proc_info.hThread;
    CloseHandle(data.parent_from_child_[1]);
    CloseHandle(data.child_from_parent_[0]);

    std::thread{[&data, proc_handle=proc_info.hProcess]() {
        WaitForSingleObject(proc_handle, INFINITE);
        DWORD exit_code{};
        GetExitCodeProcess(proc_handle, &exit_code);

        // Needs to lock so data remains valid
        std::lock_guard lk(data_lock);

        if (exit_code == 0) {
            data.promise_.set_value({.err_=Result::eSuccess});
        } else if (exit_code >= ERROR_SEVERITY_ERROR) {
            data.promise_.set_value({
                .err_=Result::eCrashed,
                .system_result_=exit_code
            });
        } else {
            data.promise_.set_value({
                .err_=Result::eExited_With_Failure,
                .system_result_=exit_code
            });
        }

        CloseHandle(proc_handle);
    }}.detach();
#   else
#   error Unsupported
#   endif
}

std::optional<std::string> Process::read() {
    assert(m_ref);
    InternalData& data{*reinterpret_cast<InternalData *>(m_ref)};

#   if __APPLE__ or __linux__
    std::string ret;
    ret.resize(2048);

    while (not false) {
        auto res{::read(data.parent_from_child_[0], ret.data(), ret.size())};
        if (res == -1 and errno == EINTR)
            continue;

        if (res == -1 or res == 0)
            return std::nullopt;

        ret.resize(res);
        break;
    }

    return ret;
#   elif _WIN32
    DWORD num_bytes{};
    auto peek_result{PeekNamedPipe(
        data.parent_from_child_[0],
        nullptr,
        0,
        nullptr,
        &num_bytes,
        nullptr
    )};
    if (not peek_result)
        return std::nullopt;

    std::string ret;
    ret.resize(num_bytes);
    auto read_result{ReadFile(
        data.parent_from_child_[0],
        ret.data(),
        ret.size(),
        nullptr,
        nullptr
    )};
    if (not read_result)
        return std::nullopt;

    return ret;
#   endif
}

bool Process::write(const std::string_view& str) {
    assert(m_ref);
    InternalData& data{*reinterpret_cast<InternalData *>(m_ref)};

#   if __APPLE__ or __linux__
    return -1 != ::write(data.child_from_parent_[1], str.data(), str.size());
#   elif _WIN32
    return WriteFile(
        data.child_from_parent_[1],
        str.data(),
        str.size(),
        nullptr,
        nullptr
    );
#   endif
}

Process::Result Process::finish() {
    assert(m_ref);

    auto& data{*reinterpret_cast<InternalData *>(m_ref)};
    auto ret{data.promise_.get_future().get()};

    std::lock_guard lk(data_lock);
    for (
            auto iter{internal_datas.begin()};
            iter != internal_datas.end();
            ++iter
        ) {
        if (&*iter == m_ref) {
#           if __APPLE__ or __linux__
            close(data.child_from_parent_[1]);
            close(data.parent_from_child_[0]);
#           elif _WIN32
            CloseHandle(data.child_from_parent_[1]);
            CloseHandle(data.parent_from_child_[0]);
            CloseHandle(data.handle_);
#           endif

            internal_datas.erase(iter);
            break;
        }
    }

    return ret;
}

void Process::interrupt() {
    assert(m_ref);

    auto& data{*reinterpret_cast<InternalData *>(m_ref)};

#if _WIN32
    if (AttachConsole(data.id_)) {
        SetConsoleCtrlHandler(nullptr, true);

        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        FreeConsole();

        if (WAIT_OBJECT_0 != WaitForSingleObject(data.handle_, 2'000)) {
            // If the process hasn't exited yet, forcefully terminate it.
            TerminateProcess(data.handle_, 1);
        }
    
        // Need a little extra wait to make sure, apparently?
        // Without this, this process will receive the event and die sometimes.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        SetConsoleCtrlHandler(nullptr, false);
    }
#else
    kill(data.pid_, SIGINT);
#endif

    finish();
}

#if _WIN32
Process::Result Process::elevated_process(
    cstring exec, const std::span<std::string>& args
) {
    std::string arg_buffer{};
    for (auto& arg : args) {
        if (not arg_buffer.empty())
            arg_buffer += ' ';

        arg_buffer += create_quoted(arg);
    }

    SHELLEXECUTEINFOA exec_info;
    std::memset(&exec_info, 0, sizeof exec_info);
    exec_info.cbSize = sizeof exec_info;
    exec_info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    exec_info.lpVerb = "runas";
    exec_info.lpFile = exec;
    exec_info.lpParameters = arg_buffer.c_str();
    exec_info.nShow = SW_SHOW;

    Result ret;

    if (ShellExecuteExA(&exec_info)) {
        WaitForSingleObject(exec_info.hProcess, INFINITE);
        DWORD exit_code{};
        GetExitCodeProcess(exec_info.hProcess, &exit_code);

        if (exit_code == 0) {
            ret = {.err_=Result::eSuccess};
        } else if (exit_code >= ERROR_SEVERITY_ERROR) {
            ret = {
                .err_=Result::eCrashed,
                .system_result_=exit_code
            };
        } else {
            ret = {
                .err_=Result::eExited_With_Failure,
                .system_result_=exit_code
            };
        }

        CloseHandle(exec_info.hProcess);
    } else {
        ret = {
            .err_=Result::eCreation_Failed,
            .system_result_=GetLastError()
        };
    }

    return ret;
}
#endif

namespace {

#if __APPLE__ or __linux__
void on_child_exit(int sig) {
    assert(sig == SIGCHLD);
    int status{};
    PidType pid{};
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Ugly macro stuff false-positive
        // NOLINTNEXTLINE(readability-simplify-boolean-expr)
        if (not WIFEXITED(status) and not WIFSIGNALED(status))
            continue;

        std::lock_guard lk(data_lock);
        for (auto& data : internal_datas) {
            if (data.pid_ != pid)
                continue;

            if (WIFEXITED(status)) {
                auto exit_status{WEXITSTATUS(status)};
                if (exit_status == 0) {
                    data.promise_.set_value({.err_=Process::Result::eSuccess});
                } else {
                    data.promise_.set_value({
                        .err_=Process::Result::eExited_With_Failure,
                        .system_result_=exit_status
                    });
                }
            } else {
                auto signal{WTERMSIG(status)};
                data.promise_.set_value({
                    .err_=Process::Result::eCrashed,
                    .system_result_=signal
                });
            }
        }
    }
}
#endif

#if _WIN32
std::string create_quoted(std::string str) {
    // As other people have pointed out, it feels a little silly this isn't a
    // function provided by Microsoft's libraries.
    // Function derived from procedure explained here:
    // https://web.archive.org/web/20161228144344/https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/

    for (auto iter{str.begin()}; iter != str.end(); ++iter) {
        // First, count contiguous backslashes (if any), since they might
        // require special handling.
        size num_backslashes{};
        while (iter != str.end() and *iter == '\\') {
            ++num_backslashes;
            ++iter;
        }

        // If backslashes are at the end of the string, they need to be
        // duplicated to be escaped. (otherwise they'll interfere with the
        // terminating quotation mark).
        if (iter == str.end()) {
            str.insert(str.end(), num_backslashes, '\\');
            break;
        }

        // Quotes need to be escaped, and also the consumed backslashes need
        // to be, otherwise there'll be misinterpretation because of the quote.
        if (*iter == '"') {
            str.insert(iter, num_backslashes + 1, '\\');
            continue;
        }

        // Anything else doesn't need modification.
    }

    // Wrap the whole thing in the enclosing quotes.
    str.insert(str.begin(), '"');
    str.insert(str.end(), '"');

    return str;
}
#endif

} // namespace

