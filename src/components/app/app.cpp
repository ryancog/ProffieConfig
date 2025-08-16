#include "app.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
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
#endif
#include <csignal>
#include <cstdio>
#include <filesystem>

#include <wx/snglinst.h>
#include <wx/utils.h>
#include <wx/app.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
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

namespace App {

static wxSingleInstanceChecker singleInstance;

class CrashDialog : public wxDialog {
public:
    CrashDialog(const wxString& error);

private:
    enum {
        ID_OK,
        ID_LOGS
    };
};

} // namespace App

App::CrashDialog::CrashDialog(const wxString& error) : 
    wxDialog(nullptr, wxID_ANY, getAppName() + " Has Crashed", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxCENTER) {

    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    auto *errorMessage{new wxStaticText(this, wxID_ANY, error, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER)};
    sizer->Add(errorMessage, wxSizerFlags(0).Border(wxALL, 10));

    auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *logButton{new wxButton(this, ID_LOGS, "Show Log Folder")};
    buttonSizer->Add(logButton, wxSizerFlags(1).Expand().Border(wxALL, 5));

    auto *okButton{new wxButton(this, ID_OK, "Ok")};
    buttonSizer->Add(okButton, wxSizerFlags(1).Expand().Border(wxRIGHT | wxTOP | wxBOTTOM, 5));

    sizer->Add(buttonSizer, wxSizerFlags(1).Expand());
    SetSizerAndFit(sizer);

    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            Close();
            }, ID_OK);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            wxLaunchDefaultApplication(Paths::logDir().native());
            }, ID_LOGS);
}


void crashHandler(const wxString& error) { 
    auto& logger{Log::Context::getGlobal().createLogger("Crash Handler")};
    logger.error(error.ToStdString());

    if (wxIsMainThread()) {
        auto errDialog{App::CrashDialog(error)};
        errDialog.ShowModal();
    }

    _exit(1);
}

#if defined(__linux__) or defined(__APPLE__)
void sigHandler(int sig, siginfo_t *info, void *) {
#elif defined(__WIN32__)
void sigHandler(int sig) {
#endif

#if defined(__linux__) or defined(__APPLE__)
    std::array<char, 19> errAddr;
    (void)std::snprintf(errAddr.data(), errAddr.size(), "%p", info->si_addr);
    auto errStr{wxString(strsignal(sig)) + " at address: " + wxString{errAddr.data()}};
#elif defined(__WIN32__)
    wxString signame;
    switch (sig) {
        case SIGSEGV:
            signame = "Segmentation fault";
            break;
        case SIGABRT:
            signame = "Function Aborted";
            break;
    }
    auto errStr{signame};
#endif

    crashHandler(errStr);
}

bool App::init(const string& appName, const string& lockName) {
#   if defined(__linux__) or defined(__APPLE__)
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigHandler;
    sigaction(SIGSEGV, &act, nullptr);
    sigaction(SIGABRT, &act, nullptr);
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

void App::exceptionHandler() {
    wxString exceptStr;
    try {
        throw;
    } catch (const std::exception& e) {
        exceptStr = e.what();
    } catch (...) {}

    if (not exceptStr.empty()) {
        exceptStr = "Unhandled Exception: " + exceptStr;
    } else {
        exceptStr = "Unknown Unhandled Exception";
    }

    crashHandler(exceptStr);
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

