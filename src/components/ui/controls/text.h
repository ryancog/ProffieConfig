#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/text.h
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

#include <wx/textctrl.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct UI_EXPORT TextData : ControlData {
    operator string() { return mValue; }
    void operator=(string&& val);

    enum {
        ID_VALUE,
    };

private:
    friend class Text;
    string mValue;
};

using TextDataProxy = ControlDataProxy<TextData>;

class UI_EXPORT Text : public ControlBase<
                       Text,
                       TextData,
                       wxTextCtrl,
                       wxCommandEvent> {
public:
    Text(
        wxWindow *parent,
        TextData& data,
        int64 style = 0,
        const wxString &label = {},
        wxOrientation orient = wxVERTICAL
    );
    Text(
        wxWindow *parent,
        TextDataProxy& proxy,
        int64 style = 0,
        const wxString &label = {},
        wxOrientation orient = wxVERTICAL
    );

    void styleStandard();
    void styleMonospace();

    // TODO: Set up use of validators to forbid certain entry.
    //
    // Prevent any chars in the given wxString from being entered.
    // void setInvalidChars(const wxString&);

    // [[nodiscard]] wxString getInvalidChars() const;

private:
    void create(int64 style, const wxString& label, wxOrientation orient);
    void onUIUpdate(uint32) final;
    void onModify(wxCommandEvent&) final;
    // void pruneText();

    // wxString mInvalidChars;
};

} // namespace PCUI
