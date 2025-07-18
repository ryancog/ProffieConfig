#include "checklist.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/checklist.cpp
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

PCUI::CheckList::CheckList(
    wxWindow *parent,
    CheckListData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

PCUI::CheckList::CheckList(
    wxWindow *parent,
    CheckListDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void PCUI::CheckList::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxCheckListBox(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        pData->mItems
    )};

    init(control, wxEVT_CHECKLISTBOX, label, orient);
}

void PCUI::CheckList::onUIUpdate() {
    pControl->Set(pData->items());
    for (auto idx : static_cast<set<uint32>>(*pData)) pControl->Check(idx);
    pData->refreshed();
}

void PCUI::CheckList::onModify(wxCommandEvent& evt) {
    const auto toggledItem{evt.GetInt()};
    if (pControl->IsChecked(toggledItem)) pData->mSelected.insert(toggledItem);
    else pData->mSelected.erase(toggledItem);
}

