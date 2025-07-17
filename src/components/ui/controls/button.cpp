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
    ButtonData& data,
    int64 style,
    const wxString& label
) : ControlBase(parent, data) {

    auto *control{new wxButton(
        this,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};

    init(control, wxEVT_BUTTON, wxEmptyString, wxVERTICAL);
};

void PCUI::Button::onUIUpdate() {}

void PCUI::Button::onModify(wxCommandEvent&) {
    pData->pNew = false;
}


} // namespace PCUI

