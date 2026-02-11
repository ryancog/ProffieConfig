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

#include <system_error>
#include <wx/stdpaths.h>

#if defined(_WIN32)
#include <shlobj.h>
#elif defined(__linux__) or defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#endif

#include "utils/types.h"
#include "utils/version.h"

std::error_code paths::init() {
    std::error_code ec;
    fs::create_directories(paths::approot(), ec);
    fs::create_directories(paths::dataDir(), ec);
    fs::create_directories(paths::configDir(), ec);
    fs::create_directories(paths::injectionDir(), ec);
    fs::create_directories(paths::propDir(), ec);
    fs::create_directories(paths::osDir(), ec);
    return ec;
}

filepath paths::user() {
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

filepath paths::approot() {
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

filepath paths::executable(Executable exec) {
    switch (exec) {
        case Executable::Current:
            return wxStandardPaths::Get().GetExecutablePath().ToStdWstring();
        case Executable::Launcher:
#           ifdef _WIN32
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
        case Executable::Main:
#           ifdef _WIN32
            return binaryDir() / "ProffieConfig.exe";
#           elif defined(__linux__)
            return binaryDir() / "ProffieConfig";
#           else
            return binaryDir() / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
#           endif
    }
    return {};
}

filepath paths::binaryDir() { return approot() / "bin"; }

filepath paths::libraryDir() {
#   ifdef _WIN32
    return binaryDir();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "lib";
#   endif
}

filepath paths::componentDir() {
#   ifdef _WIN32
    return binaryDir();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "components";
#   endif
}

filepath paths::resourceDir() { return approot() / "resources"; }

filepath paths::logDir() {
#   if defined(_WIN32)
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

filepath paths::dataDir() {
#   ifdef _WIN32
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

filepath paths::configDir() { return paths::dataDir() / "configs"; }

filepath paths::injectionDir() { return paths::dataDir() / "injections"; }

filepath paths::versionDir() { return paths::dataDir() / "versions"; }

filepath paths::propDir() { return paths::versionDir() / "props"; }

filepath paths::osDir() {
    return paths::versionDir() / "os";
}

filepath paths::os(const Utils::Version& version) {
    return paths::osDir() / static_cast<string>(version) / "ProffieOS";
}

filepath paths::stateFile() { return paths::dataDir() / ".state.pconf"; }


string paths::website() { return "https://proffieconfig.kafrenetrading.com"; }

string paths::remoteAssets() {
    return website() + "/assets/appsupport";
}

string paths::remoteUpdateAssets() { 
    return remoteAssets() + "/update";
}

