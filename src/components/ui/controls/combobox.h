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

#include <mutex>
#include <wx/combobox.h>

#include "../private/export.h"
#include "ui/controls/text.h"

namespace PCUI {

struct UI_EXPORT ComboBoxData : TextData {
    const vector<string>& defaults() const { return mDefaults; }
    void setDefaults(vector<string>&& defaults);

    enum {
        ID_DEFAULTS = ID_TEXT_MAX,
    };

private:
    friend class ComboBox;
    vector<string> mDefaults;
};

using ComboBoxDataProxy = ControlDataProxy<ComboBoxData>;

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
    ComboBox(
        wxWindow *parent,
        ComboBoxDataProxy& proxy,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void create(const wxString& label, wxOrientation orient);
    void onUIUpdate(uint32) final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI
