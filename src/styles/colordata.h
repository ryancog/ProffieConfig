#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/colordata.h
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

#include <cstdint>

#include <wx/colour.h>

namespace BladeStyles {

struct ColorData {
    // All the colors are represented on a
    // 0 to 32768 scale, however due to scaling during
    // temporary operations, they're stored as uint32_t
    uint32_t red{0};
    uint32_t green{0};
    uint32_t blue{0};
    uint32_t alpha{32768};

    bool operator==(const ColorData& other) const;
    ColorData operator*(uint16_t multiplier) const;
    ColorData operator+(const ColorData& other) const;
    ColorData operator>>(int32_t shift) const;

};

/**
 * Used in ProffieOS to "mix colors" (obviously)
 *
 * Not sure what x is exactly, haven't closely examined the code.
 * Shift is a bitshift value... also not sure the significance of it.
 */
ColorData mixColors(const ColorData&, const ColorData&, int32_t x, int32_t shift);

// Yes, we're going to play off the english spelling, lol
wxColour colorToColour(const ColorData&);

}

