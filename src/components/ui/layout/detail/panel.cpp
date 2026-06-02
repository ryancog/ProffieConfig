#include "panel.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/detail/panel.hpp
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

#ifdef _WIN32
#include <windows.h>
#endif

#include <wx/gdicmn.h>

using namespace pcui::detail;

Panel::Panel(
    wxWindow *parent,
    wxWindowID id,
    long style,
    const wxString& name
) {
    create(parent, id, style, name);
}

void Panel::create(
    wxWindow *parent,
    wxWindowID id,
    long style,
    const wxString& name
) {
    wxPanel::Create(
        parent,
        id,
        wxDefaultPosition,
        wxDefaultSize,
        style,
        name
    );

#   ifdef _WIN32
#   ifdef __WXGTK__
    auto *hwnd{GTKGetWin32Handle()};
#   else
    auto *hwnd{GetHWND()};
#   endif

    /*
    auto exStyle{GetWindowLongA(hwnd, GWL_EXSTYLE)};
    SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    */
#   endif
}

