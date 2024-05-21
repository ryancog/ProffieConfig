#include "bool.h"
#include "wx/gtk/tglbtn.h"
#include "wx/gtk/tooltip.h"
#include "wx/tglbtn.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/bool.cpp
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

using namespace PCUI;

Bool::Bool(
    wxWindow* parent,
    int32_t id,
    bool initialValue,
    const wxString& label,
    const wxString& onText,
    const wxString& offText,
    const wxPoint& pos,
    const wxSize& size,
    int32_t style,
    const wxOrientation& orient) :
    wxPanel(parent, id, pos, size), onText(onText), offText(offText) {
    auto sizer{new wxBoxSizer(orient)};

    if (!label.empty()) {
        mText = new wxStaticText(this, wxID_ANY, label);
        auto sizerFlags{wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5)};
        sizer->Add(mText, orient == wxHORIZONTAL ? sizerFlags.Center() : sizerFlags);
    }

    mEntry = new wxToggleButton(
            this,
            wxID_ANY,
            offText,
            wxDefaultPosition,
            wxDefaultSize,
            style);
    mEntry->SetValue(initialValue);
    sizer->Add(mEntry, wxSizerFlags(1).Expand());

    bindEvents();

    SetSizer(sizer);
}

void Bool::bindEvents() {
    mEntry->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent& evt){
        evt.Skip(true);
        mEntry->SetLabel(mEntry->GetValue() ? onText : offText);
    });
}

void Bool::setToolTip(wxToolTip* tip) {
    SetToolTip(tip);
    mEntry->SetToolTip(new wxToolTip(tip->GetTip()));
    if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
}

wxToggleButton* Bool::entry() { return mEntry; }
wxStaticText* Bool::text() { return mText; }
const wxToggleButton* Bool::entry() const { return mEntry; }
const wxStaticText* Bool::text() const { return mText; }


