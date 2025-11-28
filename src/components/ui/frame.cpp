#include "frame.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/frame.cpp
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

#ifdef __WIN32__
#include <dwmapi.h>
#endif

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/window.h>

#include <app/app.h>

namespace PCUI {

Frame::Frame(wxWindow *parent,
             wxWindowID winID,
             const wxString& title,
             const wxPoint& pos,
             const wxSize& size,
             int32_t style,
             const wxString& name) {
#   ifdef __WXMSW__
    SetDoubleBuffered(true);
#   endif
    Create(parent, winID, title, pos, size, style, name);

#	ifdef __WIN32__
    SetIcon(wxICON(ApplicationIcon));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));

    Bind(wxEVT_CREATE, [this](wxWindowCreateEvent&) {
        DWORD useDarkMode{App::darkMode()};
        DwmSetWindowAttribute(
#           ifdef __WXGTK__
            GTKGetWin32Handle(),
#           else
            GetHWND(),
#           endif
            DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
            &useDarkMode,
            sizeof(DWORD)
        );
    });
#	endif
}

Frame::~Frame() {
    if (mReference and *mReference) (*mReference) = nullptr;
}

void Frame::setReference(Frame** ref) {
    mReference = ref;
}

} // namespace PCUI

