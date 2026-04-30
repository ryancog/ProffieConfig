#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/context.hpp
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

#include "data_export.h"

namespace data {

template <typename T>
auto context(T& t) {
    if constexpr (std::is_const_v<T>) {
        return typename T::ROContext(t);
    } else {
        return typename T::Context(t);
    }
}

/*
This would be nicer but C++ doesn't support it I guess.

template <typename T>
using Context = std::conditional_t<
    std::is_const_v<T>, typename T::ROContext, typename T::Context
>;

template <typename T>
Context(T&) -> Context<T>;

template <typename T>
Context(const T&) -> Context<const T>;
*/

} // namespace data

