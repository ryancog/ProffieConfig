#include "general.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/general.cpp
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

void pcui::detail::apply(const ChildBase& desc, wxSizerItem *item) {
    // wxSizerItem only calls the virtual func for a window, not a sizer,
    // So I have to do this check manually here in addition to the item call.
    if (item->IsSizer()) {
        item->GetSizer()->SetMinSize(desc.minSize_);
    } else {
        // Although in most cases the window min size will be handled by
        // WinBase, there may be some that aren't, so set it here anyways.
        // Worst case it's redundant.
        item->SetMinSize(desc.minSize_);
    }

    item->SetProportion(desc.proportion_);
    item->SetBorder(desc.border_.size_);
    item->SetFlag(
        desc.border_.dirs_ | (desc.expand_ ? wxEXPAND : 0) | desc.align_
    );
}

