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

#include <wx/textctrl.h>

namespace PCUI {

} // namespace PCUI

template<typename T> requires std::is_arithmetic_v<T>
PCUI::Private::NumericDataTemplate<T>& PCUI::Private::NumericDataTemplate<T>::operator=(T val) {
    std::scoped_lock scopeLock{getLock()};
    if (mValue == val) return *this;
    setValue(val);
    return *this;
}

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>::setValue(T val) {
    std::scoped_lock scopeLock{getLock()};
    const auto clampedVal{std::clamp((((val - mOffset) / mIncrement) * mIncrement) + mOffset, mMin, mMax)};
    mValue = clampedVal;
    notify(ID_VALUE);
}

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>::setRange(T min, T max, bool valUpdate) {
    std::scoped_lock scopeLock{getLock()};
    if (min == mMin and max == mMax) return;
    assert(min <= max);
    mMin = min; 
    mMax = max; 
    notify(ID_RANGE);
    if (valUpdate) setValue(mValue);
}

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>::setIncrement(T inc, bool valUpdate) {
    std::scoped_lock scopeLock{getLock()};
    if (inc == mIncrement) return;
    assert(inc > 0);
    mIncrement = inc;
    notify(ID_INCREMENT);
    if (valUpdate) setValue(mValue);
}

template<typename T> requires std::is_arithmetic_v<T>
void PCUI::Private::NumericDataTemplate<T>::setOffset(T offset, bool valUpdate) {
    std::scoped_lock scopeLock{getLock()};
    if (offset == mOffset) return;
    mOffset = offset;
    // Nothing to notify, it's purely an ish-this.
    if (valUpdate) setValue(mValue);
}

template struct PCUI::Private::NumericDataTemplate<int32>;
template struct PCUI::Private::NumericDataTemplate<float64>;

PCUI::Numeric::Numeric(
    wxWindow *parent,
    NumericData& data,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {
    create(label, orient);
}

PCUI::Numeric::Numeric(
    wxWindow *parent,
    NumericDataProxy& proxy,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void PCUI::Numeric::create(
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

void PCUI::Numeric::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == NumericData::ID_RANGE) {
        pControl->SetRange(data()->mMin, data()->mMax);
    }
    if (id == ID_REBOUND or id == NumericData::ID_INCREMENT) {
        pControl->SetIncrement(data()->mIncrement);
    }
    if (id == ID_REBOUND or id == NumericData::ID_VALUE) {
        pControl->SetValue(*data());
        refreshSizeAndLayout();
    }
}

void PCUI::Numeric::onModify(wxSpinEvent& evt) {
    data()->mValue = evt.GetPosition();
    data()->update(NumericData::ID_VALUE);
}

void PCUI::Numeric::onModifySecondary(wxCommandEvent&) {
    SetFocusIgnoringChildren();
    pControl->SetFocus();
}

PCUI::Decimal::Decimal(
    wxWindow *parent,
    DecimalData& data,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, data) {
    create(label, orient);
}

PCUI::Decimal::Decimal(
    wxWindow *parent,
    DecimalDataProxy& proxy,
    const wxString& label,
    const wxOrientation& orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void PCUI::Decimal::create(
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

void PCUI::Decimal::onUIUpdate(uint32 id) {
    if (id == ID_REBOUND or id == DecimalData::ID_RANGE) {
        pControl->SetRange(data()->mMin, data()->mMax);
    }
    if (id == ID_REBOUND or id == DecimalData::ID_INCREMENT) {
        pControl->SetIncrement(data()->mIncrement);
    }
    if (id == ID_REBOUND or id == DecimalData::ID_VALUE) {
        pControl->SetValue(*data());
        refreshSizeAndLayout();
    }
}

void PCUI::Decimal::onModify(wxSpinDoubleEvent& evt) {
    data()->mValue = evt.GetValue();
    data()->update(DecimalData::ID_VALUE);
}

void PCUI::Decimal::onModifySecondary(wxCommandEvent&) {
    SetFocusIgnoringChildren();
    pControl->SetFocus();
}

