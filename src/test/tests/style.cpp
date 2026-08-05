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

#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include "config/blades/bladeconfig.hpp"
#include "config/config.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "data/context.hpp"
#include "utils/paths.hpp" // IWYU pragma: keep for fs::path

namespace {

const fs::path CONFIG_DIR{CONFIG_DIR_STR};

const cstring RAW_STYLE_STR{R"(
/* This is a comment outside the style. */
StylePtr< // This is a comment in the style (scary).
    // Another comment line
            MACRO(WithStuffInside),
            SomeSomethingOrOther<>
>("Arg ~ ~"), AndBadStuffOutside
)"};

const cstring PARSED_COMMENT_STR{"This is a comment outside the style."};

// Note the lack of period after "(scary)". Currently, only special characters
// are allowed in block comments
//
// There's also tabs in this string. I forget if there was a real reason for
// using tabs instead of spaces, but there might've been. There's math for
// considering tabs as 4-wide spaces in style::format(), but I don't just use
// spaces...
const cstring PARSED_STYLE_STR{R"(StylePtr<
	// This is a comment in the style (scary)
	// Another comment line
	MACRO(WithStuffInside),
	SomeSomethingOrOther<>
>("Arg ~ ~"))"};

} // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
TEST_CASE("Styles") {
    for (const auto& entry : fs::directory_iterator(paths::configDir()))
        fs::remove_all(entry);

    std::ofstream file{paths::configDir() / "Test.h"};
    REQUIRE(not file.fail());
    file.close();

    config::update();

    auto list{data::context(config::list())};
    auto& info{list.child<config::Info>(0)};
    { auto loadErr{info.load()};
        REQUIRE(not loadErr);

        auto& config{*info.config()};

        auto presetArrays{data::context(config.presetArrays_)};
        auto& presetArray{presetArrays.append<config::presets::Array>(config)};
        // Saving also requires a name
        presetArray.name_.change("presets");

        auto presets{data::context(presetArray.presets_)};
        auto& preset{presets.append<config::presets::Preset>(config)};

        auto styles{data::context(preset.styles_)};
        auto& style{styles.append<config::presets::Style>(config)};

        auto content{data::context(style.content_)};
        auto comments{data::context(style.comment_)};

        comments.clear();
        content.change(RAW_STYLE_STR);
        content.change(style.format());

        REQUIRE(comments.val() == PARSED_COMMENT_STR);
        REQUIRE(content.val() == PARSED_STYLE_STR);

        // Board needs to be selected to save
        // 0 should be most recent.
        config.boardChoice().choose(0);

        // Add a blade array and blade so the preset is used.
        auto bladeConfigs{data::context(config.bladeConfigs_)};
        auto& bladeConfig{bladeConfigs.append<config::blades::BladeConfig>(config)};
        bladeConfig.presetArray_.choice().choose(0);
        auto blades{data::context(bladeConfig.blades_)};
        auto& blade{blades.append<config::blades::Blade>(config)};
    }

    // Save and reload and make sure the style was reparsed correctly, since it
    // has to make it through the whole parsing pipeline unscathed, as opposed
    // to just the field filtering.
    auto saveErr{info.save()};
    REQUIRE(not saveErr);

    info.unload();
    { auto loadErr{info.load()};
        REQUIRE(not loadErr);

        auto& config{*info.config()};

        auto presetArrays{data::context(config.presetArrays_)};
        REQUIRE(presetArrays.children().size() == 1);
        auto& presetArray{presetArrays.child<config::presets::Array>(0)};

        auto presets{data::context(presetArray.presets_)};
        REQUIRE(presets.children().size() == 1);
        auto& preset{presets.child<config::presets::Preset>(0)};

        auto styles{data::context(preset.styles_)};
        REQUIRE(styles.children().size() == 1);
        auto& style{styles.child<config::presets::Style>(0)};

        auto content{data::context(style.content_)};
        auto comments{data::context(style.comment_)};

        REQUIRE(comments.val() == PARSED_COMMENT_STR);
        REQUIRE(content.val() == PARSED_STYLE_STR);
    }
}

