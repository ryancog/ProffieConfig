#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/toggle.h
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

#include <wx/tglbtn.h>
#include <wx/checkbox.h>

#include "base.h"
#include "ui_export.h"

namespace PCUI {

struct UI_EXPORT ToggleData : ControlData {
    operator bool() const { return mValue; }

    /**
     * Efficiently set/update value
     */
    void operator=(bool val);
    void operator|=(bool val);

    /**
     * Unconditionally set/update value
     */
    void setValue(bool val);

    enum {
        ID_VALUE,
    };

private:
    friend class Toggle;
    friend class CheckBox;
    bool mValue{false};
};

using ToggleDataProxy = ControlDataProxy<ToggleData>;

class UI_EXPORT Toggle : public ControlBase<
                         Toggle,
                         ToggleData,
                         wxToggleButton,
                         wxCommandEvent> {
public:
    Toggle(
        wxWindow *parent,
        ToggleData& data,
        wxString onText = "True",
        wxString offText = "False",
        int64 style = 0,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );
    Toggle(
        wxWindow *parent,
        ToggleDataProxy& proxy,
        wxString onText = "True",
        wxString offText = "False",
        int64 style = 0,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );

private:
    void create(
        int64 style,
        const wxString& label,
        wxOrientation orient
    );
    void onUIUpdate(uint32) final;
    void onModify(wxCommandEvent&) final;

    wxString mOnText;
    wxString mOffText;
};

class UI_EXPORT CheckBox : public ControlBase<
                           CheckBox,
                           ToggleData,
                           wxCheckBox,
                           wxCommandEvent> {
public:
    CheckBox(
        wxWindow *parent,
        ToggleData& data,
        int64 style = 0,
        const wxString& label = {}
    );
    CheckBox(
        wxWindow *parent,
        ToggleDataProxy& proxy,
        int64 style = 0,
        const wxString& label = {}
    );

private:
    void create(
        int64 style,
        const wxString& label
    );
    void onUIUpdate(uint32) final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI
