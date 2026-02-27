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

#include "data/choice.hpp"
#include "data/number.hpp"
#include "data/string.hpp"

#include "config_export.h"

namespace config::blades {

struct CONFIG_EXPORT Simple {
    Simple();

    struct Star {
        Star();

        data::Choice led_;
        data::String powerPin_;
        data::Integer resistance_;
    };

    Star star1_;
    Star star2_;
    Star star3_;
    Star star4_;
};

} // namespace config::blades

