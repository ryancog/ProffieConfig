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

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>::operator=(T val) {
    std::scoped_lock scopeLock{getLock()};
    const auto newVal{std::clamp(val, mMin, mMax)};
    if (mValue == newVal) return;
    mValue = newVal;
    notify(ID_VALUE);
}

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>:: setRange(T min, T max) { 
    std::scoped_lock scopeLock{getLock()};
    if (min == mMin and max == mMax) return;
    assert(min <= max);
    mMin = min; 
    mMax = max; 
    notify(ID_RANGE);
}

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>:: setIncrement(T inc) {
    std::scoped_lock scopeLock{getLock()};
    if (inc == mIncrement) return;
    assert(inc > 0);
    mIncrement = inc;
    notify(ID_INCREMENT);
}

template struct PCUI::Private::NumericDataTemplate<int32>;
template struct PCUI::Private::NumericDataTemplate<float64>;

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
        wxID_ANY
    )};

    init(control, wxEVT_SPINCTRL, label, orient);
}

void PCUI::Numeric::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == data()->ID_RANGE) pControl->SetRange(data()->mMin, data()->mMax);
    if (id == ID_REBOUND or id == data()->ID_INCREMENT) pControl->SetIncrement(data()->mIncrement);
    if (id == ID_REBOUND or id == data()->ID_VALUE) pControl->SetValue(*data());
}

void PCUI::Numeric::onModify(wxSpinEvent& evt) {
    data()->mValue = evt.GetPosition();
    data()->update(data()->ID_VALUE);
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
        wxID_ANY
    )};

    init(control, wxEVT_SPINCTRLDOUBLE, label, orient);
}

void PCUI::Decimal::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == data()->ID_RANGE) pControl->SetRange(data()->mMin, data()->mMax);
    if (id == ID_REBOUND or id == data()->ID_INCREMENT) pControl->SetIncrement(data()->mIncrement);
    if (id == ID_REBOUND or id == data()->ID_VALUE) pControl->SetValue(*data());
}

void PCUI::Decimal::onModify(wxSpinDoubleEvent& evt) {
    data()->mValue = evt.GetValue();
    data()->update(data()->ID_VALUE);
}

