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

#include <wx/stdpaths.h>

#if defined(_WIN32)
#include <shlobj.h>
#elif defined(__linux__) or defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#endif

#include "utils/types.h"
#include "utils/version.h"

namespace Paths {} // namespace Paths

filepath Paths::user() {
#   ifdef _WIN32
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &rawStr);
    array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return shortPath.data();
#   elif defined(__APPLE__) or defined(__linux__)
    return getpwuid(getuid())->pw_dir;
#   endif
}

filepath Paths::approot() {
#   ifdef APP_DEPLOY_PATH 
    return APP_DEPLOY_PATH;
#   else
#   ifdef _WIN32
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return filepath{shortPath.data()} /  "ProffieConfig";
#   elif defined(__APPLE__)
    return filepath(getpwuid(getuid())->pw_dir) / "Library" / "Application Support" / "ProffieConfig";
#   elif defined(__linux__)
    return filepath(getpwuid(getuid())->pw_dir) / ".local" / "share" / "ProffieConfig";
#   endif
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
                auto rawPathRes{SHGetKnownFolderPath(FOLDERID_UserProgramFiles, KF_FLAG_CREATE, nullptr, &rawStr)};
                assert(rawPathRes == S_OK);

                array<wchar_t, MAX_PATH> shortPath;
                auto shortPathRes{GetShortPathNameW(rawStr, shortPath.data(), shortPath.size())};
                assert(shortPathRes != 0);

                CoTaskMemFree(rawStr);
                return filepath{shortPath.data()} / "ProffieConfig.exe";
            }
#           elif defined(__linux__)
            return filepath(getpwuid(getuid())->pw_dir) / ".proffieconfig" / "ProffieConfig";
#           elif defined(__APPLE__)
            return filepath(getpwuid(getuid())->pw_dir) / "Applications" / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
#           endif
        case Executable::MAIN:
#           ifdef __WIN32__
            return binaryDir() / "ProffieConfig.exe";
#           elif defined(__linux__)
            return binaryDir() / "ProffieConfig";
#           else
            return binaryDir() / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
#           endif
    }
    return {};
}

filepath Paths::binaryDir() { return approot() / "bin"; }

filepath Paths::libraryDir() {
#   ifdef __WIN32__
    return binaryDir();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "lib";
#   endif
}

filepath Paths::componentDir() {
#   ifdef __WIN32__
    return binaryDir();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "components";
#   endif
}

filepath Paths::resourceDir() { return approot() / "resources"; }

filepath Paths::logDir() {
#   if defined(__WIN32__)
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return filepath{shortPath.data()} / "ProffieConfig";
#   elif defined(__linux__)
    return dataDir() / "logs";
#   elif defined(__APPLE__)
    return string{getpwuid(getuid())->pw_dir} + "/Library/Logs/ProffieConfig";
#   endif
}

filepath Paths::dataDir() {
#   ifdef __WIN32__
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return filepath{shortPath.data()} / "ProffieConfig";
#   elif defined(__APPLE__)
    return approot();
#   elif defined(__linux__)
    return approot();
#   endif
}

filepath Paths::configDir() { return Paths::dataDir() / "configs"; }

filepath Paths::injectionDir() { return Paths::dataDir() / "injections"; }

filepath Paths::versionDir() { return Paths::dataDir() / "versions"; }

filepath Paths::propDir() { return Paths::versionDir() / "props"; }

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
