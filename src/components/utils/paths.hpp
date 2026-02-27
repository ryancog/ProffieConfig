#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/paths.hpp
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
#include <string>

#include "utils_export.h"

namespace fs = std::filesystem;

namespace paths {

[[nodiscard]] UTILS_EXPORT std::error_code init();

enum class Executable {
    Current,
    Launcher,
    Main,
};

/**
 * @return ~/
 */
[[nodiscard]] UTILS_EXPORT fs::path user();

/**
 * Retrieve the path to an executable binary.
 *
 * @param exec The executable to retrieve a path for.
 *
 * @return The full executable path (including file)
 */
[[nodiscard]] UTILS_EXPORT fs::path executable(Executable exec = Executable::Current);

/**
 * Retrieve the app root path
 *
 * This is the location where core app things are located.
 * A location modifiable without admin rights.
 *
 * @return root path
 */
[[nodiscard]] UTILS_EXPORT fs::path approot();

[[nodiscard]] UTILS_EXPORT fs::path binaryDir();
[[nodiscard]] UTILS_EXPORT fs::path libraryDir();
[[nodiscard]] UTILS_EXPORT fs::path componentDir();
[[nodiscard]] UTILS_EXPORT fs::path resourceDir();

[[nodiscard]] UTILS_EXPORT fs::path dataDir();
[[nodiscard]] UTILS_EXPORT fs::path configDir();
[[nodiscard]] UTILS_EXPORT fs::path injectionDir();

[[nodiscard]] UTILS_EXPORT fs::path versionDir();
[[nodiscard]] UTILS_EXPORT fs::path propDir();
[[nodiscard]] UTILS_EXPORT fs::path osDir();

[[nodiscard]] UTILS_EXPORT fs::path logDir();

[[nodiscard]] UTILS_EXPORT fs::path stateFile();

[[nodiscard]] UTILS_EXPORT std::string website();
[[nodiscard]] UTILS_EXPORT std::string remoteAssets();
[[nodiscard]] UTILS_EXPORT std::string remoteUpdateAssets();

} // namespace paths

