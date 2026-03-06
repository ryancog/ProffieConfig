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

#include <vector>

#include "data/hierarchy/model.hpp"
#include "data/vector.hpp"
#include "utils/version.hpp"

#include "versions_export.h"

namespace versions::os {

struct VERSIONS_EXPORT Available {
    const utils::Version version_;

    const std::string coreUrl_;
    const utils::Version coreVersion_;

    struct BoardInfo {
        const std::string name_;
        const std::string coreId_;
        const std::string include_;
    };
    const std::vector<BoardInfo> boards_;
};

struct VERSIONS_EXPORT BoardInfo : data::Model {
    BoardInfo(
        std::string name,
        std::string coreId,
        std::string include
    );

    BoardInfo(const BoardInfo&, data::Node *);

    const std::string name_;
    const std::string coreId_;
    const std::string include_;
};

struct VERSIONS_EXPORT Versioned : data::Model {
    Versioned(
        utils::Version version,
        std::string coreUrl,
        utils::Version coreVer,
        std::vector<BoardInfo> boards
    );

    Versioned(const Versioned&, data::Node *);

    const utils::Version version_;

    const std::string coreUrl_;
    const utils::Version coreVersion_;

    const data::Vector boards_;
};

struct VERSIONS_EXPORT Context {
    Context();
    ~Context();

    const std::vector<Available>& available() [[clang::lifetimebound]];
    const std::vector<std::unique_ptr<Versioned>>&
        list() [[clang::lifetimebound]];
};

} // namespace versions::os

