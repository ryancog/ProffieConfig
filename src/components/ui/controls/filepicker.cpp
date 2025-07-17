#include "filepicker.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/filepicker.cpp
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

PCUI::FilePicker::FilePicker(
    wxWindow *parent,
    FilePickerData& data,
    int64 style,
    const wxString& prompt,
    const wxString& wildcard,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {

    auto *control{new wxFilePickerCtrl(
        this,
        wxID_ANY,
        static_cast<filepath>(*pData).string(),
        prompt.IsEmpty() ? wxFileSelectorPromptStr : prompt,
        wildcard.IsEmpty() ? wxFileSelectorDefaultWildcardStr : wildcard,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};

    init(control, wxEVT_FILEPICKER_CHANGED, label, orient);
};

void PCUI::FilePicker::onUIUpdate() {
    pControl->SetPath(static_cast<filepath>(*pData).string());
    pData->pNew = false;
}

void PCUI::FilePicker::onModify(wxFileDirPickerEvent& evt) {
    pData->mValue = evt.GetPath().ToStdString();
}

