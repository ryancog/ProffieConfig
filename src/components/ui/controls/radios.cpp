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

enum {
    ID_SELECTION,
    ID_CHOICE_STATE,
};

}

void PCUI::RadiosData::init(vector<string>&& choices) {
    assert(mSelected = std::numeric_limits<uint32>::max());

    mSelected = 0;
    mChoices = std::move(choices);
    mEnabled.resize(mChoices.size());
    mShown.resize(mChoices.size());
}

void PCUI::RadiosData::operator=(uint32 idx) {
    std::scoped_lock scopeLock{getLock()};
    if (mSelected == idx) return;
    assert(idx < mChoices.size());
    mSelected = idx;
    notify(ID_SELECTION);
}

void PCUI::RadiosData::showChoice(uint32 idx, bool show) {
    std::scoped_lock scopeLock{getLock()};
    assert(idx < mChoices.size());
    if (mShown[idx] == show) return;
    mShown[idx] = show;
    notify(ID_CHOICE_STATE);
}

void PCUI::RadiosData::enableChoice(uint32 idx, bool enable) {
    std::scoped_lock scopeLock{getLock()};
    assert(idx < mChoices.size());
    if (mEnabled[idx] == enable) return;
    mEnabled[idx] = enable;
    notify(ID_CHOICE_STATE);
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
        data()->mChoices,
        0,
        orient == wxVERTICAL ? wxRA_SPECIFY_COLS : wxRA_SPECIFY_ROWS
    )};

    init(control, wxEVT_RADIOBOX, wxEmptyString, wxVERTICAL);
}

void PCUI::Radios::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == ID_CHOICE_STATE) {
        for (auto idx{0}; idx < data()->mChoices.size(); ++idx) {
            pControl->Show(idx, data()->mShown[idx] or data()->mEnabled[idx]);
            pControl->Enable(idx, data()->mEnabled[idx]);
        }
    }
    if (id == ID_REBOUND or id == ID_SELECTION) pControl->SetSelection(*data());
}

void PCUI::Radios::onModify(wxCommandEvent& evt) {
    data()->mSelected = evt.GetInt();
    data()->update(ID_SELECTION);
}

