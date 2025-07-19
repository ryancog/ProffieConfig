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

#include <wx/string.h>

#include "utils/types.h"

#include "private/export.h"

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

template<typename STRING>
constexpr void trimUnsafe(STRING& str) {
    auto checkIllegal{[](char chr) -> bool {
        if (std::isalnum(chr)) return false;
        if (chr == '-' or chr == '_') return false;
        return true;
    }};

    str.erase(
        std::remove_if(str.begin(), str.end(), checkIllegal),
        str.end()
    );
};

UTILS_EXPORT vector<string> createEntries(const std::vector<wxString>& vec);
UTILS_EXPORT vector<string> createEntries(const std::initializer_list<wxString>& list);

template<typename T, size_t SIZE>
vector<string> createEntries(const array<T, SIZE>& list) {
    vector<string> entries;
    for (const auto& entry : list) {
        entries.push_back(string{entry});
    }
    return entries;
}

} // namespace Utils
