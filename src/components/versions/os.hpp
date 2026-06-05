#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/os.hpp
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

#include <map>

#include "data/primitive/model.hpp"
#include "data/primitive/models/vector.hpp"
#include "utils/version.hpp"

#include "versions_export.h"

namespace versions::os {

struct VERSIONS_EXPORT Board {
    const std::string name_;
    const std::string coreId_;
    const std::string include_;
};

struct VERSIONS_EXPORT OS : data::prim::Model {
    using BoardsMap = std::map<uint64, Board>;

    OS(
        utils::Version,
        std::string,
        utils::Version,
        BoardsMap
    );

    OS(const OS&);

    const utils::Version version_;

    const std::string coreUrl_;
    const utils::Version coreVersion_;

    const BoardsMap boards_;
};

[[nodiscard]] VERSIONS_EXPORT const data::prim::Vector& available();
[[nodiscard]] VERSIONS_EXPORT const data::prim::Vector& list();

} // namespace versions::os

