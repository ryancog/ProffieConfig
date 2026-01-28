#include "filepicker.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

auto pcui::FilePickerData::operator=(filepath&& val) -> FilePickerData& {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return *this;

    mValue = std::move(val);
    notify(eID_Path);

    return *this;
}

pcui::FilePicker::FilePicker(
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

pcui::FilePicker::FilePicker(
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

void pcui::FilePicker::create(
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

void pcui::FilePicker::onUIUpdate(uint32 id) {
    if (id == Notifier::eID_Rebound or id == FilePickerData::eID_Path) {
        pControl->SetPath(static_cast<filepath>(*data()).string());
    }
}

void pcui::FilePicker::onModify(wxFileDirPickerEvent& evt) {
    data()->mValue = evt.GetPath().ToStdString();
    data()->update(FilePickerData::eID_Path);
}

