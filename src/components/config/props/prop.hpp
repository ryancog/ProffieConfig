#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/props/prop.hpp
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

#include "config_export.h"

namespace config::props {

struct CONFIG_EXPORT Prop : data::Node {
    virtual std::string filename() = 0;
    virtual bool isDefault() = 0;
};

struct CONFIG_EXPORT Setting : data::Model {
    /**
     * Generate the define string representation for this prop setting.
     *
     * @return nullopt if the prop setting is not output, string otherwise.
     */
    virtual std::optional<std::string> defineString() = 0;
};

/**
 * Glue executable must link this w/ the versions manager to provide a way to
 * generate the prop data when creating a Config
 */
using Generator = std::vector<std::unique_ptr<Prop>>(*)(data::Node *parent);

CONFIG_EXPORT void connectGenerator(Generator);

} // namespace config::props

