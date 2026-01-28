#include "checklist.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

void pcui::CheckListData::select(uint32 idx) {
    std::scoped_lock scopeLock{getLock()};
    if (idx >= mItems.size()) return;

    auto [_, added]{mSelected.insert(idx)};
    if (not added) return;

    notify(eID_Checked);
}

void pcui::CheckListData::select(string&& str) {
    if (str.empty()) return;
    std::scoped_lock scopeLock{getLock()};

    uint32 idx{0};
    for (; idx < mItems.size(); ++idx) {
        if (mItems[idx] == str) break;
    }
    if (idx == mItems.size()) {
        mItems.emplace_back(std::move(str));
        mSelected.insert(mItems.size() - 1);
        notify(eID_Items);
    } else {
        mSelected.insert(idx);
    }

    notify(eID_Checked);
}


void pcui::CheckListData::unselect(uint32 idx) {
    std::scoped_lock scopeLock{getLock()};
    if (not mSelected.erase(idx)) return;
    notify(eID_Checked);
}

void pcui::CheckListData::clearSelections() { 
    std::scoped_lock scopeLock{getLock()};
    if (mSelected.empty()) return;
    mSelected.clear();
    notify(eID_Checked);
}

void pcui::CheckListData::setItems(vector<string>&& items) { 
    std::scoped_lock scopeLock{getLock()};
    mItems = std::move(items); 
    for (auto iter{mSelected.begin()}; iter != mSelected.end();) {
        if (*iter >= mItems.size()) iter = mSelected.erase(iter);
        else ++iter;
    }
    notify(eID_Items);
}


pcui::CheckList::CheckList(
    wxWindow *parent,
    CheckListData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

pcui::CheckList::CheckList(
    wxWindow *parent,
    CheckListDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void pcui::CheckList::create(const wxString& label, wxOrientation orient) {
    auto *control{new wxCheckListBox(this, wxID_ANY)};
    init(control, wxEVT_CHECKLISTBOX, label, orient);
}

void pcui::CheckList::onUIUpdate(uint32 id) {
    if (id == CheckListData::eID_Items or id == Notifier::eID_Rebound) {
        pControl->Set(data()->items());
        for (auto idx : static_cast<set<uint32>>(*data())) {
            pControl->Check(idx);
        }
        refreshSizeAndLayout();
    } else if (id == CheckListData::eID_Checked) {
        const auto numItems{data()->items().size()};
        const auto& selected{static_cast<set<uint32>>(*data())};
        for (auto idx{0}; idx < numItems; ++idx) {
            pControl->Check(idx, selected.contains(idx));
        }
    }
}

void pcui::CheckList::onUnbound() {
    pControl->Clear();
}

void pcui::CheckList::onModify(wxCommandEvent& evt) {
    const auto toggledItem{evt.GetInt()};
    if (pControl->IsChecked(toggledItem)) {
        data()->mSelected.insert(toggledItem);
    } else data()->mSelected.erase(toggledItem);

    data()->update(CheckListData::eID_Checked);
}

