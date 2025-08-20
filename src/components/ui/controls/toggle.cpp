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

PCUI::ToggleData& PCUI::ToggleData::operator=(bool val) {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return *this;
    setValue(val);
    return *this;
}

void PCUI::ToggleData::operator|=(bool val) {
    if (not val) return;
    *this = val;
}

void PCUI::ToggleData::setValue(bool val) {
    std::scoped_lock scopeLock{getLock()};
    mValue = val;
    notify(ID_VALUE);
}

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
    create(style, label, orient);
}

PCUI::Toggle::Toggle(
    wxWindow *parent,
    ToggleDataProxy& proxy,
    wxString onText,
    wxString offText,
    int64 style,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy),
    mOnText{std::move(onText)},
    mOffText{std::move(offText)} {
    create(style, label, orient);
}

void PCUI::Toggle::create(
    int64 style,
    const wxString& label,
    wxOrientation orient
) {
    auto *control{new wxToggleButton(
        this,
		wxID_ANY,
		wxEmptyString,
		wxDefaultPosition,
		wxDefaultSize,
		style
	)};

    init(control, wxEVT_TOGGLEBUTTON, label, orient);
}

void PCUI::Toggle::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == ToggleData::ID_VALUE) {
        pControl->SetValue(*data());
        pControl->SetLabelText(data() ? mOnText : mOffText);
        SetSizerAndFit(GetSizer());
        auto *parent{wxGetTopLevelParent(this)};
        if (parent) {
            parent->Layout();
            parent->Fit();
        }
    }
}

void PCUI::Toggle::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetInt();
    data()->update(ToggleData::ID_VALUE);
}

PCUI::CheckBox::CheckBox(
    wxWindow *parent,
    ToggleData& data,
    int64 style,
    const wxString& label
) : ControlBase(parent, data) {
    create(style, label);
};

PCUI::CheckBox::CheckBox(
    wxWindow *parent,
    ToggleDataProxy& proxy,
    int64 style,
    const wxString& label
) : ControlBase(parent, proxy) {
    create(style, label);
};

void PCUI::CheckBox::create(
    int64 style,
    const wxString& label
) {
    auto *control{new wxCheckBox(
        this,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};
    init(control, wxEVT_CHECKBOX, wxEmptyString, wxVERTICAL);
}

void PCUI::CheckBox::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == ToggleData::ID_VALUE) pControl->SetValue(*data());
}

void PCUI::CheckBox::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetInt();
    data()->update(ToggleData::ID_VALUE);
}

