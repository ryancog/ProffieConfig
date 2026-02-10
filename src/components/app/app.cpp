#include "app.h"
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

#if defined(__linux__) or defined(__APPLE__)
#include <array>
#include <execinfo.h>
#include <unistd.h>
#include <dlfcn.h>
#endif
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <ranges>

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

#if defined(__WIN32__) and defined(__WXGTK__)
#include <dwmapi.h>
#endif
#ifdef __WIN32__
#include <wincon.h>
#include <winreg.h>
#endif

#include "log/context.h"
#include "log/logger.h"
#include "utils/paths.h"

namespace {

// TODO: Segfaults on close on GTK
wxSingleInstanceChecker singleInstance;

class CrashDialog : public wxDialog {
public:
    CrashDialog(const wxString& error, const wxString& detail) :
        wxDialog(
            nullptr,
            wxID_ANY,
            App::getAppName() + " Has Crashed",
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxCENTER | wxRESIZE_BORDER
        ) {

        auto *sizer{new wxBoxSizer(wxVERTICAL)};
        auto *errorMessage{new wxStaticText(this, wxID_ANY, error, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER)};
        sizer->Add(errorMessage, wxSizerFlags(0).Border(wxALL, 10));

        if (not detail.IsEmpty()) {
            auto *detailMessage{new wxTextCtrl(this, wxID_ANY, detail, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_DONTWRAP)};
            detailMessage->SetFont(wxFontInfo{}.Family(wxFONTFAMILY_TELETYPE));
            sizer->Add(detailMessage, 1, wxEXPAND);
        }

        auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
        auto *logButton{new wxButton(this, eID_Logs, "Show Log Folder")};
        buttonSizer->Add(logButton, wxSizerFlags(1).Expand().Border(wxALL, 5));

        auto *okButton{new wxButton(this, eID_Ok, "Ok")};
        buttonSizer->Add(okButton, wxSizerFlags(1).Expand().Border(wxRIGHT | wxTOP | wxBOTTOM, 5));

        sizer->Add(buttonSizer, wxSizerFlags().Expand());
        SetSizerAndFit(sizer);

        Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            Close();
        }, eID_Ok);
        Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            wxLaunchDefaultApplication(Paths::logDir().native());
        }, eID_Logs);
    }

private:
    enum {
        eID_Ok,
        eID_Logs
    };
};

void crashHandler(const wxString& error, const wxString& detail) {
    auto& logger{Log::Context::getGlobal().createLogger("Crash Handler")};
    logger.error(error.ToStdString());
    if (not detail.IsEmpty()) logger.error(detail.ToStdString());

    if (wxIsMainThread()) {
        auto errDialog{CrashDialog(error, detail)};
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

void appendStackFrame(void *frame, wxString& str) {
    if (not str.IsEmpty()) str += '\n';
    str += addrToStr(frame);
    str += ": ";

    Dl_info info;
    if (dladdr(frame, &info) == 0 or info.dli_sname == nullptr) {
        str += "???";
    } else {
        str += info.dli_sname;
        str += '+';
        const auto diff{
            reinterpret_cast<size_t>(frame) -
            reinterpret_cast<size_t>(info.dli_saddr)
        };
        str += addrToStr(reinterpret_cast<void *>(diff), 0);
    }
}

#if defined(__linux__) or defined(__APPLE__)
void sigHandler(int sig, siginfo_t *info, void *ucontext) {
    std::array<void *, 50> trace;

    const auto *context{reinterpret_cast<ucontext_t *>(ucontext)};

    wxString detailAppStr{'\n'};
    const auto numFrames{backtrace(trace.data(), trace.size())};
    auto numBeforeBadFrame{0};
    for (; numBeforeBadFrame < numFrames; ++numBeforeBadFrame) {
        Dl_info info;
        if (0 != dladdr(trace[numBeforeBadFrame], &info)) {
            continue;
        }

        const auto programCounter{[context]() {
#           if defined(__x86_64__)
            return context->uc_mcontext->__ss.__rip;
#           elif defined(__i386__)
            return context->uc_mcontext->__ss.__eip;
#           elif defined(__aarch64__)
            return context->uc_mcontext->__ss.__pc;
#           endif
        }()};

        detailAppStr += "Program Counter: ";
        detailAppStr += addrToStr(reinterpret_cast<void *>(programCounter));
        detailAppStr += '\n';

        trace[numBeforeBadFrame] = reinterpret_cast<void *>(programCounter);
        break;
    }

    detailAppStr += "Dropped (Signal?) Frames: ";
    for (auto idx{0}; idx < numBeforeBadFrame; ++idx) {
        appendStackFrame(trace[idx], detailAppStr);
    }

    const auto contextFrames{
        trace |
        std::views::drop(numBeforeBadFrame) |
        std::views::take(numFrames - numBeforeBadFrame)
    };

    wxString errStr{strsignal(sig)};
    errStr += " at address: ";
    errStr += addrToStr(info->si_addr);

    wxString detailStr;
    for (void *const frame : contextFrames) {
        appendStackFrame(frame, detailStr);
    }

    detailStr += '\n';
    detailStr += detailAppStr;
#elif defined(__WIN32__)
void sigHandler(int sig) {
    wxString signame;
    switch (sig) {
        case SIGSEGV:
            signame = "Segmentation fault";
            break;
        case SIGABRT:
            signame = "Function Aborted";
            break;
        case SIGFPE:
            signame = "Illegal Arithmetic";
            break;
        case SIGILL:
            signame = "Illegal Hardware Instruction";
            break;
    }
    auto errStr{signame};
#endif

    crashHandler(errStr, detailStr);
}

} // namespace

bool App::init(const string& appName, const string& lockName) {
#   if defined(__linux__) or defined(__APPLE__)
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigHandler;
    sigaction(SIGSEGV, &act, nullptr);
    sigaction(SIGABRT, &act, nullptr);
    sigaction(SIGFPE, &act, nullptr);
    sigaction(SIGILL, &act, nullptr);
#   elif defined(__WIN32__)
    (void)signal(SIGSEGV, sigHandler);
    (void)signal(SIGABRT, sigHandler);
#   endif
#   ifdef __WIN32__
    if (AttachConsole(ATTACH_PARENT_PROCESS) /* or AllocConsole() */) {
        (void)freopen("CONOUT$", "w", stdout);
        (void)freopen("CONOUT$", "w", stderr);
        (void)freopen("CONIN$", "r", stdin);
    }
#   endif

    auto execStr{wxApp::GetInstance()->argv[0]};
    auto runDir{execStr.substr(0, execStr.find_last_of("/\\"))};
    chdir(runDir.c_str());

    wxApp::GetInstance()->SetAppName(wxString{appName});
    wxApp::GetInstance()->SetAppDisplayName(wxString{appName});

    fs::create_directories(Paths::approot());
    fs::create_directories(Paths::dataDir());
    fs::create_directories(Paths::logDir());
    fs::create_directories(Paths::configDir());
    fs::create_directories(Paths::injectionDir());
    fs::create_directories(Paths::propDir());
    fs::create_directories(Paths::osDir());

    auto& logger {Log::Context::getGlobal().createLogger("App::init()")};
    logger.info("First-stage app initialization complete.");

#   if defined(__WIN32__) and defined(__WXGTK__)
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

    singleInstance.Create(wxString{lockName.empty() ? appName : lockName} + '-' + wxGetUserId());
    if (singleInstance.IsAnotherRunning()) {
        logger.info("We already exist, cancelling initialization.");
        return false;
    }

    wxImage::AddHandler(new wxPNGHandler());

    logger.info("Initialized.");
    return true;
}

void App::appendDefaultMenuItems(wxMenuBar *menuBar) {
#   ifdef __WXOSX__
    menuBar->Append(new wxMenu, _("&Window"));
    menuBar->Append(new wxMenu, _("&Help"));
#   endif
}

#if defined(__WIN32__)
APP_EXPORT [[nodiscard]] bool App::darkMode() { 
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

string App::getAppName() { 
    return wxApp::GetGUIInstance()->GetAppName().ToStdString();
}

