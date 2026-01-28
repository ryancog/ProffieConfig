#include "choice.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/ui/controls/choice.cpp
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

auto pcui::ChoiceData::operator=(int32 val) -> ChoiceData& {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return *this;

    setValue(val);
    return *this;
}

void pcui::ChoiceData::setValue(int32 val) {
    std::scoped_lock scopeLock{getLock()};
    assert(val == -1 or (val >= 0 and val < mChoices.size()));
    mValue = val;
    notify(eID_Selection);
}

auto pcui::ChoiceData::operator=(const string& val) -> ChoiceData& {
    std::scoped_lock scopeLock{getLock()};
    if (mValue != -1 and mChoices[mValue] == val) return *this;

    setValue(val);
    return *this;
}

void pcui::ChoiceData::setValue(const string& val) {
    std::scoped_lock scopeLock{getLock()};

    auto idx{0};
    for (; idx < mChoices.size(); ++idx) {
        if (mChoices[idx] == val) break;
    }
    if (idx == mChoices.size()) idx = -1;

    if (mValue == idx) return;
    mValue = idx;
    notify(eID_Selection);
}

void pcui::ChoiceData::setChoices(vector<string>&& choices) { 
    std::scoped_lock scopeLock{getLock()};

    // If equal return
    if (mChoices.size() == choices.size()) {
        auto idx{0};
        for (; idx < choices.size(); ++idx) {
            if (mChoices[idx] != choices[idx]) break;
        }
        if (idx == choices.size()) return;
    }

    string lastChoice;
    int32 lastValue{mValue};
    if (mValue != -1) lastChoice = mChoices[mValue];

    mChoices = std::move(choices); 
    mValue = -1;
    switch (mPersistence) {
        using enum Persistence;
        case None:
            break;
        case Index:
            mValue = lastValue;
            if (mValue >= mChoices.size()) mValue = -1;
            break;
        case String:
            for (auto idx{0}; idx < mChoices.size(); ++idx) {
                if (mChoices[idx] == lastChoice) {
                    mValue = idx;
                    break;
                }
            }
            break;
    }

    notify(eID_Choices);
    if (lastValue != mValue) notify(eID_Selection);
}

pcui::Choice::Choice(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

pcui::Choice::Choice(
    wxWindow *parent,
    ChoiceDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void pcui::Choice::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxChoice(
        this,
		wxID_ANY
	)};

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, wxEVT_CHOICE, label, orient);
}

void pcui::Choice::onUIUpdate(uint32 id) {
    if (id == eID_Rebound or id == ChoiceData::eID_Choices) {
        pControl->Set(data()->mChoices);
        pControl->SetSelection(*data());
        refreshSizeAndLayout();
    } else if (id == ChoiceData::eID_Selection) {
        pControl->SetSelection(*data());
    }
}
void pcui::Choice::onUnbound() {
    pControl->Clear();
}


void pcui::Choice::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetInt();
    data()->update(ChoiceData::eID_Selection);
}

pcui::List::List(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

pcui::List::List(
    wxWindow *parent,
    ChoiceDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void pcui::List::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxListBox(
        this,
		wxID_ANY
	)};

    init(control, wxEVT_LISTBOX, label, orient);
}

void pcui::List::onUIUpdate(uint32 id) {
    if (id == eID_Rebound or id == ChoiceData::eID_Choices) {
        pControl->Set(data()->mChoices);
        pControl->SetSelection(*data());
        refreshSizeAndLayout();
    } else if (id == ChoiceData::eID_Selection) {
        pControl->SetSelection(*data());
    }
}

void pcui::List::onUnbound() {
    pControl->Clear();
}

void pcui::List::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetInt();
    data()->update(ChoiceData::eID_Selection);
}

