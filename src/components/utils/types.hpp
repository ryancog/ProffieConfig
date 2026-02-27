#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/types.hpp
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

#include <cstdint>
#include <type_traits>

// "Standard" types for ProffieConfig

// NOLINTBEGIN(readability-identifier-naming)
using int64 = int64_t;
using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;

using uint64 = uint64_t;
using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;

using size = size_t;
using ssize = std::make_signed_t<size>;

// These float sizes technically aren't 100% guaranteed, but
// afaics on modern desktop this should always be the case...
using float32 = float;
using float64 = double;
using float128 = long double;

static_assert(sizeof(float32) == 4);
static_assert(sizeof(float64) == 8);
static_assert(sizeof(float128) == 16);

using cstring = const char *;
// NOLINTEND(readability-identifier-naming)

