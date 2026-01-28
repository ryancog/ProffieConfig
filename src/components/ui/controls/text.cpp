#include "text.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

pcui::TextData& pcui::TextData::operator=(string&& val) {
    std::scoped_lock scopeLock{getLock()};
    if (pValue == val) return *this;
    pValue = std::move(val);
    notify(eID_Value);
    return *this;
}

void pcui::TextData::operator+=(const string_view& val) {
    std::scoped_lock scopeLock{getLock()};
    pValue += val;
    notify(eID_Value);
}

void pcui::TextData::operator+=(char val) {
    std::scoped_lock scopeLock{getLock()};
    pValue += val;
    notify(eID_Value);
}

string::size_type pcui::TextData::find(char chr, string::size_type pos) {
    std::scoped_lock scopeLock{getLock()};
    return pValue.find(chr, pos);
}

string::size_type pcui::TextData::find(
    const string_view& str, string::size_type pos
) {
    std::scoped_lock scopeLock{getLock()};
    return pValue.find(str, pos);
}

bool pcui::TextData::startsWith(const string_view& str) {
    std::scoped_lock scopeLock{getLock()};
    return pValue.starts_with(str);
}

string pcui::TextData::substr(string::size_type pos, string::size_type n) {
    std::scoped_lock scopeLock{getLock()};
    return pValue.substr(pos, n);
}

void pcui::TextData::clear() {
    std::scoped_lock scopeLock{getLock()};
    pValue.clear();
    notify(eID_Value);
}

void pcui::TextData::erase(string::size_type pos, string::size_type n) {
    std::scoped_lock scopeLock{getLock()};
    pValue.erase(pos, n);
    notify(eID_Value);
}

void pcui::TextData::erase(
    string::const_iterator first, optional<string::const_iterator> last
) {
    std::scoped_lock scopeLock{getLock()};
    pValue.erase(first, last.value_or(pValue.end()));
    notify(eID_Value);
}

void pcui::TextData::insert(string::size_type pos, const string_view& str) {
    std::scoped_lock scopeLock{getLock()};
    pValue.insert(pos, str);
    notify(eID_Value);
}

bool pcui::TextData::empty() {
    std::scoped_lock scopeLock{getLock()};
    return pValue.empty();
}

bool pcui::TextData::operator==(const string_view& str) {
    std::scoped_lock scopeLock{getLock()};
    return pValue == str;
}

void pcui::TextData::setValue(string&& val) {
    std::scoped_lock scopeLock{getLock()};
    pValue = std::move(val);
    notify(eID_Value);
}

void pcui::TextData::setInsertionPoint(uint32 insertionPoint) {
    std::scoped_lock scopeLock{getLock()};
    if (pInsertionPoint == insertionPoint) return;

    const auto clampedPoint{std::clamp<uint32>(
        insertionPoint, 0, pValue.length()
    )};
    pInsertionPoint = clampedPoint;

    notify(eID_Insertion);
}

void pcui::TextData::setInsertionPointEnd() {
    setInsertionPoint(std::numeric_limits<uint32>::max());
}

pcui::Text::Text(
    wxWindow *parent, TextData& data,
    int64 style,
    bool insertNewline,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data),
    mInsertNewline{insertNewline} {
    create(style, label, orient);
}

pcui::Text::Text(
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

void pcui::Text::create(
    int64 style, const wxString& label, wxOrientation orient
) {
    assert(not ((style & wxTE_MULTILINE) and mInsertNewline));
    assert(
        not mInsertNewline or
        static_cast<bool>(style & wxTE_PROCESS_ENTER)
    );

    if (style & wxTE_MULTILINE) style |= wxTE_PROCESS_TAB;
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

    // Shut up wxWidgets
    if (style & wxTE_PROCESS_ENTER) {
        init(control, wxEVT_TEXT, wxEVT_TEXT_ENTER, label, orient);
    } else {
        init(control, wxEVT_TEXT, label, orient);
    }
}

void pcui::Text::styleStandard() {
    auto font{pControl->GetFont()};
    font.SetFamily(wxFONTFAMILY_DEFAULT);
    pControl->SetFont(font);
}

void pcui::Text::styleMonospace() {
    auto font{pControl->GetFont()};
    font.SetFamily(wxFONTFAMILY_TELETYPE);
    pControl->SetFont(font);
}

void pcui::Text::onUIUpdate(uint32 id) {
    bool rebound{id == Notifier::eID_Rebound};

    if (rebound or id == TextData::eID_Value) {
        pControl->ChangeValue(static_cast<string>(*data()));
    }
    if (rebound or id == TextData::eID_Insertion) {
        pControl->SetInsertionPoint(data()->pInsertionPoint);
        data()->pInsertionPoint = pControl->GetInsertionPoint();
    }

    if (rebound) {
        pControl->DiscardEdits();
        pControl->EmptyUndoBuffer();
    }
}

void pcui::Text::onUnbound() {
    pControl->Clear();
}

void pcui::Text::onModify(wxCommandEvent& evt) {
    data()->pValue = evt.GetString().ToStdString();
    data()->pInsertionPoint = pControl->GetInsertionPoint();

    if (
            evt.GetEventType() == wxEVT_TEXT or
            (evt.GetEventType() == wxEVT_TEXT_ENTER and mInsertNewline)
       ) {
        data()->update(TextData::eID_Value);
    } else if (evt.GetEventType() == wxEVT_TEXT_ENTER) {
        data()->update(TextData::eID_Enter);
    }
}

void pcui::Text::onModifySecondary(wxCommandEvent& evt) {
    if (mInsertNewline) pControl->WriteText("\\n");
    else onModify(evt);
}

