#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/paths/paths.h
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

#include <utils/types.h>

#include "private/export.h"

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
[[nodiscard]] PATHS_EXPORT filepath executable(Executable exec = Executable::CURRENT);

/**
 * Retrieve the app root path
 *
 * This is the location where core app things are located.
 * A location modifiable without admin rights.
 *
 * @return root path
 */
[[nodiscard]] PATHS_EXPORT filepath approot();

[[nodiscard]] PATHS_EXPORT filepath binaries();
[[nodiscard]] PATHS_EXPORT filepath libraries();
[[nodiscard]] PATHS_EXPORT filepath components();
[[nodiscard]] PATHS_EXPORT filepath resources();

[[nodiscard]] PATHS_EXPORT filepath data();
[[nodiscard]] PATHS_EXPORT filepath configs();
[[nodiscard]] PATHS_EXPORT filepath injections();
[[nodiscard]] PATHS_EXPORT filepath props();
[[nodiscard]] PATHS_EXPORT filepath proffieos();

[[nodiscard]] PATHS_EXPORT filepath logs();

[[nodiscard]] PATHS_EXPORT string website();
[[nodiscard]] PATHS_EXPORT string remoteAssets();
[[nodiscard]] PATHS_EXPORT string remoteUpdateAssets();

} // namespace Paths

