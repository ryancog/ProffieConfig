#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/simple.h
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

#include "utils/types.h"

namespace Config {

enum LED : int32 {
    NONE = 0,

    CREE_RED          = 1 << 1,
    CREE_GREEN        = 1 << 2,
    CREE_BLUE         = 1 << 3,
    CREE_AMBER        = 1 << 4,
    CREE_RED_ORANGE   = 1 << 5,
    CREE_WHITE        = 1 << 6,
    CREE_LED = CREE_RED | CREE_GREEN | CREE_BLUE | CREE_AMBER | CREE_RED_ORANGE | CREE_WHITE,

    RED               = 1 << 7,
    GREEN             = 1 << 8,
    BLUE              = 1 << 9,
    SIMPLE_LED = RED | GREEN | BLUE,

    USES_RESISTANCE = CREE_LED,
};

struct SimpleBlade {

};

};
