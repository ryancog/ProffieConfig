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
#include <wx/gdicmn.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

PCUI::Toggle::Toggle(
    wxWindow *parent,
    ToggleData& data,
    wxString onText,
    wxString offText,
    int64 style,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent),
    mData{data},
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
    control->SetValue(data);

    init(control, label, orient);

    const auto modifyCallback{[this, control](wxCommandEvent& evt) {
        control->SetLabel(evt.GetInt() ? mOnText : mOffText);
        if (not mData.isEnabled() or mData.pDirty) return;

        mData.mValue = evt.GetInt();
        if (mData.onGUIUpdate) mData.onGUIUpdate();
    }};
    const auto updateUICallback{[this, control](wxUpdateUIEvent& evt) {
        if (not mData.pDirty) return;

        control->SetValue(mData);
        Enable(mData.isEnabled());
        mData.pDirty = false;
    }};
    Bind(wxEVT_TOGGLEBUTTON, modifyCallback);
    Bind(wxEVT_UPDATE_UI, updateUICallback);
}

PCUI::Choice::Choice(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent),
    mData{data} {

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

    init(control, label, orient);

    const auto modifyCallback{[this](wxCommandEvent& evt) {
        if (not mData.isEnabled() or mData.pDirty) return;

        mData.mValue = evt.GetInt();
        if (mData.onGUIUpdate) mData.onGUIUpdate();
    }};
    const auto updateUICallback{[this, control](wxUpdateUIEvent& evt) {
        if (not mData.pDirty) return;

        control->Set(mData.mChoices);
        control->SetSelection(mData);
        Enable(mData.isEnabled());
        mData.pDirty = false;
    }};
    Bind(wxEVT_CHOICE, modifyCallback);
    Bind(wxEVT_UPDATE_UI, updateUICallback);
}

PCUI::ComboBox::ComboBox(
    wxWindow *parent,
    ComboBoxData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent),
    mData{data} {

    auto *control{new wxComboBox(
        this,
		wxID_ANY,
        string{data},
		wxDefaultPosition,
		wxDefaultSize,
        data.mDefaults
	)};
    if (data.mValue.empty() and not data.mDefaults.empty()) control->SetSelection(0);

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, label, orient);

    const auto modifyCallback{[this](wxCommandEvent& evt) {
        if (not mData.isEnabled() or mData.pDirty) return;

        mData.mValue = evt.GetInt();
        if (mData.onGUIUpdate) mData.onGUIUpdate();
    }};
    const auto updateUICallback{[this, control](wxUpdateUIEvent& evt) {
        if (not mData.pDirty) return;

        control->Set(mData.mDefaults);
        control->SetValue(string{mData});
        Enable(mData.isEnabled());
        mData.pDirty = false;
    }};
    Bind(wxEVT_COMBOBOX, modifyCallback);
    Bind(wxEVT_UPDATE_UI, updateUICallback);
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
) : ControlBase(parent),
    mData{data} {

    auto *control{new wxSpinCtrl(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        min,
        max,
        mData
    )};
    control->SetIncrement(increment);

    init(control, label, orient);

    const auto modifyCallback{[this](wxCommandEvent& evt) {
        if (not mData.isEnabled() or mData.pDirty) return;

        mData.mValue = evt.GetInt();
        if (mData.onGUIUpdate) mData.onGUIUpdate();
    }};
    const auto updateUICallback{[this, control](wxUpdateUIEvent& evt) {
        if (not mData.pDirty) return;

        control->SetValue(mData);
        Enable(mData.isEnabled());
        mData.pDirty = false;
    }};
    Bind(wxEVT_SPINCTRL, modifyCallback);
    Bind(wxEVT_UPDATE_UI, updateUICallback);
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
) : ControlBase(parent),
    mData{data} {

    auto *control{new wxSpinCtrlDouble(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        min,
        max,
        data
    )};
    control->SetIncrement(increment);

    init(control, label, orient);

    const auto modifyCallback{[this](wxCommandEvent& evt) {
        if (not mData.isEnabled() or mData.pDirty) return;

        mData.mValue = evt.GetInt();
        if (mData.onGUIUpdate) mData.onGUIUpdate();
    }};
    const auto updateUICallback{[this, control](wxUpdateUIEvent& evt) {
        if (not mData.pDirty) return;

        control->SetValue(mData);
        Enable(mData.isEnabled());
        mData.pDirty = false;
    }};
    Bind(wxEVT_SPINCTRLDOUBLE, modifyCallback);
    Bind(wxEVT_UPDATE_UI, updateUICallback);
}

PCUI::Text::Text(
    wxWindow *parent,
    TextData &data,
    int64 style,
    const wxString &label,
    wxOrientation orient
) : ControlBase(parent),
    mData{data} {

    auto *control{new wxTextCtrl(
        this,
        wxID_ANY,
        string{mData},
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};
#   ifdef __WXMAC__
    control->OSXDisableAllSmartSubstitutions();
#   endif

    init(control, label, orient);

    const auto modifyCallback{[this](wxCommandEvent& evt) {
        if (not mData.isEnabled() or mData.pDirty) return;

        mData.mValue = evt.GetInt();
        if (mData.onGUIUpdate) mData.onGUIUpdate();
    }};
    const auto updateUICallback{[this, control](wxUpdateUIEvent& evt) {
        if (not mData.pDirty) return;

        control->SetValue(string{mData});
        Enable(mData.isEnabled());
        mData.pDirty = false;
    }};
    Bind(wxEVT_TEXT, modifyCallback);
    Bind(wxEVT_UPDATE_UI, updateUICallback);
}

