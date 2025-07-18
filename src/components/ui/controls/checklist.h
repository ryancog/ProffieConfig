#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/checklist.h
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

#include <wx/checklst.h>

#include "utils/types.h"

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct CheckListData : ControlData {
    operator set<uint32>() const { return mSelected; }

    void select(uint32 idx) {
        if (idx >= mItems.size()) return;

        auto [_, added]{mSelected.insert(idx)};
        if (not added) return;

        refresh();
    }
    void unselect(uint32 idx) {
        if (not mSelected.erase(idx)) return;
        refresh();
    }
    void clearSelections() { 
        if (mSelected.empty()) return;
        mSelected.clear();
        refresh();
    }

    const vector<string>& items() const { return mItems; }
    void setItems(vector<string>&& items) { 
        mItems = std::move(items); 
        for (auto iter{mSelected.begin()}; iter != mSelected.end();) {
            if (*iter >= mItems.size()) iter = mSelected.erase(iter);
            else ++iter;
        }
        refresh();
    }

private:
    friend class CheckList;
    vector<string> mItems;
    set<uint32> mSelected;
};

using CheckListDataProxy = ControlDataProxy<CheckListData>;

class UI_EXPORT CheckList : public ControlBase<
                         CheckList,
                         CheckListData,
                         wxCheckListBox,
                         wxCommandEvent> {
public:
    CheckList(
        wxWindow *parent,
        CheckListData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );
    CheckList(
        wxWindow *parent,
        CheckListDataProxy& proxy,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void create(const wxString& label, wxOrientation orient);

    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI
