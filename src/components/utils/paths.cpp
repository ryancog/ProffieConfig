#include "paths.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * components/utils/paths.cpp
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
#include <iostream>

#include <wx/stdpaths.h>

#if defined(__WIN32__)
#include <shlobj.h>
#elif defined(__linux__) or defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#endif

#include "utils/types.h"

namespace Paths {} // namespace Paths

filepath Paths::approot() {
    std::error_code err;
#   ifdef __WIN32__
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<TCHAR, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return filepath{shortPath.data()} /  "ProffieConfig";
#   elif defined(__APPLE__)
    return filepath(getpwuid(getuid())->pw_dir) / "Library" / "Application Support" / "ProffieConfig";
#   elif defined(__linux__)
    return filepath(getpwuid(getuid())->pw_dir) / ".proffieconfig";
#   endif
}

filepath Paths::executable(Executable exec) {
    switch (exec) {
        case Executable::CURRENT:
            return wxStandardPaths::Get().GetExecutablePath().ToStdWstring();
        case Executable::LAUNCHER:
#           ifdef __WIN32__
            {
                LPWSTR rawStr{};
                auto res{SHGetKnownFolderPath(FOLDERID_UserProgramFiles, KF_FLAG_CREATE, nullptr, &rawStr)};
                if (res != S_OK) {
                    throw std::runtime_error{"Failed getting program files: " + std::to_string(res)};
                }
                array<TCHAR, MAX_PATH> shortPath;
                if (0 == GetShortPathNameW(rawStr, shortPath.data(), shortPath.size())) {
                    throw std::runtime_error{"Failed getting shortname: " + std::to_string(GetLastError())};
                }
                CoTaskMemFree(rawStr);
                std::wcout << shortPath.data() << std::endl;
                return filepath{shortPath.data()} / "ProffieConfig.exe";
            }
#           elif defined(__linux__)
            return filepath(getpwuid(getuid())->pw_dir) / ".proffieconfig" / "ProffieConfig";
#           elif defined(__APPLE__)
            return filepath(getpwuid(getuid())->pw_dir) / "Applications" / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
#           endif
        case Executable::MAIN:
#           ifdef __WIN32__
            return binaries() / "ProffieConfig.exe";
#           elif defined(__linux__)
            return binaries() / "ProffieConfig";
#           else
            return binaryDir() / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
#           endif
    }
    return {};
}

filepath Paths::binaryDir() { return approot() / "bin"; }

filepath Paths::libraryDir() {
#   ifdef __WIN32__
    return binaries();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "lib";
#   endif
}

filepath Paths::componentDir() {
#   ifdef __WIN32__
    return binaries();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "components";
#   endif
}

filepath Paths::resourceDir() { return approot() / "resources"; }

filepath Paths::logDir() {
#   if defined(__WIN32__)
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<TCHAR, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return filepath{shortPath.data()} / "ProffieConfig";
#   elif defined(__linux__)
    return data() / "logs";
#   elif defined(__APPLE__)
    return string{getpwuid(getuid())->pw_dir} + "/Library/Logs/ProffieConfig";
#   endif
}

filepath Paths::dataDir() {
#   ifdef __WIN32__
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<TCHAR, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return filepath{shortPath.data()} / "ProffieConfig";
#   elif defined(__APPLE__)
    return approot();
#   elif defined(__linux__)
    return filepath(getpwuid(getuid())->pw_dir) / ".local" / "share" / "ProffieConfig";
#   endif
}

filepath Paths::configDir() { return Paths::dataDir() / "configs"; }

filepath Paths::injectionDir() { return Paths::dataDir() / "injections"; }

filepath Paths::versionDir() { return Paths::dataDir() / "versions"; }

filepath Paths::propDir() { return Paths::versionDir() / "props"; }

std::pair<filepath, filepath> Paths::prop(const string& name) {
    const auto base{Paths::propDir() / name};
    return { base / "info.pconf", base / "header.h" };
}

filepath Paths::osDir() {
    return Paths::versionDir() / "os";
}

filepath Paths::os(const Utils::Version& version) {
    return Paths::osDir() / static_cast<string>(version) / "ProffieOS";
}

filepath Paths::stateFile() { return Paths::dataDir() / ".state.pconf"; }


string Paths::website() { return "https://proffieconfig.kafrenetrading.com"; }

string Paths::remoteAssets() {
    return website() + "/assets/appsupport";
}

string Paths::remoteUpdateAssets() { 
    return remoteAssets() + "/update";
}
