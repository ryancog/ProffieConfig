#include "data.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/priv/data.cpp
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

using namespace versions;

// ptr-in-a-vec is "needed" because there's const members in these structures,
// which prevents a vector working normally.
//
// This probably isn't a very ideal solution, and if the ergonomics of it are
// particularly annoying I might want to do something different.

data::prim::Vector priv::props;
data::prim::Vector priv::os;

data::prim::Vector priv::availableOS;
data::prim::Vector priv::availableProps;

