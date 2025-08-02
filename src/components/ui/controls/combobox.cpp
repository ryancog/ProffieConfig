#include "combobox.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/combobox.cpp
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

void PCUI::ComboBoxData::operator=(string&& val) {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return;
    mValue = std::move(val);
    notify(ID_VALUE);
}

void PCUI::ComboBoxData::setDefaults(vector<string>&& defaults) {
    std::scoped_lock scopeLock{getLock()};
    if (mDefaults.size() == defaults.size()) {
        auto idx{0};
        for (; idx < defaults.size(); ++idx) {
            if (mDefaults[idx] != defaults[idx]) break;
        }
        if (idx == defaults.size()) return;
    }
    mDefaults = std::move(defaults);
    notify(ID_DEFAULTS);
}

void PCUI::ComboBoxData::setInsertionPoint(uint32 insertionPoint) {
    std::scoped_lock scopeLock{getLock()};
    if (mInsertionPoint == insertionPoint) return;
    mInsertionPoint = insertionPoint;
    notify(ID_INSERTION);
}

PCUI::ComboBox::ComboBox(
    wxWindow *parent,
    ComboBoxData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

PCUI::ComboBox::ComboBox(
    wxWindow *parent,
    ComboBoxDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void PCUI::ComboBox::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxComboBox(
        this,
		wxID_ANY
	)};

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, wxEVT_TEXT, label, orient);
}

void PCUI::ComboBox::onUIUpdate(uint32 id) {
    bool rebound{id == ID_REBOUND};
    if (rebound or id == ComboBoxData::ID_DEFAULTS) {
        pControl->Set(data()->mDefaults);
        refreshSizeAndLayout();
    }
    if (rebound or id == ComboBoxData::ID_VALUE) pControl->SetValue(static_cast<string>(*data()));
    if (rebound or id == ComboBoxData::ID_VALUE or id == ComboBoxData::ID_INSERTION) {
        pControl->SetInsertionPoint(data()->mInsertionPoint);
    }
}

void PCUI::ComboBox::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetString().ToStdString();
    data()->mInsertionPoint = pControl->GetInsertionPoint();
    data()->update(ComboBoxData::ID_VALUE);
}

