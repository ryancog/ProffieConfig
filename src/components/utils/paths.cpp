#include "paths.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
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

#include <string>

#include <system_error>
#include <wx/stdpaths.h>

#if defined(_WIN32)
#include <array>
#include <shlobj.h>
#elif defined(__linux__) or defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#endif

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

fs::path paths::user() {
#   ifdef _WIN32
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &rawStr);
    std::array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return shortPath.data();
#   elif defined(__APPLE__) or defined(__linux__)
    return getpwuid(getuid())->pw_dir;
#   endif
}

fs::path paths::approot() {
#   ifdef APP_DEPLOY_PATH 
    return APP_DEPLOY_PATH;
#   else
#   ifdef _WIN32
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    std::array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return fs::path{shortPath.data()} /  "ProffieConfig";
#   elif defined(__APPLE__)
    return fs::path(getpwuid(getuid())->pw_dir) / "Library" / "Application Support" / "ProffieConfig";
#   elif defined(__linux__)
    return fs::path(getpwuid(getuid())->pw_dir) / ".local" / "share" / "ProffieConfig";
#   endif
#   endif
}

fs::path paths::executable(Executable exec) {
    switch (exec) {
        case Executable::Current:
            return wxStandardPaths::Get().GetExecutablePath().ToStdWstring();
        case Executable::Launcher:
#           ifdef _WIN32
            {
                LPWSTR rawStr{};
                auto rawPathRes{SHGetKnownFolderPath(FOLDERID_UserProgramFiles, KF_FLAG_CREATE, nullptr, &rawStr)};
                assert(rawPathRes == S_OK);

                std::array<wchar_t, MAX_PATH> shortPath;
                auto shortPathRes{GetShortPathNameW(rawStr, shortPath.data(), shortPath.size())};
                assert(shortPathRes != 0);

                CoTaskMemFree(rawStr);
                return fs::path{shortPath.data()} / "ProffieConfig.exe";
            }
#           elif defined(__linux__)
            return fs::path(getpwuid(getuid())->pw_dir) / ".proffieconfig" / "ProffieConfig";
#           elif defined(__APPLE__)
            return fs::path(getpwuid(getuid())->pw_dir) / "Applications" / "ProffieConfig.app" / "Contents" / "MacOS" / "ProffieConfig";
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

fs::path paths::binaryDir() { return approot() / "bin"; }

fs::path paths::libraryDir() {
#   ifdef _WIN32
    return binaryDir();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "lib";
#   endif
}

fs::path paths::componentDir() {
#   ifdef _WIN32
    return binaryDir();
#   elif defined(__linux__) or defined(__APPLE__)
    return approot() / "components";
#   endif
}

fs::path paths::resourceDir() { return approot() / "resources"; }

fs::path paths::logDir() {
#   if defined(_WIN32)
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    std::array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return fs::path{shortPath.data()} / "ProffieConfig";
#   elif defined(__linux__)
    return dataDir() / "logs";
#   elif defined(__APPLE__)
    return std::string{getpwuid(getuid())->pw_dir} + "/Library/Logs/ProffieConfig";
#   endif
}

fs::path paths::dataDir() {
#   ifdef _WIN32
    PWSTR rawStr{};
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &rawStr);
    array<wchar_t, MAX_PATH> shortPath;
    GetShortPathNameW(rawStr, shortPath.data(), shortPath.size());
    CoTaskMemFree(rawStr);
    return fs::path{shortPath.data()} / "ProffieConfig";
#   elif defined(__APPLE__)
    return approot();
#   elif defined(__linux__)
    return approot();
#   endif
}

fs::path paths::configDir() { return paths::dataDir() / "configs"; }

fs::path paths::injectionDir() { return paths::dataDir() / "injections"; }

fs::path paths::versionDir() { return paths::dataDir() / "versions"; }

fs::path paths::propDir() { return paths::versionDir() / "props"; }

fs::path paths::osDir() {
    return paths::versionDir() / "os";
}

fs::path paths::stateFile() { return paths::dataDir() / ".state.pconf"; }

std::string paths::website() { return "https://proffieconfig.kafrenetrading.com"; }

std::string paths::remoteAssets() {
    return website() + "/assets/appsupport";
}

std::string paths::remoteUpdateAssets() { 
    return remoteAssets() + "/update";
}

