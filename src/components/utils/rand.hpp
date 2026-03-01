#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/rand.hpp
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

#include <limits>
#include <random>
#include <type_traits>

#include "utils/types.hpp"

#include "utils_export.h"

namespace utils::rand {

/**
 * Can be used for special distributions
 */
UTILS_EXPORT std::mt19937_64& gen();

/**
 * Simple use for random number generation [min, max]
 *
 * @param min lower bound of generation
 * @param max upper bound of generation
 */
template<typename T = uint32>
inline T get(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
    static_assert(std::is_arithmetic_v<T>);
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> distribution(min, max);
        return distribution(gen());
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> distribution(min, max);
        return distribution(gen());
    }
}

} // namespace utils::rand

