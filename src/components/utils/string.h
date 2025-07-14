#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/utils/string.h
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

#include <algorithm>
#include <cctype>

namespace Utils {

template<typename STRING>
constexpr void trimWhiteSpace(STRING& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char chr) {
        return not std::isspace(chr);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char chr) {
        return not std::isspace(chr);
    }).base(), str.end());
};

} // namespace Utils
