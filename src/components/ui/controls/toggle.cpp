#include "toggle.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/toggle.cpp
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

} // namespace PCUI

PCUI::Toggle::Toggle(
    wxWindow *parent,
    ToggleData& data,
    wxString onText,
    wxString offText,
    int64 style,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data),
    mOnText{std::move(onText)},
    mOffText{std::move(offText)} {
    auto *control{new wxToggleButton(
        this,
		wxID_ANY,
		data ? mOnText : mOffText,
		wxDefaultPosition,
		wxDefaultSize,
		style
	)};
    init(control, wxEVT_TOGGLEBUTTON, label, orient);
}

void PCUI::Toggle::onUIUpdate() {
    pControl->SetValue(pData);
    pData->refreshed();
}

void PCUI::Toggle::onModify(wxCommandEvent& evt) {
    pData->mValue = evt.GetInt();
}

PCUI::CheckBox::CheckBox(
    wxWindow *parent,
    ToggleData& data,
    int64 style,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    auto *control{new wxCheckBox(
        this,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};
    init(control, wxEVT_CHECKBOX, label, orient);
};

void PCUI::CheckBox::onUIUpdate() {
    pControl->SetValue(pData);
    pData->refreshed();
}

void PCUI::CheckBox::onModify(wxCommandEvent& evt) {
    pData->mValue = evt.GetInt();
}

