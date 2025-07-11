#include "paths.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/paths/paths.cpp
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
            return binaries() / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
#           endif
    }
    return {};
}

filepath Paths::binaries() { return approot() / "bin"; }

filepath Paths::libraries() {
#   ifdef __WIN32__
    return binaries();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "lib";
#   endif
}

filepath Paths::components() {
#   ifdef __WIN32__
    return binaries();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "components";
#   endif
}

filepath Paths::resources() { return approot() / "resources"; }

filepath Paths::logs() {
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

filepath Paths::data() {
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

filepath Paths::configs() { return Paths::data() / "configs"; }
filepath Paths::injections() { return Paths::data() / "injections"; }
filepath Paths::props() { return Paths::data() / "props"; }
filepath Paths::proffieos() { return Paths::data() / "ProffieOS"; }

string Paths::website() { return "https://proffieconfig.kafrenetrading.com"; }

string Paths::remoteAssets() {
    return website() + "/assets/appsupport";
}

string Paths::remoteUpdateAssets() { 
    return remoteAssets() + "/update";
}
