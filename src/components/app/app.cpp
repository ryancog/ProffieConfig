#include "app.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/app/app.cpp
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

#include <csignal>
#include <cstdio>
#include <ranges>
#include <span>
#include <string>

#if defined(__linux__) or defined(__APPLE__)
#include <array>
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>
#endif

#if defined(__linux__)
#include <sys/ucontext.h>
#endif

#if defined(_WIN32) and defined(__WXGTK__)
#include <dwmapi.h>
#endif

#ifdef _WIN32
#include <wincon.h>
#include <winreg.h>
#include <dbghelp.h>
#include <backtrace.h>
#endif

#include <wx/snglinst.h>
#include <wx/utils.h>
#include <wx/app.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/uilocale.h>
#include <wx/log.h>
#include <wx/stdpaths.h>

#include "app/critical_dialog.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"
#include "utils/demangle.hpp"

namespace {

constexpr auto NUM_STACK_FRAMES{64};

// TODO: Segfaults on close on GTK
wxSingleInstanceChecker singleInstance;
app::ShowMessageFunc *showMessage;

// Win32 assumes/requires the char array comes immediately after the
// SYMBOL_INFO. This structure formalizes that.
struct SymbolInfo {
    SymbolInfo() {
#       if defined(_WIN32)
        memset(&mInfo, 0, sizeof(SYMBOL_INFO));
        // The extra byte for null term is in the SYMBOL_INFO struct
        mInfo.MaxNameLen = mName.size();
        mInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
#       endif
    }

#   if defined(_WIN32)
    [[nodiscard]] SYMBOL_INFO *raw() { return &mInfo; }
#   elif defined(__APPLE__) or defined(__linux__)
    [[nodiscard]] Dl_info *raw() { return &mInfo; }
#   endif

    [[nodiscard]] cstring name() const { 
#       if defined(_WIN32)
        return mInfo.NameLen ? mInfo.Name : nullptr;
#       elif defined(__APPLE__) or defined(__linux__)
        return mInfo.dli_sname;
#       endif
    };

    [[nodiscard]] void *addr() const {
#       if defined(_WIN32)
        return reinterpret_cast<void *>(mInfo.Address);
#       elif defined(__APPLE__) or defined(__linux__)
        return mInfo.dli_saddr;
#       endif
    };

    [[nodiscard]] cstring filename() const {
#       if defined(_WIN32)
        (void)this;
        return nullptr;
#       elif defined(__APPLE__) or defined(__linux__)
        return mInfo.dli_fname;
#       endif
    }

    [[nodiscard]] void *fileAddr() const {
#       if defined(_WIN32)
        (void)this;
        return nullptr;
#       elif defined(__APPLE__) or defined(__linux__)
        return mInfo.dli_fbase;
#       endif
    }

private:
#   if defined(_WIN32)
    SYMBOL_INFO mInfo;
    array<char, 255> mName;
#   elif defined(__APPLE__) or defined(__linux__)
    Dl_info mInfo;
#   endif
};

void crashHandler(const wxString& error, const wxString& detail) {
    auto& logger{logging::Context::getGlobal().createLogger("Crash Handler")};
    logger.error(error.ToStdString());
    if (not detail.IsEmpty()) logger.error(detail.ToStdString());

    if (wxIsMainThread()) {
        auto errDialog{app::CriticalDialog(error, detail)};
        errDialog.ShowModal();
    }

    _exit(1);
}

cstring addrToStr(
    void *addr, int width = sizeof(size_t) * 2
) {
    thread_local static std::array<char, (sizeof(size_t) * 2) + 3> errAddr;
    (void)std::snprintf(
        errAddr.data(),
        errAddr.size(),
        "0x%0*zx",
        width, reinterpret_cast<size_t>(addr)); return errAddr.data();
};

void fillSymbolInfo(void *frame, SymbolInfo& info) {
#if defined(__linux__) or defined(__APPLE__)
    if (dladdr(frame, info.raw()) == 0) {
        return;
    } 
#elif defined(_WIN32)
    const auto errCB{[&](cstring /* msg */, int /* errnum */) {
        info.raw()->NameLen = 0;
        info.raw()->Address = 0;
    }};
    const auto errCBWrapper{[](void *arg, cstring msg, int errnum) {
        const auto& cb{*reinterpret_cast<decltype(errCB) *>(arg)};
        cb(msg, errnum);
    }};

    const auto symInfoCB{[&](
        uintptr_t,
        cstring symname,
        uintptr_t symval,
        uintptr_t /* symsize */
    ) {
        if (symname) {
            info.raw()->NameLen = strnlen(symname, 0xFF);
            memcpy(info.raw()->Name, symname, info.raw()->NameLen + 1);
        } else {
            info.raw()->NameLen = 0;
        }

        info.raw()->Address = symval;
    }};
    const auto symInfoCBWrapper{[](
        void *arg,
        uintptr_t pc,
        cstring symname,
        uintptr_t symval,
        uintptr_t symsize
    ) {
        const auto& cb{(*reinterpret_cast<decltype(symInfoCB) *>(arg))};
        cb(pc, symname, symval, symsize);
    }};
    
    HANDLE proc{GetCurrentProcess()};
    static bool initRes{[proc]() {
        return static_cast<bool>(SymInitialize(proc, nullptr, true));
    }()};

    int res{};
    res = SymFromAddr(
        proc,
        reinterpret_cast<DWORD64>(frame),
        nullptr,
        info.raw()
    );

    if (res) return;

    static backtrace_state *state{backtrace_create_state(
        nullptr,
        true,
        errCBWrapper,
        const_cast<void *>(reinterpret_cast<const void *>(&errCB))
    )};

    res = backtrace_syminfo(
        state,
        reinterpret_cast<uintptr_t>(frame),
        symInfoCBWrapper,
        nullptr,
        const_cast<void *>(reinterpret_cast<const void *>(&symInfoCB))
    );

    if (not res) {
        info.raw()->NameLen = 0;
        info.raw()->Address = 0;
    }
#endif
}

wxString generateBacktrace(void *pc, std::span<void *> frames) {
    SymbolInfo info;
    wxString framesStr;
    wxString droppedFramesStr;
    bool inSigFrames{true};
    for (auto iter{frames.begin()}; iter != frames.end(); ++iter) {
        auto *const frame{*iter};

        fillSymbolInfo(frame, info);

#       if defined(__APPLE__) and defined(__x86_64__)
        auto& str{inSigFrames ? droppedFramesStr : framesStr};

        (void)pc; // Suppress the unused diagnostic
        if (inSigFrames and 0 == strcmp(info.name(), "_sigtramp")) {
            // For reasons beyond what I care to understand, the frame above
            // sigtramp always seems to be corrupted/invalid.
            //
            // Note that incrementing the `iter` here means it's invalid below.
            // There's not a reason to use it below anyways, and it shouldn't
            // be, but I note this anyways.
            ++iter;
            if (iter == frames.end()) break;
            inSigFrames = false;
        }
#       else
        if (inSigFrames and frame == pc) {
            inSigFrames = false;
        }

        auto& str{inSigFrames ? droppedFramesStr : framesStr};
#       endif

        if (not str.IsEmpty()) str += '\n';
        str += addrToStr(frame);
        str += ": ";
        str += info.name() ? utils::demangle(info.name()) : "???";
        str += '+';
        const auto diff{
            reinterpret_cast<ptrdiff_t>(frame) -
            reinterpret_cast<ptrdiff_t>(info.addr())
        };
        str += addrToStr(reinterpret_cast<void *>(diff), 0);
        if (info.filename() and info.fileAddr()) {
            str += " (";
            str += info.filename();
            str += '+';
            const auto diff{
                reinterpret_cast<ptrdiff_t>(frame) -
                reinterpret_cast<ptrdiff_t>(info.fileAddr())
            };
            str += addrToStr(reinterpret_cast<void *>(diff), 0);
            str += ')';
        }
    }
        
    return
        framesStr + "\n\n" +
        "Dropped (Signal?) Frames:\n" +
        droppedFramesStr;
}

#if defined(__linux__) or defined(__APPLE__)
void sigHandler(int sig, siginfo_t *info, void *ucontext) {
    const auto *context{reinterpret_cast<ucontext_t *>(ucontext)};

    wxString detailAppStr{'\n'};
    void *const programCounter{reinterpret_cast<void *>([context]() {
#       if defined(__linux__)
#       if defined(__x86_64__)
        return context->uc_mcontext.gregs[REG_RIP];
#       endif
#       elif defined(__APPLE__)
#       if defined(__x86_64__)
        return context->uc_mcontext->__ss.__rip;
#       elif defined(__i386__)
        return context->uc_mcontext->__ss.__eip;
#       elif defined(__aarch64__)
        return context->uc_mcontext->__ss.__pc;
#       endif
#       endif
    }())};

    wxString errStr{strsignal(sig)};
    errStr += " at address: ";
    errStr += addrToStr(info->si_addr);

    std::array<void *, NUM_STACK_FRAMES> frames;
    const auto numFrames{backtrace(frames.data(), frames.size())};
    const auto backtraceStr{generateBacktrace(
        programCounter, frames | std::views::take(numFrames)
    )};

    crashHandler(errStr, backtraceStr);
}
#elif defined(_WIN32)
WINAPI LONG exceptionFilter(LPEXCEPTION_POINTERS exception) {
    wxString signame;
    switch (exception->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_IN_PAGE_ERROR:
            signame = "Access Violation at ";
            signame += addrToStr(reinterpret_cast<void *>(
                exception->ExceptionRecord->ExceptionInformation[1]
            ));
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            signame = "Access Violation";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            signame = "Stack Overflow";
        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_UNDERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
        case EXCEPTION_INT_OVERFLOW:
            signame = "Illegal Arithmetic";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_PRIV_INSTRUCTION:
            signame = "Illegal Hardware Instruction";
            break;
        default: return true;
    }
    auto errStr{signame};

    array<void *, NUM_STACK_FRAMES> frames;
    const auto numFrames{CaptureStackBackTrace(
        0,
        frames.size(),
        frames.data(),
        nullptr
    )};
    void *const programCounter{exception->ExceptionRecord->ExceptionAddress};
    const auto backtraceStr{generateBacktrace(
        programCounter, frames | std::views::take(numFrames)
    )};

    crashHandler(errStr, backtraceStr);
    return false;
}
#endif

} // namespace

bool app::setupExclusion(const std::string& lockName) {
    singleInstance.Create(wxString{lockName} + '-' + wxGetUserId());
    if (singleInstance.IsAnotherRunning()) {
        auto res{showMessage(
            _("It looks like ProffieConfig is already running, continuing may break things!"),
            lockName,
            wxOK | wxCANCEL | wxCANCEL_DEFAULT,
            nullptr
        )};
        if (res == wxCANCEL) return false;
    }

    return true;
}

bool app::init() {
    auto& logger{logging::Context::getGlobal().createLogger("app::init()")};

#   ifdef _WIN32
    // Must be done before setting control event handlers
    if (AttachConsole(ATTACH_PARENT_PROCESS) /* or AllocConsole() */) {
        (void)freopen("CONOUT$", "w", stdout);
        (void)freopen("CONOUT$", "w", stderr);
        (void)freopen("CONIN$", "r", stdin);
    }
#   endif

#   if defined(__linux__) or defined(__APPLE__)
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigHandler;
    if (
            -1 == sigaction(SIGSEGV, &act, nullptr) or
            -1 == sigaction(SIGABRT, &act, nullptr) or
            -1 == sigaction(SIGFPE, &act, nullptr) or
            -1 == sigaction(SIGILL, &act, nullptr)
       ) {
        logger.warn("Signal handlers could not be registered: " + std::to_string(errno));
    }
#   elif defined(_WIN32)
    SetUnhandledExceptionFilter(exceptionFilter);
#   endif

    auto execStr{wxApp::GetInstance()->argv[0]};
    auto runDir{execStr.substr(0, execStr.find_last_of("/\\"))};
    if (-1 == chdir(runDir.c_str())) {
        logger.warn("Dir could not be changed: " + std::to_string(errno));
        return false;
    }

    logger.info("First-stage app initialization complete.");

#   if defined(_WIN32) and defined(__WXGTK__)
    SetEnvironmentVariableA("GTK_CSD", "0");
    constexpr cstring DARK_THEME{"Adwaita:dark"};
    constexpr cstring LIGHT_THEME{"Adwaita"};
    SetEnvironmentVariableA("GTK_THEME", darkMode() ? DARK_THEME : LIGHT_THEME);
    logger.info("WinGTK Theme Loaded");
#   endif
#   ifdef __WXGTK__
    wxApp::GTKSuppressDiagnostics();
    logger.info("GTK? Shh");
#   endif
#   ifdef __WXMSW__
    static_cast<wxApp *>(wxApp::GetGUIInstance())->MSWEnableDarkMode();
    logger.info("MSW Dark Mode Enabled");
#   endif

    if (not wxUILocale::UseDefault()) {
        logger.warn("Failed to use system default locale.");
    }

    auto *translations{new wxTranslations};
    wxTranslations::Set(translations);
    for (const auto& lang : translations->GetAvailableTranslations("proffieconfig")) {
        logger.info("Found language: " + lang.ToStdString());
    }
    logger.info("System language: " + std::to_string(wxUILocale::GetSystemLanguage()));

    if (not translations->AddStdCatalog()) {
        logger.info("Standard catalog not loaded.");
    }

    if (not translations->AddCatalog("proffieconfig")) {
        logger.warn("Translation catalog not loaded.");
    }
    logger.info("Translation loading complete.");

    wxImage::AddHandler(new wxPNGHandler());

    logger.info("Initialized.");
    return true;
}

#if defined(_WIN32)
APP_EXPORT [[nodiscard]] bool app::darkMode() { 
    DWORD buffer{};
    DWORD bufferSize{sizeof(buffer)};
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD,
        nullptr,
        &buffer,
        &bufferSize
    );
    return ~buffer;
}
#endif

void app::setName(const wxString& appName) {
    wxApp::GetInstance()->SetAppName(appName);
    wxApp::GetInstance()->SetAppDisplayName(appName);
}

wxString app::getName() { 
    return wxApp::GetGUIInstance()->GetAppName();
}

void app::provideUI(
    int32 showMessage(const wxString&, const wxString&, long, wxWindow *)
) {
    ::showMessage = showMessage;
}

