#include "controls.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/controls.cpp
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

#include <wx/choice.h>
#include <wx/event.h>
#include <wx/filepicker.h>
#include <wx/gdicmn.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

namespace PCUI {

} // namespace PCUI

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
    pData.pDirty = false;
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
    pData.pDirty = false;
}

void PCUI::Toggle::onModify(wxCommandEvent& evt) {
    pData.mValue = evt.GetInt();
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
    pData.pDirty = false;
}

void PCUI::CheckBox::onModify(wxCommandEvent& evt) {
    pData = evt.GetInt();
}

PCUI::Choice::Choice(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {

    auto *control{new wxChoice(
        this,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize,
        data.mChoices
	)};
    control->SetSelection(data);

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, wxEVT_CHOICE, label, orient);
}

void PCUI::Choice::onUIUpdate() {
    pControl->Set(pData.mChoices);
    pControl->SetSelection(pData);
    pData.pDirty = false;
}

void PCUI::Choice::onModify(wxCommandEvent& evt) {
    pData.mValue = evt.GetInt();
}

PCUI::ComboBox::ComboBox(
    wxWindow *parent,
    ComboBoxData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {

    auto *control{new wxComboBox(
        this,
		wxID_ANY,
        static_cast<string>(data),
		wxDefaultPosition,
		wxDefaultSize,
        data.mDefaults
	)};

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, wxEVT_COMBOBOX, label, orient);
}

void PCUI::ComboBox::onUIUpdate() {
    pControl->Set(pData.mDefaults);
    pControl->SetValue(static_cast<string>(pData));
    pData.pDirty = false;
}

void PCUI::ComboBox::onModify(wxCommandEvent& evt) {
    pData.mValue = evt.GetString().ToStdString();
}

PCUI::Numeric::Numeric(
    wxWindow *parent,
    NumericData& data,
    int32 min,
    int32 max,
    int32 increment,
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {

    auto *control{new wxSpinCtrl(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        min,
        max,
        pData
    )};
    control->SetIncrement(increment);

    init(control, wxEVT_SPINCTRL, label, orient);

}

void PCUI::Numeric::onUIUpdate() {
    pControl->SetValue(pData);
    pData.pDirty = false;
}

void PCUI::Numeric::onModify(wxSpinEvent& evt) {
    pData.mValue = evt.GetPosition();
}

PCUI::Decimal::Decimal(
    wxWindow *parent,
    DecimalData& data,
    float64 min,
    float64 max,
    float64 increment,
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {

    auto *control{new wxSpinCtrlDouble(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        min,
        max,
        pData
    )};
    control->SetIncrement(increment);

    init(control, wxEVT_SPINCTRLDOUBLE, label, orient);
}

void PCUI::Decimal::onUIUpdate() {
    pControl->SetValue(pData);
    pData.pDirty = false;
}

void PCUI::Decimal::onModify(wxSpinDoubleEvent& evt) {
    pData.mValue = evt.GetValue();
}

PCUI::Text::Text(
    wxWindow *parent,
    TextData &data,
    int64 style,
    const wxString &label,
    wxOrientation orient
) : ControlBase(parent, data) {

    auto *control{new wxTextCtrl(
        this,
        wxID_ANY,
        static_cast<string>(pData),
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};
#   ifdef __WXMAC__
    control->OSXDisableAllSmartSubstitutions();
#   endif

    init(control, wxEVT_TEXT, label, orient);
}

void PCUI::Text::onUIUpdate() {
    pControl->SetValue(static_cast<string>(pData));
    pData.pDirty = false;
}

void PCUI::Text::onModify(wxCommandEvent& evt) {
    pData.mValue = evt.GetString().ToStdString();
}

PCUI::FilePicker::FilePicker(
    wxWindow *parent,
    FilePickerData& data,
    int64 style,
    const wxString& prompt,
    const wxString& wildcard,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {

    auto *control{new wxFilePickerCtrl(
        this,
        wxID_ANY,
        static_cast<filepath>(pData).string(),
        prompt.IsEmpty() ? wxFileSelectorPromptStr : prompt,
        wildcard.IsEmpty() ? wxFileSelectorDefaultWildcardStr : wildcard,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};

    init(control, wxEVT_FILEPICKER_CHANGED, label, orient);
};

void PCUI::FilePicker::onUIUpdate() {
    pControl->SetPath(static_cast<filepath>(pData).string());
    pData.pDirty = false;
}

void PCUI::FilePicker::onModify(wxFileDirPickerEvent& evt) {
    pData.mValue = evt.GetPath().ToStdString();
}


