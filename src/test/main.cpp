/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * test/main.cpp
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

#include <catch2/catch_test_macros.hpp>

#include "utils/crypto.h"

// Catch2 uses `static` keyword internally
// NOLINTBEGIN(misc-use-anonymous-namespace)

TEST_CASE("Crypto::Hash string") {
    constexpr cstring HASH_STR{"3925fc3ca17db9b69c3ff8d456965ccc838baed6088aab52c1148e757258b077"};
    auto hash{Crypto::Hash::parseString(HASH_STR)};

    REQUIRE(hash);
    REQUIRE(static_cast<string>(*hash) == HASH_STR);
}

// NOLINTEND(misc-use-anonymous-namespace)

