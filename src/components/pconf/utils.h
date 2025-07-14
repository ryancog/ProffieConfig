#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/pconf/utils.h
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

#include "private/export.h"
#include "types.h"

namespace PConf {

[[nodiscard]] PCONF_EXPORT HashedData hash(const Data&);
[[nodiscard]] PCONF_EXPORT vector<string> valueAsList(const optional<string>&);
[[nodiscard]] PCONF_EXPORT optional<string> listAsValue(const vector<string>&);

} // namespace PConf

