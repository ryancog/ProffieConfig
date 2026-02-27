#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/utils/string.hpp
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

#include <cctype>
#include <optional>
#include <string>
#include <string_view>

#include <wx/string.h>

#include "utils/types.hpp"

#include "utils_export.h"

namespace utils {

/**
 * Purge all whitespace in string
 */
UTILS_EXPORT void trimWhitespace(std::string& str);

/**
 * Purge all whitespace in a string that is not surrounded by quotes
 *
 * Whitespace is ' ' only. Other whitespace is still trimmed
 */
UTILS_EXPORT void trimWhitespaceOutsideString(std::string& str);

/**
 * Purge whitespace around visible chars.
 */
UTILS_EXPORT void trimSurroundingWhitespace(std::string& str);

/**
 * Trim characters unsafe for a C++ variable name
 *
 * @param str Reference to string to trim
 * @param[out] numTrimmed The number of characters trimmed
 * @param countTrimIndex The index before which trims should be counted.
 * @param allowNum If a digit is first-encountered, processing switches to ensuring *only* num.
 */
UTILS_EXPORT void trimCppName(
    std::string& str,
    bool allowNum = false,
    uint32 *numTrimmed = nullptr,
    uint32 countTrimIndex = -1
);

struct TrimRules {
    bool allowAlpha{false};
    bool allowNum{false};
    std::string_view safeList;
};

/**
 * Trim characters for a generic field.
 */
UTILS_EXPORT void trim(
    std::string& str,
    TrimRules rules,
    uint32 *numTrimmed = nullptr,
    uint32 countTrimIndex = -1
);

/**
 * Remove all non-digit values, remove leading 0
 *
 * @param str Reference to string to trim
 * @param[out] numTrimmed The number of characters trimmed
 * @param countTrimIndex The index before which trims should be counted.
 */
UTILS_EXPORT void trimForNumeric(
    std::string& str,
    uint32 *numTrimmed = nullptr,
    uint32 countTrimIndex = -1
);

/**
 * Clears whitespace (if hit) and parses all comment strings encountered until
 * non-space is encountered.
 *
 * @param stream Stream to read from.
 *
 * @return comment(s) extracted, nullopt if none
 */
[[nodiscard]] UTILS_EXPORT
std::optional<std::string> extractComment(std::istream& stream);

/**
 * Similar to extractComment(), however comment data is not parsed.
 * All comment data (including start/end conditions) are directly forwarded to
 * the string (if provided).
 *
 * stream positions ends at non-comment data.
 *
 * @return Whether anything was skipped.
 */
UTILS_EXPORT bool skipComment(std::istream& stream, std::string *str = nullptr);

[[nodiscard]] UTILS_EXPORT
std::vector<std::string> createEntries(const std::vector<wxString>& vec);

[[nodiscard]] UTILS_EXPORT
std::vector<std::string> createEntries(
    const std::initializer_list<wxString>& list
);

/**
 * Evaluate the string for math operations
 *
 * @return Evaluated value
 */
[[nodiscard]] UTILS_EXPORT
std::optional<float64> doStringMath(const std::string&);

} // namespace utils

