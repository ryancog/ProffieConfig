#include "radios.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

pcui::RadiosData::RadiosData(uint32 numSelections) {
    assert(numSelections > 0);

    mSelected = 0;
    mEnabled.resize(numSelections, true);
    mShown.resize(numSelections, true);
}

pcui::RadiosData& pcui::RadiosData::operator=(uint32 idx) {
    std::scoped_lock scopeLock{getLock()};
    if (mSelected == idx) return *this;
    setValue(idx);
    return *this;
}

void pcui::RadiosData::setValue(uint32 idx) {
    std::scoped_lock scopeLock{getLock()};
    assert(idx < mEnabled.size());
    mSelected = idx;
    notify(eID_Selection);
}

void pcui::RadiosData::showChoice(uint32 idx, bool show) {
    std::scoped_lock scopeLock{getLock()};
    assert(idx < mEnabled.size());
    if (mShown[idx] == show) return;
    mShown[idx] = show;
    notify(eID_Choice_State);
}

void pcui::RadiosData::enableChoice(uint32 idx, bool enable) {
    std::scoped_lock scopeLock{getLock()};
    assert(idx < mEnabled.size());
    if (mEnabled[idx] == enable) return;
    mEnabled[idx] = enable;
    notify(eID_Choice_State);
}

pcui::Radios::Radios(
    wxWindow *parent,
    RadiosData& data,
    const wxArrayString& labels,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(labels, label, orient);
};

pcui::Radios::Radios(
    wxWindow *parent,
    RadiosDataProxy& proxy,
    const wxArrayString& labels,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(labels, label, orient);
};

void pcui::Radios::create(const wxArrayString& labels, const wxString& label, wxOrientation orient) {
    assert(labels.size() > 1);
    assert(data() != nullptr or proxy() != nullptr);

    auto *box{new pcui::StaticBox(
        orient,
        this,
        label
    )};

    mRadios.reserve(labels.size());
    for (const auto& label : labels) {
        auto *radio{new wxRadioButton(
            box->childParent(),
            wxID_ANY,
            label,
            wxDefaultPosition,
            wxDefaultSize,
            mRadios.empty() ? wxRB_GROUP : 0
        )};
        if (not mRadios.empty()) box->AddSpacer(5);
        box->Add(radio);

        mRadios.push_back(radio);
    }

    init(box, wxEVT_RADIOBUTTON, wxEmptyString, wxVERTICAL);
}

void pcui::Radios::SetToolTip(uint32 idx, const wxString& tip) {
    assert(idx < mRadios.size());
    mRadios[idx]->SetToolTip(tip);
}

void pcui::Radios::onUIUpdate(uint32 id) {
    if (id == eID_Rebound) {
        assert(data()->mEnabled.size() == mRadios.size());
        refreshSizeAndLayout();
    }

    if (id == eID_Rebound or id == RadiosData::eID_Choice_State) {
        for (auto idx{0}; idx < data()->mEnabled.size(); ++idx) {
            mRadios[idx]->Show(data()->mShown[idx] or data()->mEnabled[idx]);
            mRadios[idx]->Enable(data()->mEnabled[idx]);
        }
    }
    if (id == eID_Rebound or id == RadiosData::eID_Selection) {
        mRadios[*data()]->SetValue(true);
    }
}

void pcui::Radios::onModify(wxCommandEvent& evt) {
    auto idx{0};
    for (; idx < mRadios.size(); ++idx) {
        if (mRadios[idx] == evt.GetEventObject()) break;
    }
    data()->mSelected = idx;
    data()->update(RadiosData::eID_Selection);
}

