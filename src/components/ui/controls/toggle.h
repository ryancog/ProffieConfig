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
#include "../private/export.h"

namespace PCUI {

struct ToggleData : ControlData {
    operator bool() const { return mValue; }
    void operator=(bool val) {
        mValue = val;
        refresh();
    }

private:
    friend class Toggle;
    friend class CheckBox;
    bool mValue;
};

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

private:
    void onUIUpdate() final;
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
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI
