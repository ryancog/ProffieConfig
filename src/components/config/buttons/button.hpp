#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/buttons/button.hpp
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
#include "data/hierarchic/models/choice.hpp"
#include "data/hierarchic/models/string.hpp"
#include "data/hierarchic/models/number.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace buttons {

struct CONFIG_EXPORT Button : data::hier::Model {
    Button(Config&);
    ~Button() override;

    using Model::children;
    std::vector<const Model *> children() const override;

    data::hier::Choice type_;
    data::hier::Choice event_;

    data::hier::String pin_;
    data::hier::String name_;
    data::hier::Integer touch_;
};

} // namespace buttons

} // namespace config

