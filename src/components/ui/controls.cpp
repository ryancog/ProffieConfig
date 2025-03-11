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

PCUI::Bool::Bool(
        ::wxWindow *parent,
        wxWindowID winID,
        bool initialValue,
        string onText,
        string offText,
        int64 style,
        const string& label,
        wxOrientation orient) :
    ControlBase(parent, winID),
    mOffText(std::move(offText)),
    mOnText(std::move(onText)) {

    auto *control{new wxToggleButton(this, winID, initialValue ? mOnText : mOffText, wxDefaultPosition, wxDefaultSize, style)};
    if (initialValue) control->SetValue(true);

    init(control, label, orient);

    control->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        entry()->SetLabel(entry()->GetValue() ? mOnText : mOffText);
    });
}

PCUI::Choice::Choice(
            wxWindow *parent,
            wxWindowID winID,
            const wxArrayString& choices,
            const string& label,
            wxOrientation orient) :
    ControlBase(parent, winID) {

    auto *control{new wxChoice(this, winID, wxDefaultPosition, wxDefaultSize, choices)};
    if (choices.size()) control->SetSelection(0);

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, label, orient);
}

PCUI::ComboBox::ComboBox(
            wxWindow *parent,
            wxWindowID winID,
            const wxArrayString& choices,
            wxString defaultValue,
            const string& label,
            wxOrientation orient) :
    ControlBase(parent, winID) {

    auto *control{new wxComboBox(this, winID, defaultValue, wxDefaultPosition, wxDefaultSize, choices)};
    if (choices.size() and defaultValue.empty()) control->SetSelection(0);

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, label, orient);
}

PCUI::Numeric::Numeric(
        wxWindow* parent,
        wxWindowID winID,
        int32_t min,
        int32_t max,
        int32_t initial,
        int32_t increment,
        int64_t style,
        const string& label,
        const wxOrientation& orient) :
    ControlBase(parent, winID) {

    auto *control{new wxSpinCtrl(this, winID, {}, wxDefaultPosition, wxDefaultSize, style, min, max, initial)};
    control->SetIncrement(increment);

    init(control, label, orient);
}

PCUI::NumericDec::NumericDec(
        wxWindow* parent,
        wxWindowID winID,
        float64 min,
        float64 max,
        float64 initial,
        float64 increment,
        int64 style,
        const string& label,
        const wxOrientation& orient) :
    ControlBase(parent, winID) {

    auto *control{new wxSpinCtrlDouble(this, winID, {}, wxDefaultPosition, wxDefaultSize, style, min, max, initial)};
    control->SetIncrement(increment);

    init(control, label, orient);
}

PCUI::Text::Text(
        wxWindow *parent,
        wxWindowID winID,
        const string &initial,
        int64 style,
        const string &label,
        wxOrientation orient) :
    ControlBase(parent, winID) {

    auto *control{new wxTextCtrl(this, winID, initial, wxDefaultPosition, wxDefaultSize, style)};

    init(control, label, orient);
}

