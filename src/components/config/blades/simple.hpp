#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/blades/simple.hpp
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
#include "data/hierarchic/models/number.hpp"
#include "data/hierarchic/models/string.hpp"
#include "data/receiver.hpp"

#include "config_export.h"

namespace config {

struct Config;

namespace blades {

struct Blade;

struct CONFIG_EXPORT Simple : data::hier::Model {
    Simple(Blade&);

    std::vector<Model *> children() override;

    struct LED : data::hier::Model, private data::Receiver {
        LED(Simple&);

        std::vector<Model *> children() override;

        data::hier::Choice profile_;
        data::hier::String powerPin_;
        data::hier::Integer resistance_;

    private:
        void onProfile();
    };

    LED led1_;
    LED led2_;
    LED led3_;
    LED led4_;

    Blade& parent_;
};

} // namespace blades

} // namespace config

