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

void PCUI::CheckListData::select(uint32 idx) {
    if (idx >= mItems.size()) return;

    auto [_, added]{mSelected.insert(idx)};
    if (not added) return;

    notify(ID_SELECTION);
}
void PCUI::CheckListData::unselect(uint32 idx) {
    if (not mSelected.erase(idx)) return;
    notify(ID_SELECTION);
}
void PCUI::CheckListData::clearSelections() { 
    if (mSelected.empty()) return;
    mSelected.clear();
    notify(ID_SELECTION);
}

void PCUI::CheckListData::setItems(vector<string>&& items) { 
    mItems = std::move(items); 
    for (auto iter{mSelected.begin()}; iter != mSelected.end();) {
        if (*iter >= mItems.size()) iter = mSelected.erase(iter);
        else ++iter;
    }
    notify(ID_ITEMS);
}


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
    auto *control{new wxCheckListBox(this, wxID_ANY)};
    init(control, wxEVT_CHECKLISTBOX, label, orient);
}

void PCUI::CheckList::onUIUpdate(uint32 id) {
    if (id == CheckListData::ID_ITEMS or id == ID_REBOUND) {
        pControl->Set(data()->items());
        for (auto idx : static_cast<set<uint32>>(*data())) {
            pControl->Check(idx);
        }
    } else if (id == CheckListData::ID_SELECTION) {
        const auto numItems{data()->items().size()};
        const auto& selected{static_cast<set<uint32>>(*data())};
        for (auto idx{0}; idx < numItems; ++idx) {
            pControl->Check(idx, selected.find(idx) != selected.end());
        }
    }
}

void PCUI::CheckList::onModify(wxCommandEvent& evt) {
    const auto toggledItem{evt.GetInt()};
    if (pControl->IsChecked(toggledItem)) data()->mSelected.insert(toggledItem);
    else data()->mSelected.erase(toggledItem);
    data()->update(CheckListData::ID_SELECTION);
}

