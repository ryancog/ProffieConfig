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
#include "ui_export.h"

namespace PCUI {

struct UI_EXPORT TextData : ControlData {
    operator string() const { return pValue; }
    void operator=(string&&);

    void operator+=(const string_view&);
    void operator+=(char);

    // In a perfect world the data would get locked to use these...
    auto begin() { return pValue.begin(); }
    auto end() { return pValue.end(); }
    auto rbegin() { return pValue.rbegin(); }
    auto rend() { return pValue.rend(); }

    string::size_type find(char, string::size_type pos = 0);
    string::size_type find(const string_view&, string::size_type pos = 0);
    bool startsWith(const string_view&);

    string substr(string::size_type pos, string::size_type n = string::npos);

    void clear();
    void erase(string::size_type pos = 0, string::size_type n = string::npos);
    void erase(string::const_iterator first, optional<string::const_iterator> last = nullopt);
    void insert(string::size_type pos, const string_view&);

    [[nodiscard]] bool empty();
    [[nodiscard]] bool operator==(const string_view&);

    /**
     * Unconditionally set value and trigger update.
     */
    void setValue(string&& val);

    [[nodiscard]] uint32 getInsertionPoint() { return pInsertionPoint; }
    void setInsertionPoint(uint32);
    void setInsertionPointEnd();

    enum {
        ID_VALUE,
        ID_ENTER,
        ID_INSERTION,
        ID_TEXT_MAX,
    };

protected:
    friend class Text;
    string pValue;
    uint32 pInsertionPoint{0};
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
        bool insertNewline = false,
        const wxString &label = {},
        wxOrientation orient = wxVERTICAL
    );
    Text(
        wxWindow *parent,
        TextDataProxy& proxy,
        int64 style = 0,
        bool insertNewline = false,
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
    bool mInsertNewline;

    void create(int64 style, const wxString& label, wxOrientation orient);
    void onUIUpdate(uint32) final;
    void onUnbound() final;
    void onModify(wxCommandEvent&) final;
    void onModifySecondary(wxCommandEvent&) final;
    // void pruneText();

    // wxString mInvalidChars;
};

} // namespace PCUI
