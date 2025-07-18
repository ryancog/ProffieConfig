#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/button.h
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

#include <wx/button.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct ButtonData : ControlData {
    void operator=(bool val) {
        if (not val) return;
        refresh();
    }

private:
    friend class Button;
};

using ButtonDataProxy = ControlDataProxy<ButtonData>;

class UI_EXPORT Button : public ControlBase<
                         Button,
                         ButtonData,
                         wxButton,
                         wxCommandEvent> {
public:
    Button(
        wxWindow *parent,
        ButtonData& data,
        int64 style = 0,
        const wxString& label = {}
    );
    Button(
        wxWindow *parent,
        ButtonDataProxy& proxy,
        int64 style = 0,
        const wxString& label = {}
    );

private:
    void create(int64 style, const wxString& label);

    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};


} // namespace PCUI
