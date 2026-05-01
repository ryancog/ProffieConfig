#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/priv/tlw.hpp
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
#include <wx/toplevel.h>

namespace pcui::priv::tlw {

void preCreate(wxTopLevelWindow *tlw);
void postCreate(wxTopLevelWindow *tlw);
void bindOnCreate(wxTopLevelWindow *tlw);

template <typename TLW>
void fit(auto *tlw) {
    // IMO it's silly that the usual fit doesn't set min size. Fit is supposed
    // to set it to fit around the children (i.e. at min size), so not making
    // it so the sizing reflects that is a little odd. Probably historical.
    tlw->SetMinSize({0, 0});
    tlw->TLW::Fit();
    tlw->SetMinSize(tlw->GetSize());
}

} // namespace pcui::priv::tlw

