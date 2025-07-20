#include "text.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/text.cpp
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

void PCUI::TextData::operator=(string&& val) {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return;
    mValue = std::move(val);
    notify(ID_VALUE);
}

PCUI::Text::Text(
    wxWindow *parent,
    TextData& data,
    int64 style,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(style, label, orient);
}

PCUI::Text::Text(
    wxWindow *parent,
    TextDataProxy& proxy,
    int64 style,
    const wxString &label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(style, label, orient);
}

void PCUI::Text::create(int64 style, const wxString& label, wxOrientation orient) {
    auto *control{new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        style | wxTE_PROCESS_ENTER
    )};
#   ifdef __WXMAC__
    control->OSXDisableAllSmartSubstitutions();
#   endif

    init(control, wxEVT_TEXT_ENTER, label, orient);
}

void PCUI::Text::styleStandard() {
    pControl->SetFont(wxFont(
        10,
        wxFONTFAMILY_TELETYPE,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    ));
}

void PCUI::Text::styleMonospace() {
    pControl->SetFont(pControl->GetDefaultStyle().GetFont());
}

void PCUI::Text::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == TextData::ID_VALUE) pControl->SetValue(static_cast<string>(*data()));
}

void PCUI::Text::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetString().ToStdString();
    data()->update(TextData::ID_VALUE);
}

