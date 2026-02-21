/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * test/tests/config.cpp
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

#include "config/config.h"
#include "utils/string.h"

namespace {

const fs::path CONFIG_DIR{CONFIG_DIR_STR};

} // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
TEST_CASE("Config") {
    variant<Config::Config *, string> configRes;

    SECTION("Tsukuyomi") {
        constexpr cstring CONFIG_NAME{"Tsukuyomi"};
        Config::remove(CONFIG_NAME);
        auto importErr{Config::import(
            CONFIG_NAME,
            CONFIG_DIR / (string{CONFIG_NAME} + ".h")
        )};
        REQUIRE(importErr == nullopt);

        configRes = Config::open(CONFIG_NAME);
        REQUIRE(configRes.index() == 0);
        const auto *cfg{std::get<Config::Config *>(configRes)};

        REQUIRE(cfg->settings.massStorage);
        REQUIRE(cfg->settings.webUSB);

        REQUIRE(cfg->settings.volume == 2000);
        REQUIRE(cfg->settings.bootVolume == 200);
        REQUIRE(cfg->settings.clashThreshold == 2);
        REQUIRE(cfg->settings.pliOffTime == 120);
        REQUIRE(cfg->settings.idleOffTime == 10);
        REQUIRE(cfg->settings.motionTimeout == 15);

        REQUIRE(cfg->bladeArrays.arrays().size() == 1);
        REQUIRE(cfg->bladeArrays.arrays()[0]->blades().size() == 7);

        REQUIRE(cfg->settings.buttons().size() == 2);
        const auto& powerButton{cfg->settings.button(0)};
        REQUIRE(powerButton->type == Config::BUTTON);
        REQUIRE(powerButton->event == Config::POWER);
        REQUIRE(powerButton->pin == "powerButtonPin");
        REQUIRE(powerButton->name == "pow");
        const auto& auxButton{cfg->settings.button(1)};
        REQUIRE(auxButton->type == Config::BUTTON);
        REQUIRE(auxButton->event == Config::AUX);
        REQUIRE(auxButton->pin == "auxPin");
        REQUIRE(auxButton->name == "aux");

        constexpr cstring POWER_RING_EXPANDED_STR{"StylePtr<Layers<TransitionLoop<Black,TrConcat<TrWipeIn<600>,RgbArg<BASE_COLOR_ARG,Blue>,TrWipeIn<600>>>>>()"};
        REQUIRE(cfg->presetArrays.arrays().size() == 1);
        REQUIRE(cfg->presetArrays.arrays()[0]->name == "presets");
        const auto& presets{cfg->presetArrays.arrays()[0]->presets()};
        REQUIRE(presets.size() == 35);
        REQUIRE(presets[0]->styles().size() == 9);
        string RgueCmdrStyleStr{presets[0]->styles()[6]->style};
        Utils::trimWhitespaceOutsideString(RgueCmdrStyleStr);
        REQUIRE(RgueCmdrStyleStr == POWER_RING_EXPANDED_STR);
    }

    SECTION("my_saber2") {
        constexpr cstring CONFIG_NAME{"my_saber2"};
        Config::remove(CONFIG_NAME);
        auto importErr{Config::import(
            CONFIG_NAME,
            CONFIG_DIR / (string{CONFIG_NAME} + ".h")
        )};
        REQUIRE(importErr == nullopt);
    }
}

