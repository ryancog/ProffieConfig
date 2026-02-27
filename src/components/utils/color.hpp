#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/color.hpp
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

#include "utils_export.h"

namespace color {

struct UTILS_EXPORT Dynamic {
    Dynamic(wxColour dark, wxColour light);
    Dynamic(wxSystemColour color);

    Dynamic();
    ~Dynamic();
    Dynamic(const Dynamic&);
    Dynamic& operator=(const Dynamic&);

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
            wxColour dark_;
            // Lighter color
            wxColour light_;
        } mStd;
        wxSystemColour mSysColor;
    };
};

UTILS_EXPORT extern const wxColour DARK_BLUE;
UTILS_EXPORT extern const wxColour LIGHT_BLUE;

} // namespace color


