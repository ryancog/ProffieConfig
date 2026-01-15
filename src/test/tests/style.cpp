/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * test/tests/style.cpp
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

#include "config/preset/preset.h"
#include "utils/string.h"

namespace {

const fs::path CONFIG_DIR{CONFIG_DIR_STR};

} // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
TEST_CASE("Styles") {
    const cstring RAW_STYLE_STR{
        "/* This is a comment outside the style. */           \n"
        "StylePtr< // This is a comment in the style (scary). \n"
        "  // Another comment line                            \n"
        "    SomeSomethingOrOther<>                           \n"
        ">(\"Arg ~ ~\")                                       \n"
    };
    const cstring COMMENT_STR{
        "This is a comment outside the style.\n"
        "This is a comment in the style (scary).\n"
        "Another comment line"
    };
    const cstring STYLE_STR{R"(StylePtr<SomeSomethingOrOther<>>("Arg ~ ~"))"};

    Config::Preset::Style style;
    style.comment.clear();
    style.style = RAW_STYLE_STR;
    string styleStr{style.style};
    Utils::trimWhitespaceOutsideString(styleStr);

    REQUIRE(styleStr == STYLE_STR);
    REQUIRE(static_cast<string>(style.comment) == COMMENT_STR);
}

