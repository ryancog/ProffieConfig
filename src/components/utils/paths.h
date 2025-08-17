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

} // namespace Paths

