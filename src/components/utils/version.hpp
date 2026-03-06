#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/utils/version.hpp
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

#include <compare>
#include <utility>
#include <string>
#include <string_view>

#include "utils/types.hpp"

#include "utils_export.h"

namespace utils {

struct UTILS_EXPORT Version {
    // Version is valid if `err == NONE`
    enum class Err : uint8 {
        None,
        Invalid,
        Num_Range,
        Str_Invalid,
        Str_Empty,
        Unknown,
    };

    enum class CompMode : uint8 {
        /**
         * Must match exactly.
         */
        Exact,
        /**
         * Will match any. i.e. 'x' or '*'
         */
        Permissive,
    };

    struct VerNum {
        // Another clang bug w/ implicit ctor :/
        // NOLINTNEXTLINE(modernize-use-equals-default)
        constexpr VerNum() {}
        constexpr VerNum(uint8 v) : val_{v} {}

        CompMode mode_{CompMode::Exact};
        uint8 val_{0};
    };

    struct Tag {
        // NOLINTNEXTLINE(modernize-use-equals-default)
        constexpr Tag() {}
        constexpr Tag(std::string s) : val_{std::move(s)} {}

        CompMode mode_{CompMode::Exact};
        std::string val_;
    };

    /**
     * Construct a Version from a string.
     *
     * Uses the form: "[major](.[minor])(.[bugfix])(-[tag])".
     * - "-[tag]", minor, and bugfix are optional
     * - tag is a string unbroken by space
     * - major, minor, and bugfix are positive ints [0-255]
     *
     * Unpopulated nums are set to 0/Exact
     */
    Version(std::string_view str);
    constexpr Version(
        VerNum major = {}, VerNum minor = {}, VerNum bugfix = {}, Tag tag = {}
    ) : major_{major}, minor_{minor}, bugfix_{bugfix}, tag_{std::move(tag)} {}

    static Version invalid();

    Err err_{Err::None};

    VerNum major_;
    VerNum minor_;
    VerNum bugfix_;

    /**
     * Used for (optional) tagging of specialized versions. e.g 1.0.0-dev.
     * "dev" would be the tag.
     */
    Tag tag_;

    /**
     * Compare another version to this one, evaluated according to the
     * semantics of *this* version.
     *
     * Permissive values are evaluated less than exacts.
     *
     * Tags must be exact.
     *
     * Ex: 
     * `this`="v1.8.x", v1.8.1 compares equal, however
     * `this`=""
     * v1.8.1 compared to v1.8.x
     */
    [[nodiscard]] std::strong_ordering compare(const Version&) const;

    /**
     * Strictly compare the two items based on data content rather than version
     * semantics.
     */
    auto operator==(const Version&) const = delete;
    std::strong_ordering operator<=>(const Version&) const;

    /**
     * If the Version refers to a distinct, singular version.
     */
    [[nodiscard]] bool isExact() const;

    /**
     * Convert a Version into string representation.
     *
     * If valid, it returns [major](.[minor])(.[bugfix])(-[tag]).
     * If invalid, it returns a string form of the `Err`
     */
    operator std::string() const;
    operator bool() const;
};

} // namespace utils

