#include "button.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/button.cpp
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

namespace PCUI {

PCUI::Button::Button(
    wxWindow *parent,
    wxWindowID id,
    const wxString& label,
    const wxSize& size,
    int64 style,
    const string& bmp,
    const wxSize& bmpSize,
    Image::DynamicColor bmpColor
) : wxButton(parent, id, label, wxDefaultPosition, size, style) {
    if (bmp.empty()) return;

    auto handler{[this, bmp, bmpSize, bmpColor](wxSysColourChangedEvent& evt) {
        auto bitmap{Image::loadPNG(bmp, bmpSize, bmpColor.color())};
        SetBitmap(bitmap);
        evt.Skip();
    }};

    wxSysColourChangedEvent evt;
    handler(evt);
    Bind(wxEVT_SYS_COLOUR_CHANGED, handler);
}


} // namespace PCUI

