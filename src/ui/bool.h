#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/bool.h
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

#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/tglbtn.h>
#include <wx/panel.h>
#include <wx/stattext.h>

namespace PCUI {

class Bool : public wxPanel {
public:
    Bool(
            wxWindow* parent, 
            int32_t id, 
            bool initialValue = false,
            const wxString& label = wxEmptyString,
            const wxString& onText = "True",
            const wxString& offText = "False",
            const wxPoint& pos = wxDefaultPosition, 
            const wxSize& size = wxDefaultSize, 
            int32_t style = 0,
            const wxOrientation& orient = wxVERTICAL);

    void setToolTip(wxToolTip* tip);

    wxToggleButton* entry();
    wxStaticText* text();
    const wxToggleButton* entry() const;
    const wxStaticText* text() const;

private:
    wxToggleButton* mEntry{nullptr};
    wxStaticText* mText{nullptr};

    void bindEvents();

    const wxString onText;
    const wxString offText;
};

}
