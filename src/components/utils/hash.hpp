#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/hash.hpp
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

#include <array>
#include <iostream>
#include <optional>
#include <string>

#include "utils/types.hpp"

#include "utils_export.h"

namespace utils::hash {

/**
 * SHA256 Hash
 */
struct UTILS_EXPORT SHA256 {
    /**
     * Raw initialization
     */
    SHA256(std::array<uint8, 32>);

    /**
     * Compute the hash of the given stream.
     *
     * If a file, should be opened as binary.
     */
    static SHA256 stream(std::istream&);

    /**
     * Does not compute the hash of the string, but rather parses it as a
     * hex-encoded 256-bit value.
     */
    static std::optional<SHA256> parseString(const std::string&);

    [[nodiscard]] std::array<uint8, 32> value() const;
    explicit operator std::string() const;

    auto operator<=>(const SHA256&) const = default;

private:
    std::array<uint8, 32> mValue;
};

} // namespace utils::hash

