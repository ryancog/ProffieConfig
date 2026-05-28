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

struct CommentData {
    /**
     * Stream to read data in from.
     */
    std::istream& stream_;

    /**
     * Output for comment string(s) read.
     */
    std::string out_;

    /**
     * As an input, which types of comments to read (none clears whitespace)
     *
     * As an output, which types were read.
     */
    enum : uint32 {
        eType_None = 0,
        eType_Line = 1U << 0,
        eType_Block = 1U << 1,
    };
    uint32 type_{eType_Line | eType_Block};

    /**
     * Only extract a single comment
     */
    bool single_{false};

    /**
     * Skip over newlines.
     */
    bool skipNewlines_{true};

    /**
     * Skip over spaces.
     */
    bool skipSpaces_{true};
};

/**
 * Clears whitespace (if hit) and parses comment strings according to data
 * params until non-comment content is encountered.
 *
 * @return If a comment was parsed.
 */
[[nodiscard]] UTILS_EXPORT bool extractComments(CommentData&);

/**
 * Evaluate the string for math operations
 *
 * @return Evaluated value
 */
[[nodiscard]] UTILS_EXPORT
std::optional<float64> doStringMath(std::string_view);

} // namespace utils

