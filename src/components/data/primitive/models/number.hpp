#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/number.hpp
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
#include "data/primitive/model.hpp"

#include "data_export.h"

namespace data::prim {

namespace detail {

template <typename T>
struct DATA_EXPORT Number : base::detail::Number<T>, Model {
    bool set(T) override;
    bool update(typename Number<T>::Params) override;
};

} // namespace detail

using Integer = detail::Number<int32>;
using Decimal = detail::Number<float64>;

} // namespace data::prim

