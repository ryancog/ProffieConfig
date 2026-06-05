#include "os.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/os.cpp
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

#include "versions/priv/data.hpp"

using namespace versions::os;

OS::OS(
    utils::Version version,
    std::string coreUrl,
    utils::Version coreVersion,
    BoardsMap boards
) : version_(std::move(version)),
    coreUrl_(std::move(coreUrl)),
    coreVersion_(std::move(coreVersion)),
    boards_(std::move(boards)) {}

OS::OS(const OS& other) :
    OS(
        other.version_,
        other.coreUrl_,
        other.coreVersion_,
        other.boards_
    ) {}

const data::prim::Vector& versions::os::available() {
    return priv::availableOS;
}

const data::prim::Vector&  versions::os::list() {
    return priv::os;
}

