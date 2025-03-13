#include "plaque.h"
#include "wx/generic/statbmpg.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/plaque.cpp
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

#include <wx/event.h>
#include <wx/settings.h>
#include <wx/statbox.h>

#include <utils/theme.h>

UI_EXPORT wxWindow *PCUI::createPlaque(wxWindow *parent, wxWindowID winID) {
#   ifdef __WXOSX__
    // This is kind of a reverse way of building what's effectively a wxStaticBoxSizer
    auto *plaque{new wxStaticBox(parent, winID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0)};
#   else
    // TODO: the border doesn't update on color/theme switch
    constexpr auto STYLE{wxTAB_TRAVERSAL | wxBORDER_SUNKEN};
    auto *plaque{new wxPanel(parent, winID, wxDefaultPosition, wxDefaultSize, STYLE)};
    Theme::colorWindow(plaque, {{ 30, 2 }});
#   endif

    return plaque;
}

UI_EXPORT wxStaticBitmapBase *PCUI::createStaticImage(wxWindow *parent, wxWindowID winID, const wxBitmap& bitmap) {
#   ifdef __WINDOWS__
    auto *image{new wxGenericStaticBitmap(parent, winID, bitmap)};
#   else
    auto *image{new wxStaticBitmap(parent, winID, bitmap)};
#   endif
    image->SetScaleMode(wxStaticBitmapBase::Scale_AspectFill);
    return image;
}

