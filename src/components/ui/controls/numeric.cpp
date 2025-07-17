#include "numeric.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/numeric.cpp
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

PCUI::Numeric::Numeric(
    wxWindow *parent,
    NumericData& data,
    int32 min,
    int32 max,
    int32 increment,
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {

    auto *control{new wxSpinCtrl(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        min,
        max,
        *pData
    )};
    control->SetIncrement(increment);

    init(control, wxEVT_SPINCTRL, label, orient);
}

void PCUI::Numeric::onUIUpdate() {
    pControl->SetRange(pData->mMin, pData->mMax);
    pControl->SetIncrement(pData->mIncrement);
    pControl->SetValue(*pData);
    pData->refreshed();
}

void PCUI::Numeric::onModify(wxSpinEvent& evt) {
    pData->mValue = evt.GetPosition();
}

PCUI::Decimal::Decimal(
    wxWindow *parent,
    DecimalData& data,
    float64 min,
    float64 max,
    float64 increment,
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {

    auto *control{new wxSpinCtrlDouble(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        min,
        max,
        *pData
    )};
    control->SetIncrement(increment);

    init(control, wxEVT_SPINCTRLDOUBLE, label, orient);
}

void PCUI::Decimal::onUIUpdate() {
    pControl->SetRange(pData->mMin, pData->mMax);
    pControl->SetIncrement(pData->mIncrement);
    pControl->SetValue(*pData);
    pData->refreshed();
}

void PCUI::Decimal::onModify(wxSpinDoubleEvent& evt) {
    pData->mValue = evt.GetValue();
}

