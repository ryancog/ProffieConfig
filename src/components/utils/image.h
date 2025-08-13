#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/image.h
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

#include <wx/bitmap.h>
#include <wx/settings.h>

#include <utils/types.h>

#include "private/export.h"

namespace Image {

struct UTILS_EXPORT DynamicColor {
    DynamicColor(wxColour dark, wxColour light);
    DynamicColor(wxSystemColour color);

    DynamicColor();
    ~DynamicColor();
    DynamicColor(const DynamicColor&);
    DynamicColor& operator=(const DynamicColor&);

    [[nodiscard]] wxColour color() const;
    [[nodiscard]] operator bool() const;

private:
    enum class Type {
        STANDARD,
        SYSTEM,
    } mType;
    union {
        // Win32 compiler does not allow anonymous structs in this manner
        struct {
            // Darker color
            wxColour mDark;
            // Lighter color
            wxColour mLight;
        } mStd;
        wxSystemColour mSysColor;
    };
};

UTILS_EXPORT extern const wxColour DARK_BLUE;
UTILS_EXPORT extern const wxColour LIGHT_BLUE;

UTILS_EXPORT wxBitmap loadPNG(const string& name, bool dpiScaled = true);
/**
 * @param size Size to scale the bitmap to. Only one dimension may be provided.
 * @param color Optional color to set bitmap to.
 */
UTILS_EXPORT wxBitmap loadPNG(const string& name, wxSize size, wxColour color = wxNullColour); 

UTILS_EXPORT wxBitmap newBitmap(wxSize);
UTILS_EXPORT int32 getDPIScaleFactor();
UTILS_EXPORT wxColour getAccentColor();

} // namespace Image

