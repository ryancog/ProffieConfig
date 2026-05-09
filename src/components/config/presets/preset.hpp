#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/presets/preset.hpp
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

#include "data/hierarchic/model.hpp"
#include "data/hierarchic/models/string.hpp"
#include "data/hierarchic/models/vector.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace presets {

struct Array;

struct CONFIG_EXPORT Preset : data::hier::Model {
    Preset(Config&);
    Preset(const Preset&, Config&);

    std::vector<Model *> children() override;

    data::hier::String name_;
    data::hier::String fontDir_;
    // vector<string> fontDirs;
    data::hier::String track_;

    data::hier::Vector styles_;
};

struct CONFIG_EXPORT Style : data::hier::Model {
    Style(Config&);
    Style(const Style&, Config&);

    std::vector<Model *> children() override;

    data::hier::String comment_;
    data::hier::String content_;
};

} // namespace presets

} // namespace config

