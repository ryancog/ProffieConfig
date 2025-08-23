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

#include <utility>

#include "utils/types.h"

#include "utils_export.h"

namespace Utils {

struct UTILS_EXPORT Version {
    Version() = default;

    /**
     * Construct a Version from a string.
     *
     * Uses the form: "[major](.[minor])(.[bugfix])(-[tag])".
     * - "-[tag]", minor, and bugfix are optional
     * - tag is a string unbroken by space
     * - major, minor, and bugfix are positive ints
     *   - major [0-256]
     *   - minor and bugfix [0-127]
     */
    Version(string_view str);
    Version(uint8 major, int8 minor = NULL_REV, int8 bugfix = NULL_REV, string tag = {}) :
        major{major}, minor{minor}, bugfix{bugfix}, tag{std::move(tag)} {};

    static const Version& invalidObject();

    // Version is valid if `err == NONE`
    enum Err : uint8 {
        NONE,
        INVALID,
        NUM_RANGE,
        STR_INVALID,
        STR_EMPTY,
        UNKNOWN,
    } err{Err::NONE};

    static constexpr int8 NULL_REV{-1};

    uint8 major{0};
    int8 minor{NULL_REV};
    int8 bugfix{NULL_REV};

    /**
     * Used for (optional) tagging of specialized versions. e.g 1.0.0-dev.
     * "dev" would be the tag.
     */
    string tag;

    /**
     * Compare versions
     *
     * This comparison is based on semantic versioning, and is
     * invalid for when err != Err::NONE.
     *
     * Obviously since this uses the default, tags are compared as
     * strings.
     *
     * NULL_REV is -1 so that v1.1 < v1.1.0, and only v1.2 > v1.1.0 (e.g. next parent ver)
     */
    auto operator<=>(const Version&) const = default;

    /**
     * Convert a Version into string representation.
     *
     * If valid, it returns [major](.[minor])(.[bugfix])(-[tag]).
     * If invalid, it returns a string form of the `Err`
     */
    operator string() const;
    operator bool() const;

    bool operator==(const string_view&) const;
};

} // namespace Utils
