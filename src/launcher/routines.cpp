#include "routines.hpp" 
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * launcher/routines.cpp
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

#include <filesystem>

#ifdef _WIN32
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <errhandlingapi.h>

#include <wx/app.h>

#include "utils/files.hpp"
#endif

#ifdef __APPLE__
#include <pwd.h>
#endif

#include "app/app.hpp"
#include "app/critical_dialog.hpp"
#include "ui/dialogs/message.hpp"
#include "utils/paths.hpp"
#include "log/logger.hpp"

namespace {

#ifdef _WIN32
constexpr auto SUB_KEY{LR"(Software\Microsoft\Windows\CurrentVersion\Uninstall\ProffieConfig)"};
#endif

} // namespace

void routine::launch(logging::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Routine::launch()")};

    // Make sure to not interfere with the new process.
    app::releaseExclusion();

    auto exec{paths::executable(paths::Executable::Main)};
    auto execString{exec.string()};
    logger.info("Launching ProffieConfig (" + execString + ")...");

#   ifdef _WIN32
    PROCESS_INFORMATION procInfo;
    STARTUPINFOA startupInfo;
    memset(&startupInfo, 0, sizeof startupInfo);
    startupInfo.cb = sizeof startupInfo;

    auto procSuccess{CreateProcessA(
        nullptr,
        execString.data(),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &procInfo
    )};
    if (not procSuccess) {
        logger.warn("Failed to launch ProffieConfig: " + std::to_string(GetLastError()));

        app::CriticalDialog dlg(_("Failed to launch ProffieConfig"));
        dlg.ShowModal();
    }

    wxExit();
#   elif defined(__APPLE__) or defined(__linux__)
    std::array<char *, 2> argv{ execString.data(), nullptr };
    execvp(argv[0], argv.data());

    logger.warn("Failed to launch ProffieConfig: " + std::to_string(errno));

    app::CriticalDialog dlg(_("Failed to launch ProffieConfig"));
    dlg.ShowModal();

    exit(1);
#   endif
}

void routine::platformInstall(logging::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Routine::platformInstall()")};

    std::error_code ec;
    auto currentExec{paths::executable()};
    auto installedExec{paths::executable(paths::Executable::Launcher)};
    if (fs::exists(installedExec, ec)) {
        logger.info("Launcher seems to already be installed, removing...");
#       ifdef __APPLE__
        fs::remove_all(installedExec.parent_path().parent_path().parent_path(), ec);
#       else
        fs::remove(installedExec, ec);
#       endif
    }

    logger.debug("Creating " + installedExec.string());
    fs::create_directories(installedExec.parent_path(), ec);
    if (ec) {
        auto errMessage{"Failed to prepare directories for installation: " + ec.message() + " (" + std::to_string(ec.value()) + ')'};
        logger.info(errMessage);
        pcui::showMessage(_("Failed to install launcher"));
        return;
    }

#   ifdef _WIN32
    logger.info("Moving launcher into install location...");
    if (not CopyFileW(currentExec.c_str(), installedExec.c_str(), true)) {
        ec.assign(static_cast<int32>(GetLastError()), std::system_category());
        if (ec) {
            auto errMessage{"Failed to install launcher: " + ec.message() + " (" + std::to_string(ec.value()) + ')'};
            logger.info(errMessage);
            pcui::showMessage(_("Failed to install launcher"));
            return;
        }
    }

    logger.info("Creating Start Menu shortcut...");
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IShellLinkW *shellLink{nullptr};

    HRESULT hres{};
    hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&shellLink);
    if (SUCCEEDED(hres)) {
        shellLink->SetPath(installedExec.c_str());
        shellLink->SetDescription(L"All-In-One Proffie Management Utility");

        IPersistFile *persistFile{nullptr};
        hres = shellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&persistFile);
        if (SUCCEEDED(hres)) {
            LPWSTR rawStr{nullptr};
            SHGetKnownFolderPath(FOLDERID_Programs, KF_FLAG_CREATE, nullptr, &rawStr);
            persistFile->Save((std::wstring{rawStr} + L"/ProffieConfig.lnk").c_str(), true);
            CoTaskMemFree(rawStr);
            persistFile->Release();
        } 
        shellLink->Release();
    } 
    CoUninitialize();

    logger.info("Creating uninstall registry entry...");
    HKEY hKey{nullptr};
    RegCreateKeyExW(HKEY_CURRENT_USER, SUB_KEY, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);

    auto setValue{[hKey](cstring key, const std::string& value) {
        RegSetValueExA(hKey, key, 0, REG_SZ, reinterpret_cast<const BYTE *>(value.c_str()), value.length());
    }};

    setValue("DisplayName", "ProffieConfig");
    setValue("Publisher", "Kafrene Trading");
    setValue("DisplayVersion", wxSTRINGIZE(BIN_VERSION));
    setValue("DisplayIcon", installedExec.string());
    setValue("URLInfoAbout", paths::website());
    setValue("UninstallString", '"' + installedExec.string() + "\" uninstall");

    RegCloseKey(hKey);
#   elif defined(__linux__)
    wxCopyFile(currentExec.c_str(), installedExec.c_str());
#   elif defined(__APPLE__)
    const auto currentBundle{currentExec.parent_path().parent_path().parent_path()};
    const auto applicationPath{installedExec.parent_path().parent_path().parent_path()};
    fs::copy(currentBundle, applicationPath, fs::copy_options::recursive, ec);
    if (ec) {
        logger.info("Couldn't copy " + currentBundle.string() + " to " + applicationPath.string() + ": " + ec.message());
        pcui::showMessage(_("Failed to install launcher"));
        return;
    }
#   endif

    pcui::showMessage(_("Launcher has been installed."));

    app::releaseExclusion();

#   ifdef _WIN32
    PROCESS_INFORMATION procInfo;
    STARTUPINFOA startupInfo;
    memset(&startupInfo, 0, sizeof startupInfo);
    startupInfo.cb = sizeof startupInfo;

    auto procSuccess{CreateProcessA(
        nullptr,
        installedExec.string().data(),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &procInfo
    )};
    if (procSuccess) {
        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);
    } else {
        logger.warn("Failed to start launcher: " + std::to_string(GetLastError()));

        app::CriticalDialog dlg(_("Failed to start Launcher"));
        dlg.ShowModal();
    }
#   else
    auto str{installedExec.string()};
    const decltype(str.data()) argv[2]{
        str.data(),
        nullptr,
    };
    execvp(argv[0], argv);

    logger.warn("Failed to start launcher: " + std::to_string(errno));

    app::CriticalDialog dlg(_("Failed to start Launcher"));
    dlg.ShowModal();
#   endif

    // This function doesn't handle exiting.
}

void routine::platformUninstall() {
#   ifdef _WIN32
    // Remove start menu shortcut
    LPWSTR rawStr{nullptr};
    SHGetKnownFolderPath(FOLDERID_Programs, KF_FLAG_CREATE, nullptr, &rawStr);
    std::error_code err;
    fs::remove(std::wstring{rawStr} + L"/ProffieConfig.lnk", err);

    // Remove uninstall registry key
    RegDeleteKeyW(HKEY_CURRENT_USER, SUB_KEY);
#   endif
}

