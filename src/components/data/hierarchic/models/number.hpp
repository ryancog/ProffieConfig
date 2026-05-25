#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/number.hpp
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

#include "data/base/models/number.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

namespace detail {

template <typename T>
struct DATA_EXPORT Number : base::detail::Number<T>, Model {
    struct SetAction;
    struct UpdateAction;

    Number(Root&);
    Number(const Number&, Root&);

    bool set(T) override;
    bool update(Number<T>::Params) override;

protected:
    uint64 hashThis() const override;
};

template <typename T>
struct DATA_EXPORT Number<T>::SetAction : Action {
    SetAction(T);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    T mValue;
};

template <typename T>
struct DATA_EXPORT Number<T>::UpdateAction : Action {
    UpdateAction(Number<T>::Params);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    Number<T>::Params mParams;
    T mValue;
};

} // namespace detail

using Integer = detail::Number<int32>;
using Decimal = detail::Number<float64>;

} // namespace data::hier

