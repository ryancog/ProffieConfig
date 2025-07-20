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

enum {
    ID_PATH,
};

} // namespace PCUI

void PCUI::FilePickerData::operator=(filepath&& val) {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return;
    mValue = std::move(val);
    notify(ID_PATH);
}

PCUI::FilePicker::FilePicker(
    wxWindow *parent,
    FilePickerData& data,
    int64 style,
    const wxString& prompt,
    const wxString& wildcard,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(style, prompt, wildcard, label, orient);
};

PCUI::FilePicker::FilePicker(
    wxWindow *parent,
    FilePickerDataProxy& proxy,
    int64 style,
    const wxString& prompt,
    const wxString& wildcard,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(style, prompt, wildcard, label, orient);
};

void PCUI::FilePicker::create(
    int64 style,
    const wxString& prompt,
    const wxString& wildcard,
    const wxString& label,
    wxOrientation orient
) {
    auto *control{new wxFilePickerCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        prompt,
        wildcard,
        wxDefaultPosition,
        wxDefaultSize,
        style
    )};

    init(control, wxEVT_FILEPICKER_CHANGED, label, orient);
}

void PCUI::FilePicker::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == ID_PATH) pControl->SetPath(static_cast<filepath>(*data()).string());
}

void PCUI::FilePicker::onModify(wxFileDirPickerEvent& evt) {
    data()->mValue = evt.GetPath().ToStdString();
    data()->update(ID_PATH);
}

