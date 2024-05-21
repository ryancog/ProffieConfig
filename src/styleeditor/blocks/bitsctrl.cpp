#include "bitsctrl.h"
#include "wx/gtk/textctrl.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/blocks/bitsctrl.cpp
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

#include <wx/tooltip.h>
#include <wx/sizer.h>

using namespace PCUI;

BitsCtrl::BitsCtrl(
        wxWindow* parent,
        int32_t id,
        int8_t numBits,
        int32_t initialValue,
        const wxString& label,
        const wxSize& size,
        int32_t style,
        const wxOrientation& orient) :
    wxPanel(parent, id, wxDefaultPosition, size), numBits(numBits) {
    auto sizer{new wxBoxSizer(orient)};

    if (!label.empty()) {
        mText = new wxStaticText(this, wxID_ANY, label);
        auto sizerFlags{wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5)};
        sizer->Add(mText, orient == wxHORIZONTAL ? sizerFlags.Center() : sizerFlags);
    }

    mEntry = new wxTextCtrl(this,
                            wxID_ANY,
                            wxEmptyString,
                            wxDefaultPosition,
                            wxDefaultSize,
                            style);
    setValue(initialValue);
    sizer->Add(mEntry, wxSizerFlags(1).Expand());

    SetSizer(sizer);
}

void BitsCtrl::setValue(int32_t value) {
    wxString str;
    for (auto i{0}; i < numBits; i++) {
        str += (value & (1 << i)) ? '1' : '0';
    }
    mEntry->SetValue(str);
}

int32_t BitsCtrl::getValue() const {
    int32_t ret{0};
    auto strVal{mEntry->GetValue()};
    for (auto i{0}; i < numBits; i++) {
        if (strVal.at(i) == '1') ret |= (1 << i);
    }
    return ret;
}

void BitsCtrl::setToolTip(wxToolTip* tip) {
    SetToolTip(tip);
    mEntry->SetToolTip(new wxToolTip(tip->GetTip()));
    if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
}

const wxTextCtrl* BitsCtrl::entry() const { return mEntry; }
const wxStaticText* BitsCtrl::text() const { return mText; }

