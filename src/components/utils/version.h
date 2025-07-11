#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/utils/version.h
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

#include "utils/types.h"

#include "private/export.h"

namespace Utils {

struct UTILS_EXPORT Version {
    Version() = default;

    /**
     * Construct a Version from a string.
     *
     * Uses the form: "[major](.[minor])(.[bugfix])(-[tag])".
     * - "-[tag]", minor, and bugfix are optional
     * - tag is a string unbroken by space
     * - major, minor, and bugfix are positive ints [0-255]
     */
    Version(string_view str);

    static Version invalidObject();

    uint8 major{0};
    uint8 minor{0};
    uint8 bugfix{0};

    // Version is valid if `err == NONE`
    enum Err : uint8 {
        NONE,
        INVALID,
        NUM_RANGE,
        STR_INVALID,
        STR_EMPTY,
    } err{Err::NONE};

    /**
     * Used for (optional) tagging of specialized versions. e.g 1.0.0-dev.
     * "dev" would be the tag.
     */
    string tag;

    auto operator<=>(const Version&) const = default;

    /**
     * Convert a Version into string representation.
     *
     * If valid, it returns [major](.[minor])(.[bugfix])(-[tag]).
     * If invalid, it returns a string form of the `Err`
     */
    operator string() const;
    operator bool() const;
};

} // namespace Utils
