#include "panel.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/panel.cpp
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


#include <wx/stattext.h>

PCUI::Panel::Panel(wxWindow *parent, PanelData& data) :
    mData{&data} {
    mPanel = new wxPanel(parent);
    if (mData->mType == PanelData::Type::FRAMED) {
        mStaticBox = new wxStaticBox(mPanel, wxID_ANY, mData->mLabel);
    }
}

void PCUI::Panel::onUIUpdate() {
    if (mStaticBox) mStaticBox->SetLabel(mData->mLabel);
    mPanel->Enable(mData->mEnabled);
    mPanel->Show(mData->mShown);
}

