#include "number.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/number.cpp
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

#include <mutex>

using namespace data::prim;

template <typename T>
bool detail::Number<T>::set(T val) {
    std::lock_guard scopeLock(*this);

    if (not Number<T>::setupSet(val)) return false;

    Number<T>::doSet(val);
    return true;
}

template <typename T>
bool detail::Number<T>::update(Number<T>::Params params) {
    std::lock_guard scopeLock(*this);

    if (not Number<T>::setupUpdate(params)) return false;

    Number<T>::doUpdate(params);
    return true;
}

template struct detail::Number<int32>;
template struct detail::Number<float64>;

