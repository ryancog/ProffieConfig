#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/numeric.h
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

#include <wx/spinctrl.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

struct NumericData : ControlData {
    operator int32() const { return mValue; }
    void operator=(int32 val) {
        mValue = val;
        pNew = true;
    }

private:
    friend class Numeric;
    int32 mValue{0};
};

class UI_EXPORT Numeric : public ControlBase<
                          Numeric,
                          NumericData,
                          wxSpinCtrl,
                          wxSpinEvent> {
public:
    Numeric(
        wxWindow *parent,
        NumericData& data,
        int32 min       = 0,
        int32 max       = 100,
        int32 increment = 1,
        int64 style = wxSP_ARROW_KEYS,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
    );

private:
    void onUIUpdate() final;
    void onModify(wxSpinEvent&) final;
};

} // namespace PCUI
