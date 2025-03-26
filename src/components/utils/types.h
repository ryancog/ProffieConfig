#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/types.h
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

#include <any>
#include <array>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <list>
#include <optional>
#include <vector>

#include <wx/string.h>

// "Standard" types for ProffieConfig

using int64 = int64_t;
using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;
using uint64 = uint64_t;
using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;

// These float sizes technically aren't 100% guaranteed, but
// afaics on modern desktop this should always be the case...
using float32 = float;
using float64 = double;
using float128 = long double;

using byte = uint8;

using cstring = const char *;

using std::any;
using std::array;
using std::deque;
using std::function;
using std::list;
using std::nullopt;
using std::optional;
using string = wxString;
using std::vector;
namespace fs = std::filesystem;
using filepath = fs::path;
