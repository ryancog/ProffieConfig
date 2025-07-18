#include "routines.h" 
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include <app/app.h>
#include <utils/types.h>
#include <paths/paths.h>
#include <ui/message.h>
#include <log/logger.h>

#ifdef __WIN32__
#include <fstream>
#include <shlobj.h>
#include <windows.h>
#include <errhandlingapi.h>
#endif

#ifdef __APPLE__
#include <pwd.h>
#endif

namespace Routine {

#ifdef __WIN32__
constexpr auto SUB_KEY{LR"(Software\Microsoft\Windows\CurrentVersion\Uninstall\ProffieConfig)"};
#endif


} // namespace Routine



void Routine::launch(Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Routine::launch()")};
    logger.info("Launching ProffieConfig...");
    auto exec{Paths::executable(Paths::Executable::MAIN)};
#   ifdef __WIN32__
    if (0 == wxExecute(exec.native())) {
        logger.warn("ProffieConfig main binary missing/failed to start.");
    }
#   elif defined(__APPLE__) or defined(__linux__)
    auto str{exec.native()};
    std::array<char *, 2> argv{ str.data(), nullptr };
    execvp(argv[0], argv.data());
    logger.error("ProffieConfig main binary missing/failed to start.");
    exit(1);
#   endif
}

void Routine::platformInstall(Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Routine::platformInstall()")};

    std::error_code err;
    auto currentExec{Paths::executable()};
    auto installedExec{Paths::executable(Paths::Executable::LAUNCHER)};
    if (fs::exists(installedExec, err)) {
        logger.info("Launcher seems to already be installed, removing...");
#       ifdef __APPLE__
        fs::remove_all(installedExec.parent_path().parent_path().parent_path());
#       else
        fs::remove(installedExec);
#       endif
    }

#   ifndef __APPLE__
    logger.debug("Creating " + installedExec.string());
    fs::create_directories(installedExec.parent_path());
#   endif

#   ifdef __WIN32__
    logger.info("Moving launcher into install location...");
    if (not MoveFileW(currentExec.c_str(), installedExec.c_str())) {
        err.assign(static_cast<int32>(GetLastError()), std::system_category());
        if (err) {
            auto errMessage{"Failed to install launcher: " + err.message() + " (" + std::to_string(err.value()) + ')'};
            logger.info(errMessage);
            PCUI::showMessage(_("Failed to install launcher"), App::getAppName());
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
            persistFile->Save((wstring{rawStr} + L"/ProffieConfig.lnk").c_str(), true);
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
    setValue("URLInfoAbout", Paths::website());
    setValue("UninstallString", '"' + installedExec.string() + "\" uninstall");

    RegCloseKey(hKey);

    auto self{Paths::executable().string()};
    constexpr cstring SELFDELETE_BATCH{"C:\\TEMP\\PCFLDel.bat"};
    std::ofstream batch{SELFDELETE_BATCH};
    batch << "@echo off\n:Repeat\ndel \"" << self << "\"\nif exist \"" << self << "\" goto Repeat\ndel \"%~f0\"\n";
    batch.close();
    ShellExecuteA(nullptr, "open", SELFDELETE_BATCH, nullptr, nullptr, SW_HIDE);
#   elif defined(__linux__)
    // move
    wxCopyFile(currentExec.c_str(), installedExec.c_str());
    (void)remove(currentExec.c_str());
#   elif defined(__APPLE__)
    const auto currentBundle{currentExec.parent_path().parent_path().parent_path()};
    const auto applicationPath{installedExec.parent_path().parent_path().parent_path()};
    fs::copy(currentBundle, applicationPath, fs::copy_options::recursive);
    // Cannot remove from DMG
    fs::remove_all(currentBundle, err);
#   endif

    PCUI::showMessage(_("Launcher has been installed."), App::getAppName());
#   ifdef __WIN32__
    if (wxExecute(installedExec.c_str()) == 0) {
        logger.warn("Failed to start launcher.");
    }
    wxExit();
#   else
    auto str{installedExec.native()};
    const decltype(str.data()) argv[2]{
        str.data(),
        nullptr,
    };
    execvp(argv[0], argv);
    logger.warn("Failed to start launcher.");
#   endif
}

void Routine::platformUninstall() {
#   ifdef __WIN32__
    // Remove start menu shortcut
    LPWSTR rawStr{nullptr};
    SHGetKnownFolderPath(FOLDERID_Programs, KF_FLAG_CREATE, nullptr, &rawStr);
    std::error_code err;
    fs::remove(wstring{rawStr} + L"/ProffieConfig.lnk", err);

    // Remove uninstall registry key
    RegDeleteKeyW(HKEY_CURRENT_USER, SUB_KEY);
#   endif
}

