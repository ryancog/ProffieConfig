#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * components/utils/paths.h
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

#ifdef _WIN32
#include <errhandlingapi.h>
// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" __attribute__((dllimport)) int CopyFileA(const char *, const char *, int);
#endif

#include <fstream>

#include "version.h"
#include "types.h"

#include "utils_export.h"

namespace Paths {

enum class Executable {
    CURRENT,
    LAUNCHER,
    MAIN,
};

/**
 * @return ~/
 */
[[nodiscard]] UTILS_EXPORT filepath user();

/**
 * Retrieve the path to an executable binary.
 *
 * @param exec The executable to retrieve a path for. Default is `Executable::CURRENT`
 *
 * @return The full executable path (including file)
 */
[[nodiscard]] UTILS_EXPORT filepath executable(Executable exec = Executable::CURRENT);

/**
 * Retrieve the app root path
 *
 * This is the location where core app things are located.
 * A location modifiable without admin rights.
 *
 * @return root path
 */
[[nodiscard]] UTILS_EXPORT filepath approot();

[[nodiscard]] UTILS_EXPORT filepath binaryDir();
[[nodiscard]] UTILS_EXPORT filepath libraryDir();
[[nodiscard]] UTILS_EXPORT filepath componentDir();
[[nodiscard]] UTILS_EXPORT filepath resourceDir();

[[nodiscard]] UTILS_EXPORT filepath dataDir();
[[nodiscard]] UTILS_EXPORT filepath configDir();
[[nodiscard]] UTILS_EXPORT filepath injectionDir();

[[nodiscard]] UTILS_EXPORT filepath versionDir();
[[nodiscard]] UTILS_EXPORT filepath propDir();
[[nodiscard]] UTILS_EXPORT filepath osDir();
[[nodiscard]] UTILS_EXPORT filepath os(const Utils::Version&);

[[nodiscard]] UTILS_EXPORT filepath logDir();

[[nodiscard]] UTILS_EXPORT filepath stateFile();

[[nodiscard]] UTILS_EXPORT string website();
[[nodiscard]] UTILS_EXPORT string remoteAssets();
[[nodiscard]] UTILS_EXPORT string remoteUpdateAssets();

inline bool copyOverwrite(const fs::path& src, const fs::path& dst, std::error_code& err) {
#   ifdef _WIN32
    auto res{CopyFileA(src.string().c_str(), dst.string().c_str(), false)};
    err = {static_cast<int>(GetLastError()), std::system_category()};
    return res;
#   else
    return fs::copy_file(src, dst, fs::copy_options::overwrite_existing, err);
#   endif
}

inline auto openInputFile(const fs::path& path) {
    // POSIX Specifies that there is no distinction between binary/text modes.
    // So this shouldn't make a difference on good operating systems.
    //
    // On Windows, however, MSVC's STL implementation just kills itself because
    // it can't handle its own stupid CRLF endings and tellg/seekg plainly do
    // not work.
    // See: https://github.com/microsoft/STL/issues/1784
    return std::ifstream{path, std::ios::binary | std::ios::in};
}

inline auto openOutputFile(const fs::path& path) {
    return std::ofstream{path, std::ios::binary | std::ios::out};
}

} // namespace Paths

