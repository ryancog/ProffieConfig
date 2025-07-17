#include "choice.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/choice.cpp
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

namespace PCUI {

} // namespace PCUI

PCUI::Choice::Choice(
    wxWindow *parent,
    ChoiceData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {

    auto *control{new wxChoice(
        this,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize,
        data.mChoices
	)};
    control->SetSelection(data);

#   ifdef __WXGTK__
    control->SetMinSize(control->GetBestSize() + wxSize{ FromDIP(20), 0 });
#   endif

    init(control, wxEVT_CHOICE, label, orient);
}

void PCUI::Choice::onUIUpdate() {
    pControl->Set(pData->mChoices);
    pControl->SetSelection(*pData);
    pData->refreshed();
}

void PCUI::Choice::onModify(wxCommandEvent& evt) {
    pData->mValue = evt.GetInt();
}

