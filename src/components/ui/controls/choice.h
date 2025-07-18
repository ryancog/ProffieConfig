#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/choice.h
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

#include <wx/choice.h>
#include <wx/listbox.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct ChoiceData : ControlData {
    operator int32() const { return mValue; }
    operator string() const { 
        if (mValue == -1) return {};
        return mChoices[mValue];
    }

    void operator=(int32 val) {
        mValue = val;
        refresh();
    }

    const vector<string>& choices() const { return mChoices; }
    void setChoices(vector<string>&& choices) { 
        mChoices = std::move(choices); 
        if (mValue >= mChoices.size()) mValue = -1;
        refresh();
    }

private:
    friend class Choice;
    friend class List;
    friend class Radios;
    vector<string> mChoices;
    int32 mValue{-1};
};

using ChoiceDataProxy = ControlDataProxy<ChoiceData>;

class UI_EXPORT Choice : public ControlBase<
                         Choice,
                         ChoiceData,
                         wxChoice,
                         wxCommandEvent> {
public:
    Choice(
        wxWindow *parent,
        ChoiceData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );
    Choice(
        wxWindow *parent,
        ChoiceDataProxy& proxy,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void create(const wxString& label, wxOrientation orient);
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

class UI_EXPORT List : public ControlBase<
                       List,
                       ChoiceData,
                       wxListBox,
                       wxCommandEvent> {
public:
    List(
        wxWindow *parent,
        ChoiceData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );
    List(
        wxWindow *parent,
        ChoiceDataProxy& proxy,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void create(const wxString& label, wxOrientation orient);
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI
