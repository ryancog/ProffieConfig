#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/define.hpp
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

#include "data/hierarchy/node.hpp"
#include "data/string.hpp"

#include "config_export.h"

namespace config {

struct Settings;

namespace settings {

struct CONFIG_EXPORT Define : data::Node {
    Define(data::Node *, std::string&& = {}, std::string&& = {});
    ~Define() override;

    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    data::String name_;
    data::String value_;
};

} // namespace settings

} // namespace config

