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
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {
    create(style, label, orient);
}

PCUI::Numeric::Numeric(
    wxWindow *parent,
    NumericDataProxy& proxy,
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, proxy) {
    create(style, label, orient);
}

void PCUI::Numeric::create(
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) {
    auto *control{new wxSpinCtrl(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        pData ? pData->min() : 0,
        pData ? pData->max() : 0,
        pData ? *pData : 0
    )};
    if (pData) control->SetIncrement(pData->increment());

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
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {
    create(style, label, orient);
}

PCUI::Decimal::Decimal(
    wxWindow *parent,
    DecimalDataProxy& proxy,
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, proxy) {
    create(style, label, orient);
}

void PCUI::Decimal::create(
    int64 style,
    const wxString& label,
    const wxOrientation& orient
) {
    auto *control{new wxSpinCtrlDouble(
        this,
        wxID_ANY,
        {},
        wxDefaultPosition,
        wxDefaultSize,
        style,
        pData ? pData->min() : 0,
        pData ? pData->max() : 0,
        pData ? *pData : 0
    )};
    if (pData) control->SetIncrement(pData->increment());

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

