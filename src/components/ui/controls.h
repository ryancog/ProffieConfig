#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/controls.h
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
#include <wx/combobox.h>
#include <wx/control.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>

#include <utils/types.h>

#include "private/export.h"
#include "wx/string.h"

namespace PCUI {


/*
 * Arg Order for derived:
 * parent
 * ID
 * ...
 * label
 * orient
 */
template<class CONTROL>
class ControlBase : public wxPanel {
public:
    static_assert(std::is_base_of<wxControl, CONTROL>(), "PCUI Control core must be wxControl descendant");

    operator wxWindow *() { return this; }

    void setToolTip(wxToolTip *tip);

    [[nodiscard]] inline constexpr CONTROL *entry() { return mEntry; }
    [[nodiscard]] inline constexpr const CONTROL *entry() const { return mEntry; }
    [[nodiscard]] inline constexpr wxStaticText *text() { return mText; }
    [[nodiscard]] inline constexpr const wxStaticText *text() const { return mText; }

protected:
    ControlBase(wxWindow *parent, wxWindowID winID) : wxPanel(parent, winID) {};

    void init(CONTROL *control, wxString label, wxOrientation orient) {
        auto *sizer{new wxBoxSizer(orient)};
        constexpr auto PADDING{5};

        if (not label.empty()) {
            mText = new wxStaticText(this, wxID_ANY, label);
            auto sizerFlags{wxSizerFlags(0).Border(wxLEFT | wxRIGHT, PADDING)};
            sizer->Add(mText, orient == wxHORIZONTAL ? sizerFlags.Center() : sizerFlags);
        }

        mEntry = control;
        sizer->Add(mEntry, wxSizerFlags(1).Expand());

        SetSizerAndFit(sizer);
    }

private:
    CONTROL *mEntry{nullptr};
    wxStaticText *mText{nullptr};
};

class UI_EXPORT Bool : public ControlBase<wxToggleButton> {
public:
    Bool(
            wxWindow *parent,
            wxWindowID winID,
            bool initialValue = false,
            string onText = "True",
            string offText = "False",
            int64 style = 0,
            const string& label = {},
            wxOrientation orient = wxVERTICAL
        );

private:
    string mOnText;
    string mOffText;
    int64 mStyle;
};

class UI_EXPORT Choice : public ControlBase<wxChoice> {
public:
    Choice(
            wxWindow *parent,
            wxWindowID winID,
            const wxArrayString& choices,
            const string& label = {},
            wxOrientation orient = wxVERTICAL
          );
};

class UI_EXPORT ComboBox : public ControlBase<wxComboBox> {
public:
    ComboBox(
            wxWindow *parent,
            wxWindowID winID,
            const wxArrayString& choices,
            wxString defaultValue = wxEmptyString,
            const string& label = {},
            wxOrientation orient = wxVERTICAL
          );
};

class UI_EXPORT Numeric : public ControlBase<wxSpinCtrl> {
public:
    Numeric(
        wxWindow* parent,
        wxWindowID winID = wxID_ANY,
        int32 min       = 0,
        int32 max       = 100,
        int32 initial   = 0,
        int32 increment = 1,
        int64 style = wxSP_ARROW_KEYS,
        const string& label = {},
        const wxOrientation& orient = wxVERTICAL
        );
};

class UI_EXPORT NumericDec : public ControlBase<wxSpinCtrlDouble> {
public:
    NumericDec(
        wxWindow* parent,
        wxWindowID winID = wxID_ANY,
        float64 min       = 0,
        float64 max       = 100,
        float64 initial   = 0,
        float64 increment = 1,
        int64 style = wxSP_ARROW_KEYS,
        const string& label = {},
        const wxOrientation& orient = wxVERTICAL
        );
};

class UI_EXPORT Text : public ControlBase<wxTextCtrl> {
public:
    Text(
        wxWindow *parent,
        wxWindowID winID = wxID_ANY,
        const string &initial = {},
        int64 style = 0,
        const string &label = {},
        wxOrientation orient = wxVERTICAL
        );

    // TODO: Set up use of validators to forbid certain entry.
    //
    // Prevent any chars in the given string from being entered.
    // void setInvalidChars(const string&);

    // [[nodiscard]] string getInvalidChars() const;

private:
    // void pruneText();

    // string mInvalidChars;
};

} // namespace PCUI

