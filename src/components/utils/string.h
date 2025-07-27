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

/**
 * @param str Reference to string to trim
 * @param[out] numTrimmed The number of characters trimmed
 * @param countTrimIndex The index before which trims should be counted.
 * @param moreSafe more safe characters
 */
template<typename STRING>
constexpr void trimUnsafe(
    STRING& str,
    uint32 *numTrimmed = nullptr,
    uint32 countTrimIndex = std::numeric_limits<uint32>::max(),
    string_view moreSafe = {}
) {
    if (numTrimmed) *numTrimmed = 0;

    auto checkIllegal{[numTrimmed, &countTrimIndex, moreSafe](char chr) -> bool {
        if (std::isalnum(chr)) return false;
        if (chr == '_') return false;
        if (moreSafe.find(chr) != std::string::npos) return false;

        if (numTrimmed and countTrimIndex > 0) {
            ++*numTrimmed;
            --countTrimIndex;
        }
        return true;
    }};

    while (not str.empty() and std::isdigit(str[0])) {
        if (numTrimmed and countTrimIndex > 0) {
            ++*numTrimmed;
            --countTrimIndex;
        }
        str.erase(0, 1);
    }

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
