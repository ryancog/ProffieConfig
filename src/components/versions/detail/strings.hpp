#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/detail/strings.hpp
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

#include "utils/types.hpp"

namespace versions::detail {

constexpr cstring VERSION_STR{"VERSION"};
constexpr cstring CORE_URL_STR{"CORE_URL"};
constexpr cstring CORE_VER_STR{"CORE_VER"};

constexpr cstring BOARD_STR{"BOARD"};
constexpr cstring CORE_ID_STR{"CORE_ID"};
constexpr cstring INCLUDE_STR{"INCLUDE"};

constexpr cstring OS_STR{"OS"};
constexpr cstring PROP_STR{"PROP"};
constexpr cstring SUPPORTED_VERSIONS_STR{"SUPPORTED_VERSIONS"};

constexpr cstring INFO_FILE_STR{"info.pconf"};
constexpr cstring DATA_FILE_STR{"data.pconf"};
constexpr cstring HEADER_FILE_STR{"header.h"};

} // namespace versions::detail

