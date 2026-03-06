#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/priv/data.hpp
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

#include <mutex>

#include "versions/os.hpp"
#include "versions/prop.hpp"

namespace versions::priv {

extern std::recursive_mutex lock;
extern std::vector<std::unique_ptr<props::Versioned>> props;
extern std::vector<std::unique_ptr<os::Versioned>> os;

extern std::vector<os::Available> availableOS;
extern std::vector<props::Available> availableProps;

} // namespace versions::priv

