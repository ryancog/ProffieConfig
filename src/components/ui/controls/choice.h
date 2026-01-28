#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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
#include "ui_export.h"

namespace pcui {

struct UI_EXPORT ChoiceData : ControlData {
    enum class Persistence {
        None,
        Index,
        String,
    };

    enum {
        eID_Selection,
        eID_Choices,
    };

    operator int32() const { return mValue; }
    operator string() const { 
        if (mValue == -1) return {};
        return mChoices[mValue];
    }

    /**
     * Efficiently assign/update value
     */
    ChoiceData& operator=(int32 val);
    ChoiceData& operator=(const string& val);

    /**
     * Unconditionally assign/update value
     */
    void setValue(int32);
    void setValue(const string&);

    [[nodiscard]] const vector<string>& choices() const { return mChoices; }
    void setChoices(vector<string>&& choices);

    /**
     * Attempt to maintain selection across calls to setChoices()
     * Disabled by default.
     */
    void setPersistence(Persistence persistence) {
        mPersistence = persistence;
    }

    /**
     * Enum Extensions
     */
    template<typename T> requires std::is_enum_v<T>
    ChoiceData& operator=(T e) {
        return *this = static_cast<std::underlying_type_t<T>>(e);
    }

    template<typename T> requires std::is_enum_v<T>
    bool operator==(T e) {
        return *this == static_cast<std::underlying_type_t<T>>(e);
    }

private:
    friend class Choice;
    friend class List;
    vector<string> mChoices;
    int32 mValue{-1};
    Persistence mPersistence{Persistence::None};
};

struct ChoiceDataProxy : ControlDataProxy<ChoiceData> {
    enum class Persistence {
        None,
        Index,
    };

    /**
     * Attempt to maintain selection across calls to bind()
     * Disabled by default.
     */
    void setPersistence(Persistence persistence) {
        mPersistence = persistence;
    }

    void bind(ChoiceData& other) {
        if (data() and mPersistence == Persistence::Index) {
            auto& old{*data()};
            auto oldSelection{static_cast<int32>(old)};
            auto numNew{other.choices().size()};

            if (oldSelection < numNew) other = oldSelection;
        }
        ControlDataProxy::bind(other);
    }

private:
    Persistence mPersistence{Persistence::None};
};

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
    void onUIUpdate(uint32) final;
    void onUnbound() final;
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
    void onUIUpdate(uint32) final;
    void onUnbound() final;
    void onModify(wxCommandEvent&) final;
};

} // namespace pcui
