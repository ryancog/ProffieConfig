#include "numeric.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

#include <wx/textctrl.h>

template<typename T> requires std::is_arithmetic_v<T>
auto pcui::priv::NumericDataTemplate<T>::operator=(
    T val
) -> NumericDataTemplate& {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return *this;
    setValue(val);
    return *this;
}

template<typename T> requires std::is_arithmetic_v<T>
void pcui::priv::NumericDataTemplate<T>::setValue(T val) {
    std::scoped_lock scopeLock{getLock()};
    const auto clampedVal{std::clamp(
        (((val - mOffset) / mIncrement) * mIncrement) + mOffset,
        mMin,
        mMax
    )};
    mValue = clampedVal;
    notify(eID_Value);
}

template<typename T> requires std::is_arithmetic_v<T>
void pcui::priv::NumericDataTemplate<T>::setRange(
    T min, T max, bool valUpdate
) {
    std::scoped_lock scopeLock{getLock()};
    if (min == mMin and max == mMax) return;

    assert(min <= max);
    mMin = min; 
    mMax = max; 

    notify(eID_Range);
    if (valUpdate) setValue(mValue);
}

template<typename T> requires std::is_arithmetic_v<T>
void pcui::priv::NumericDataTemplate<T>::setIncrement(T inc, bool valUpdate) {
    std::scoped_lock scopeLock{getLock()};
    if (inc == mIncrement) return;

    assert(inc > 0);
    mIncrement = inc;

    notify(eID_Increment);
    if (valUpdate) setValue(mValue);
}

template<typename T> requires std::is_arithmetic_v<T>
void pcui::priv::NumericDataTemplate<T>::setOffset(T offset, bool valUpdate) {
    std::scoped_lock scopeLock{getLock()};
    if (offset == mOffset) return;

    mOffset = offset;
    
    // Nothing to notify.
    if (valUpdate) setValue(mValue);
}

template struct pcui::priv::NumericDataTemplate<int32>;
template struct pcui::priv::NumericDataTemplate<float64>;

pcui::Numeric::Numeric(
    wxWindow *parent,
    NumericData& data,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {
    create(label, orient);
}

pcui::Numeric::Numeric(
    wxWindow *parent,
    NumericDataProxy& proxy,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void pcui::Numeric::create(
    const wxString& label,
    const wxOrientation& orient
) {
    auto *control{new wxSpinCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER
    )};

    init(control, wxEVT_SPINCTRL, wxEVT_TEXT_ENTER, label, orient);
}

void pcui::Numeric::onUIUpdate(uint32 id) {
    if (id == Notifier::eID_Rebound or id == NumericData::eID_Range) {
        pControl->SetRange(data()->mMin, data()->mMax);
    }
    if (id == Notifier::eID_Rebound or id == NumericData::eID_Increment) {
        pControl->SetIncrement(data()->mIncrement);
    }
    if (id == Notifier::eID_Rebound or id == NumericData::eID_Value) {
        pControl->SetValue(*data());
        refreshSizeAndLayout();
    }
}

void pcui::Numeric::onModify(wxSpinEvent& evt) {
    data()->mValue = evt.GetPosition();
    data()->update(NumericData::eID_Value);
}

void pcui::Numeric::onModifySecondary(wxCommandEvent&) {
    SetFocusIgnoringChildren();
    pControl->SetFocus();
}

pcui::Decimal::Decimal(
    wxWindow *parent,
    DecimalData& data,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {
    create(label, orient);
}

pcui::Decimal::Decimal(
    wxWindow *parent,
    DecimalDataProxy& proxy,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void pcui::Decimal::create(
    const wxString& label,
    const wxOrientation& orient
) {
    auto *control{new wxSpinCtrlDouble(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER
    )};

    init(control, wxEVT_SPINCTRLDOUBLE, wxEVT_TEXT_ENTER, label, orient);
}

void pcui::Decimal::onUIUpdate(uint32 id) {
    if (id == Notifier::eID_Rebound or id == DecimalData::eID_Range) {
        pControl->SetRange(data()->mMin, data()->mMax);
    }
    if (id == Notifier::eID_Rebound or id == DecimalData::eID_Increment) {
        pControl->SetIncrement(data()->mIncrement);
    }
    if (id == Notifier::eID_Rebound or id == DecimalData::eID_Value) {
        pControl->SetValue(*data());
        refreshSizeAndLayout();
    }
}

void pcui::Decimal::onModify(wxSpinDoubleEvent& evt) {
    data()->mValue = evt.GetValue();
    data()->update(DecimalData::eID_Value);
}

void pcui::Decimal::onModifySecondary(wxCommandEvent&) {
    SetFocusIgnoringChildren();
    pControl->SetFocus();
}

