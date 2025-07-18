#include "radios.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/radios.cpp
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

}

PCUI::Radios::Radios(
    wxWindow *parent,
    RadiosData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
};

PCUI::Radios::Radios(
    wxWindow *parent,
    RadiosDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
};

void PCUI::Radios::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxRadioBox(
        this,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        pData->mChoices,
        0,
        orient == wxVERTICAL ? wxRA_SPECIFY_COLS : wxRA_SPECIFY_ROWS
    )};

    init(control, wxEVT_RADIOBOX, wxEmptyString, wxVERTICAL);
}

void PCUI::Radios::onUIUpdate() {
    for (auto idx{0}; idx < pData->mChoices.size(); ++idx) {
        pControl->Show(idx, pData->mShown[idx] or pData->mEnabled[idx]);
        pControl->Enable(idx, pData->mEnabled[idx]);
    }
    pControl->SetSelection(*pData);
    pData->refreshed();
}

void PCUI::Radios::onModify(wxCommandEvent& evt) {
    pData->mSelected = evt.GetInt();
}

