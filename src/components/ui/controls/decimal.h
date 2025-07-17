#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/decimal.h
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

struct DecimalData : ControlData {
    operator float64() const { return mValue; }
    void operator=(float64 val) {
        mValue = val;
        pNew = true;
    }

private:
    friend class Decimal;
    float64 mValue;
};

class UI_EXPORT Decimal : public ControlBase<
                          Decimal,
                          DecimalData,
                          wxSpinCtrlDouble,
                          wxSpinDoubleEvent> {
public:
    Decimal(
        wxWindow *parent,
        DecimalData& data,
        float64 min       = 0,
        float64 max       = 100,
        float64 increment = 1,
        int64 style = wxSP_ARROW_KEYS,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxSpinDoubleEvent&) final;
};

} // namespace PCUI
