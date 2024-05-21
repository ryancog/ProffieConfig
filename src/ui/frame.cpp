#include "frame.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/frame.cpp
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

#include <wx/sizer.h>

#include "../resources/icons/icon.xpm"

using namespace PCUI;

Frame::Frame(wxWindow* parent,
             int32_t id,
             const wxString& title,
             const wxPoint& pos,
             const wxSize& size,
             int32_t style,
             const wxString& name) {
    Create(parent, id, title, pos, size, style, name);

#	ifdef __WXMSW__
    SetIcon(wxICON(IDI_ICON1));
#	endif
}

Frame::~Frame() {
    if (reference && *reference) (*reference) = nullptr;
}

void Frame::setReference(Frame** ref) {
    reference = ref;
}
