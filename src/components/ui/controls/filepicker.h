#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/filepicker.h
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

#include <wx/filepicker.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct FilePickerData : ControlData {
    operator filepath() { return mValue; }
    void operator=(filepath&& val) {
        mValue = std::move(val);
        refresh();
    }

private:
    friend class FilePicker;
    filepath mValue;
};

class UI_EXPORT FilePicker : public ControlBase<
                             FilePicker,
                             FilePickerData,
                             wxFilePickerCtrl,
                             wxFileDirPickerEvent> {
public:
    FilePicker(
        wxWindow *parent,
        FilePickerData& data,
        int64 style,
        const wxString& prompt = {},
        const wxString& wildcard = {},
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );
    
private:
    void onUIUpdate() final;
    void onModify(wxFileDirPickerEvent&) final;
};

} // namespace PCUI
