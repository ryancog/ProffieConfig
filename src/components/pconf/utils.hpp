#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/pconf/utils.hpp
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
#include <vector>
#include <string>

#include "pconf/types.hpp"

#include "pconf_export.h"

namespace pconf {

[[nodiscard]] PCONF_EXPORT HashedData hash(const Data&);

[[nodiscard]] PCONF_EXPORT std::vector<std::string> valueAsList(
    const std::optional<std::string>&
);

[[nodiscard]] PCONF_EXPORT std::optional<std::string> listAsValue(
    const std::vector<std::string>&
);

} // namespace pconf

