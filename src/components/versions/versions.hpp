#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/versions/versions.hpp
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

#include <optional>
#include <string>

#include "log/branch.hpp"
#include "utils/string.hpp"
#include "utils/version.hpp"

#include "versions_export.h"

namespace versions {

constexpr utils::TrimRules PROP_NAME_RULES{
    .allowAlpha = true,
    .allowNum = true,
    .safeList = "_",
};

/**
 * Load versions files from local computer.
 */
VERSIONS_EXPORT void loadLocal(logging::Branch * = nullptr);

/**
 * Fetch available downloads from server.
 */
VERSIONS_EXPORT std::optional<std::string> fetch(logging::Branch * = nullptr);

/**
 * Install default downloads for this installation.
 */
VERSIONS_EXPORT std::optional<std::string> installDefault(
    bool purge, logging::Branch * = nullptr
);

VERSIONS_EXPORT std::optional<std::string> downloadOS(
    const utils::Version&, logging::Branch * = nullptr
);

VERSIONS_EXPORT std::optional<std::string> downloadProp(
    const std::string&, logging::Branch * = nullptr
);

} // namespace versions

