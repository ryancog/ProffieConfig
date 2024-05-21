#include "colordata.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/colordata.cpp
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

using namespace BladeStyles;

ColorData BladeStyles::mixColors(const ColorData& a, const ColorData& b, int32_t x, int32_t shift) {
    return (a * ((1 << shift) - x) + (b * x)) >> shift;
}

bool ColorData::operator==(const ColorData& other) const {
    return (red == other.red) && (green == other.green) && (blue == other.blue);
}

ColorData ColorData::operator*(uint16_t multiplier) const {
    return ColorData{
        .red = red * multiplier,
        .green = green * multiplier,
        .blue = blue * multiplier
    };
}

ColorData ColorData::operator+(const ColorData& other) const {
    return ColorData{
        .red = red + other.red,
        .green = green + other.green,
        .blue = blue + other.blue
    };
}

ColorData ColorData::operator>>(int32_t shift) const {
    return ColorData{
        .red = red >> shift,
        .green = green >> shift,
        .blue = blue >> shift,
    };
}


