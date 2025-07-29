#include "choice.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
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

namespace PCUI {

} // namespace PCUI

void PCUI::ChoiceData::operator=(int32 val) {
    std::scoped_lock scopeLock{getLock()};
    assert(val == -1 or val < mChoices.size());
    if (mValue == val) return;
    mValue = val;
    notify(ID_SELECTION);
}

void PCUI::ChoiceData::operator=(const string& val) {
    std::scoped_lock scopeLock{getLock()};
    if (mValue != -1 and mChoices[mValue] == val) return;

    auto idx{0};
    for (; idx < mChoices.size(); ++idx) {
        if (mChoices[idx] == val) break;
    }
    if (idx == mChoices.size()) idx = -1;

    if (mValue == idx) return;
    mValue = idx;
    notify(ID_SELECTION);
}

void PCUI::ChoiceData::setChoices(vector<string>&& choices) { 
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
    if (mValue != -1) lastChoice = mChoices[mValue];

    mChoices = std::move(choices); 
    notify(ID_CHOICES);

    switch (mChoicePersistence) {
        case PERSISTENCE_NONE:
            // Use default update handler
            *this = -1;
            break;
        case PERSISTENCE_INDEX:
            if (mValue >= mChoices.size()) mValue = -1;
            notify(ID_SELECTION);
            break;
        case PERSISTENCE_STRING:
            mValue = -1;
            for (auto idx{0}; idx < mChoices.size(); ++idx) {
                if (mChoices[idx] == lastChoice) {
                    mValue = idx;
                    break;
                }
            }
            notify(ID_SELECTION);
            break;
    }
}

PCUI::Choice::Choice(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

PCUI::Choice::Choice(
    wxWindow *parent,
    ChoiceDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void PCUI::Choice::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxChoice(
        this,
		wxID_ANY
	)};

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, wxEVT_CHOICE, label, orient);
}

void PCUI::Choice::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == ChoiceData::ID_CHOICES) {
        pControl->Set(data()->mChoices);
        pControl->SetSelection(*data());
        refreshSizeAndLayout();
    } else if (id == ChoiceData::ID_SELECTION) {
        pControl->SetSelection(*data());
    }
}

void PCUI::Choice::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetInt();
    data()->update(ChoiceData::ID_SELECTION);
}

PCUI::List::List(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

PCUI::List::List(
    wxWindow *parent,
    ChoiceDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void PCUI::List::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxListBox(
        this,
		wxID_ANY
	)};

    init(control, wxEVT_LISTBOX, label, orient);
}

void PCUI::List::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == ChoiceData::ID_CHOICES) {
        pControl->Set(data()->mChoices);
        pControl->SetSelection(*data());
        refreshSizeAndLayout();
    } else if (id == ChoiceData::ID_SELECTION) {
        pControl->SetSelection(*data());
    }
}

void PCUI::List::onModify(wxCommandEvent& evt) {
    data()->mValue = evt.GetInt();
    data()->update(ChoiceData::ID_SELECTION);
}


