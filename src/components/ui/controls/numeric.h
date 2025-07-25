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

#include <type_traits>

#include <wx/spinctrl.h>

#include "base.h"
#include "../private/export.h"

namespace PCUI {

class Numeric;
class Decimal;

namespace Private {

template<typename T> requires std::is_arithmetic_v<T>
struct UI_EXPORT NumericDataTemplate : ControlData {
    operator T() const { return mValue; }
    void operator=(T val);

    [[nodiscard]] T min() const { return mMin; }
    [[nodiscard]] T max() const { return mMax; }
    void setRange(T min, T max);

    [[nodiscard]] T increment() const { return mIncrement; }
    void setIncrement(T inc);

    enum {
        ID_VALUE,
        ID_RANGE,
        ID_INCREMENT,
    };

private:
    friend class PCUI::Numeric;
    friend class PCUI::Decimal;
    T mValue{0};
    T mMin{0};
    T mMax{10};
    T mIncrement{std::is_integral_v<T> ? static_cast<T>(1) : static_cast<T>(0.1)};

};

} // namespace Private

using NumericData = Private::NumericDataTemplate<int32>;
using DecimalData = Private::NumericDataTemplate<float64>;
using NumericDataProxy = ControlDataProxy<NumericData>;
using DecimalDataProxy = ControlDataProxy<DecimalData>;

/**
 * wxALIGN and wxSP_WRAP are the only reasonable styles to pass
 */
class UI_EXPORT Numeric : public ControlBase<
                          Numeric,
                          NumericData,
                          wxSpinCtrl,
                          wxSpinEvent,
                          wxCommandEvent> {
public:
    Numeric(
        wxWindow *parent,
        NumericData& data,
        int64 style = 0,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
    );
    Numeric(
        wxWindow *parent,
        NumericDataProxy& proxy,
        int64 style = 0,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
    );

private:
    void create(
        int64 style,
        const wxString& label,
        const wxOrientation& orient
    );
    void onUIUpdate(uint32) final;
    void onModify(wxSpinEvent&) final;
    void onModifySecondary(wxCommandEvent&) final;
};

class UI_EXPORT Decimal : public ControlBase<
                          Decimal,
                          DecimalData,
                          wxSpinCtrlDouble,
                          wxSpinDoubleEvent,
                          wxCommandEvent> {
public:
    Decimal(
        wxWindow *parent,
        DecimalData& data,
        int64 style = 0,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
    );
    Decimal(
        wxWindow *parent,
        DecimalDataProxy& proxy,
        int64 style = 0,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
    );

private:
    void create(
        int64 style,
        const wxString& label,
        const wxOrientation& orient
    );
    void onUIUpdate(uint32) final;
    void onModify(wxSpinDoubleEvent&) final;
    void onModifySecondary(wxCommandEvent&) final;
};

} // namespace PCUI
