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

namespace {

const fs::path CONFIG_DIR{CONFIG_DIR_STR};

} // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
TEST_CASE("Config") {
    variant<Config::Config *, string> configRes;

    SECTION("Tsukuyomi") {
        configRes = Config::open(CONFIG_DIR / "Tsukuyomi");
        REQUIRE(configRes.index() == 0);
        const auto *tsukuyomi{std::get<Config::Config *>(configRes)};

        REQUIRE(tsukuyomi->settings.massStorage);
        REQUIRE(tsukuyomi->settings.webUSB);

        REQUIRE(tsukuyomi->settings.volume == 2000);
        REQUIRE(tsukuyomi->settings.bootVolume == 200);
        REQUIRE(tsukuyomi->settings.clashThreshold == 2);
        REQUIRE(tsukuyomi->settings.pliOffTime == 120);
        REQUIRE(tsukuyomi->settings.idleOffTime == 10);
        REQUIRE(tsukuyomi->settings.motionTimeout == 15);

        REQUIRE(tsukuyomi->bladeArrays.arrays().size() == 1);
        REQUIRE(tsukuyomi->bladeArrays.arrays()[0]->blades().size() == 7);

        REQUIRE(tsukuyomi->settings.buttons().size() == 2);
        const auto& powerButton{tsukuyomi->settings.button(0)};
        REQUIRE(powerButton->type == Config::BUTTON);
        REQUIRE(powerButton->event == Config::POWER);
        REQUIRE(powerButton->pin == "powerButtonPin");
        REQUIRE(powerButton->name == "pow");
        const auto& auxButton{tsukuyomi->settings.button(1)};
        REQUIRE(auxButton->type == Config::BUTTON);
        REQUIRE(auxButton->event == Config::AUX);
        REQUIRE(auxButton->pin == "auxPin");
        REQUIRE(auxButton->name == "aux");

        constexpr cstring POWER_RING_EXPANDED_STR{"StylePtr<Layers<TransitionLoop<Black,TrConcat<TrWipeIn<600>,RgbArg<BASE_COLOR_ARG,Blue>,TrWipeIn<600>>>>>()"};
        REQUIRE(tsukuyomi->presetArrays.arrays().size() == 1);
        REQUIRE(tsukuyomi->presetArrays.arrays()[0]->name == "presets");
        const auto& presets{tsukuyomi->presetArrays.arrays()[0]->presets()};
        REQUIRE(presets.size() == 35);
        REQUIRE(presets[0]->styles().size() == 9);
        const string RgueCmdrStyleStr{presets[0]->styles()[6]->style};
        REQUIRE(RgueCmdrStyleStr == POWER_RING_EXPANDED_STR);
    }
}

