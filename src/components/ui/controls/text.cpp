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
    if (pValue == val) return;
    pValue = std::move(val);
    notify(ID_VALUE);
}

void PCUI::TextData::operator+=(const string& val) {
    std::scoped_lock scopeLock{getLock()};
    pValue += val;
    notify(ID_VALUE);
}

void PCUI::TextData::operator+=(char val) {
    std::scoped_lock scopeLock{getLock()};
    pValue += val;
    notify(ID_VALUE);
}

string::size_type PCUI::TextData::find(char chr) {
    std::scoped_lock scopeLock{getLock()};
    return pValue.find(chr);
}

string::size_type PCUI::TextData::find(const string& str) {
    std::scoped_lock scopeLock{getLock()};
    return pValue.find(str);
}

void PCUI::TextData::clear() {
    std::scoped_lock scopeLock{getLock()};
    pValue.clear();
    notify(ID_VALUE);
}

void PCUI::TextData::erase(string::size_type pos, string::size_type n) {
    std::scoped_lock scopeLock{getLock()};
    pValue.erase(pos, n);
    notify(ID_VALUE);
}

void PCUI::TextData::erase(string::const_iterator first, optional<string::const_iterator> last) {
    std::scoped_lock scopeLock{getLock()};
    pValue.erase(first, last.value_or(pValue.end()));
    notify(ID_VALUE);
}

void PCUI::TextData::insert(string::size_type pos, const string& str) {
    std::scoped_lock scopeLock{getLock()};
    pValue.insert(pos, str);
    notify(ID_VALUE);
}

bool PCUI::TextData::empty() {
    std::scoped_lock scopeLock{getLock()};
    return pValue.empty();
}

bool PCUI::TextData::operator==(cstring str) {
    std::scoped_lock scopeLock{getLock()};
    return pValue == str;
}

void PCUI::TextData::setValue(string&& val) {
    std::scoped_lock scopeLock{getLock()};
    pValue = std::move(val);
    notify(ID_VALUE);
}

void PCUI::TextData::setInsertionPoint(uint32 insertionPoint) {
    std::scoped_lock scopeLock{getLock()};
    if (pInsertionPoint == insertionPoint) return;
    pInsertionPoint = insertionPoint;
    notify(ID_INSERTION);
}

PCUI::Text::Text(
    wxWindow *parent,
    TextData& data,
    int64 style,
    bool insertNewline,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data),
    mInsertNewline{insertNewline} {
    create(style, label, orient);
}

PCUI::Text::Text(
    wxWindow *parent,
    TextDataProxy& proxy,
    int64 style,
    bool insertNewline,
    const wxString &label,
    wxOrientation orient
) : ControlBase(parent, proxy),
    mInsertNewline{insertNewline} {
    create(style, label, orient);
}

void PCUI::Text::create(int64 style, const wxString& label, wxOrientation orient) {
    assert(not ((style & wxTE_MULTILINE) and mInsertNewline));
    assert(not mInsertNewline or static_cast<bool>(style & wxTE_PROCESS_ENTER));

    auto *control{new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};
#   ifdef __WXMAC__
    control->OSXDisableAllSmartSubstitutions();
#   endif

    init(control, wxEVT_TEXT, wxEVT_TEXT_ENTER, label, orient);
}

void PCUI::Text::styleStandard() {
    auto font{pControl->GetFont()};
    font.SetFamily(wxFONTFAMILY_DEFAULT);
    pControl->SetFont(font);
}

void PCUI::Text::styleMonospace() {
    auto font{pControl->GetFont()};
    font.SetFamily(wxFONTFAMILY_TELETYPE);
    pControl->SetFont(font);
}

void PCUI::Text::onUIUpdate(uint32 id) {
    bool rebound{id == ID_REBOUND};

    if (rebound or id == TextData::ID_VALUE) pControl->ChangeValue(static_cast<string>(*data()));
    if (rebound or id == TextData::ID_VALUE or id == TextData::ID_INSERTION) {
        pControl->SetInsertionPoint(data()->pInsertionPoint);
    }
}

void PCUI::Text::onUnbound() {
    pControl->Clear();
}

void PCUI::Text::onModify(wxCommandEvent& evt) {
    data()->pValue = evt.GetString().ToStdString();
    data()->pInsertionPoint = pControl->GetInsertionPoint();

    if (evt.GetEventType() == wxEVT_TEXT or (evt.GetEventType() == wxEVT_TEXT_ENTER and mInsertNewline)) {
        data()->update(TextData::ID_VALUE);
    } else if (evt.GetEventType() == wxEVT_TEXT_ENTER) {
        data()->update(TextData::ID_ENTER);
    }
}

void PCUI::Text::onModifySecondary(wxCommandEvent& evt) {
    if (mInsertNewline) {
        auto newString{pControl->GetValue() + "\\n"};
        pControl->ChangeValue(newString);
        pControl->SetInsertionPointEnd();
        evt.SetString(newString);
    }
    onModify(evt);
}

