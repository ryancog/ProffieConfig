#include "message.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/message.cpp
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

#include <wx/msgdlg.h>

int32 PCUI::showMessage(const wxString& msg, const wxString& caption, int32 style, wxWindow *parent) {
#   ifdef __WXMSW__
    // for dark mode
    wxGenericMessageDialog dlg(parent, msg, caption.empty() ? wxMessageBoxCaptionStr : caption, style);
    auto ret{dlg.ShowModal()};
    switch (ret) {
        case wxID_OK: return wxOK;
        case wxID_CANCEL: return wxCANCEL;
        case wxID_YES: return wxYES;
        case wxID_NO: return wxNO;
        case wxID_HELP: return wxHELP;
        default: return 0;
    }
#   else
    return wxMessageBox(msg, caption.empty() ? wxMessageBoxCaptionStr : caption, style, parent);
#   endif
}

