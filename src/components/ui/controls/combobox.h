#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/combobox.h
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

#include <wx/combobox.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct ComboBoxData : ControlData {
    operator string() const { return mValue; }
    void operator=(string&& val) {
        mValue = std::move(val);
        refresh();
    }

    const vector<string>& defaults() const { return mDefaults; }
    void setDefaults(vector<string>&& defaults) {
        mDefaults = std::move(defaults);
        refresh();
    }

private:
    friend class ComboBox;
    vector<string> mDefaults;
    string mValue;
};

class UI_EXPORT ComboBox : public ControlBase<
                           ComboBox,
                           ComboBoxData,
                           wxComboBox,
                           wxCommandEvent> {
public:
    ComboBox(
        wxWindow *parent,
        ComboBoxData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI
