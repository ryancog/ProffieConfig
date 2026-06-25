#include "font.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/font.cpp
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

#include <utility>

#include <wx/font.h>
#include <wx/settings.h>

wxFont pcui::detail::FontData::makeFont() const {
    if (const auto *ptr{std::get_if<wxFont>(this)}) {
        return *ptr;
    } 

    // So, at least on macOS, wxSYS_DEFAULT_GUI_FONT returns a font with proper
    // spacing but incorrect size, and wxFont returns a font with proper size
    // but incorrect spacing.
    //
    // I don't know why, and I can't seem to figure out how to make things
    // more elegant. Probably need to investigate this and raise an issue w/
    // wx if it's consistent in a sample.
    auto sysFont{[] {
        const auto defaultFont{wxFont{wxFontInfo{}}};
        const auto defaultFontSize{defaultFont.GetFractionalPointSize()};

        auto guiFont{wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)};
        guiFont.SetFractionalPointSize(defaultFontSize);

        return guiFont;
    }()};

    switch (std::get<Font>(*this)) {
        using enum Font;
        case Normal:
            return sysFont;
        case Monospace:
            sysFont.SetFamily(wxFONTFAMILY_TELETYPE);
            return sysFont;
        case Title:
            sysFont.Scale(1.5);
            sysFont.MakeBold();
            return sysFont;
        case Header:
            return sysFont.Scale(1.2);
        case Caption:
            return sysFont.Scale(0.8);
        }

    std::unreachable();
}

wxFont pcui::operator-(Font style) {
    return detail::FontData{style}.makeFont();
}

