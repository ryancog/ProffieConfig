#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/ui/controls/button.h
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

#include <wx/button.h>

#include "utils/image.h"
#include "utils/types.h"

#include "ui_export.h"

namespace PCUI {

/**
 * Not a normal PCUI control. No data, simply wrapper for convenience.
 */
class UI_EXPORT Button : public wxButton {
public:
    Button(
        wxWindow *parent,
        wxWindowID id,
        const wxString& label,
        const wxSize& size = wxDefaultSize,
        int64 style = 0,
        const string& bmp = {},
        const wxSize& bmpSize = wxDefaultSize,
        const Image::DynamicColor& bmpColor = {}
    );
};

} // namespace PCUI
