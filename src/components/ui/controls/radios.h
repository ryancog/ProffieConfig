#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/radios.h
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

#include <wx/radiobox.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct RadiosData : ControlData {
    RadiosData(vector<string>&& choices) :
        mChoices{std::move(choices)} {
        mEnabled.resize(mChoices.size());
        mShown.resize(mChoices.size());
    }

    operator uint32() const { return mSelected; }
    void operator=(uint32 idx) {
        if (mSelected == idx) return;
        assert(idx < mChoices.size());
        mSelected = idx;
        refresh();
    }

    [[nodiscard]] const vector<string>& choices() const { return mChoices; }

    /**
     * Choices which are enabled will be shown regardless of their shown value
     */
    [[nodiscard]] const vector<bool>& enabledChoices() const { return mEnabled; }
    [[nodiscard]] const vector<bool>& shownChoices() const { return mShown; }

    void showChoice(uint32 idx, bool show = true) {
        assert(idx < mChoices.size());
        if (mShown[idx] == show) return;
        mShown[idx] = show;
        refresh();
    }

    void enableChoice(uint32 idx, bool enable = true) {
        assert(idx < mChoices.size());
        if (mEnabled[idx] == enable) return;
        mEnabled[idx] = enable;
        refresh();
    }

private:
    friend class Radios;
    vector<bool> mEnabled;
    vector<bool> mShown;
    vector<string> mChoices;
    uint32 mSelected{0};
};

class UI_EXPORT Radios : public ControlBase<
                         Radios,
                         RadiosData,
                         wxRadioBox,
                         wxCommandEvent> {
public:
    Radios(
        wxWindow *parent,
        RadiosData& data,
        const wxString& label = {},
        int64 style = wxRA_SPECIFY_COLS,
        wxOrientation orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI

