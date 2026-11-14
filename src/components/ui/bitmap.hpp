#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/bitmap.hpp
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

#include "utils/color.hpp"
#include "utils/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Bitmap : private wxBitmap {
    enum class Type {
        Normal,
#       ifdef __APPLE__
        Resource,
#       endif
    };

    Bitmap();
    Bitmap(std::string_view, Type = Type::Normal);

    Bitmap(const wxBitmap&);
    Bitmap(wxBitmap&&);

#   ifdef _WIN32
    Bitmap(const wxIcon&);
#   endif

    /**
     * The usual bitmap scaling function, and bitmaps in particular, behaves
     * differently across platforms. This function tries to do things in a way
     * that makes the bitmap actually scale in terms of space used in UI.
     * (Focused on in buttons). It's not perfect...
     */
    Bitmap& scale(float64 scale);

    /**
     * Same as above, but calculates the float scale from a given dimension.
     */
    Bitmap& scaleTo(uint32 dim, wxOrientation = wxHORIZONTAL);

    /**
     * Utility function to pad bitmap with given number of pixels on all sides.
     *
     * @param with_dim if specified, will scale the bitmap to a target dimension
     *                 instead of maintaining the current size.
     */
    Bitmap& pad(
        uint32 padding,
        int32 with_dim = -1,
        wxOrientation = wxHORIZONTAL
    );

    /**
     * Recolors a bitmap to be monochrome according to alpha channel.
     */
    Bitmap& color(const color::Dynamic&);

    /**
     * Is there a bitmap?
     */
    explicit operator bool() const;

    [[nodiscard]] wxSize size() const { return GetDIPSize(); }

    /**
     * Generate a bitmap suitable for UI usage according to the transformations
     * applied.
     */
    [[nodiscard]] wxBitmap realize() const;

private:
    color::Dynamic mColor;
};

} // namespace pcui

