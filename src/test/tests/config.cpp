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

#include <string>

#include <catch2/catch_test_macros.hpp>

#include "config/config.hpp"
#include "config/blades/bladeconfig.hpp"
#include "config/buttons/button.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "config/styles/style.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"
#include "utils/paths.hpp"
#include "utils/string.hpp"

namespace {

const fs::path CONFIG_DIR{CONFIG_DIR_STR};

} // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace)
TEST_CASE("Config") {
    for (const auto& entry : fs::directory_iterator(paths::configDir()))
        fs::remove_all(entry);

    config::update();

    SECTION("Tsukuyomi") {
        constexpr cstring CONFIG_NAME{"Tsukuyomi"};

        auto importErr{config::import(
            CONFIG_NAME,
            CONFIG_DIR / (std::string(CONFIG_NAME) + ".h")
        )};
        REQUIRE(importErr == std::nullopt);

        auto listCtxt{data::context(config::list())};
        auto& info{dynamic_cast<config::Info&>(*listCtxt.children()[0])};

        auto loadErr{info.load()};
        REQUIRE(loadErr == std::nullopt);

        const auto& cfg{*info.config()};

        using namespace config;

        {
            REQUIRE(data::context(cfg.settings_.massStorage_).val());
            REQUIRE(data::context(cfg.settings_.webUsb_).val());

            REQUIRE(data::context(cfg.settings_.volume_).val() == 2000);
            REQUIRE(data::context(cfg.settings_.bootVolume_.enable_).val());
            REQUIRE(data::context(cfg.settings_.bootVolume_.value_).val() == 200);
            REQUIRE(data::context(cfg.settings_.clashThreshold_).val() == 2);
            REQUIRE(data::context(cfg.settings_.pliOffTime_).val() == 120);
            REQUIRE(data::context(cfg.settings_.idleOffTime_).val() == 10);
            REQUIRE(data::context(cfg.settings_.motionTimeout_).val() == 15);
        }

        { auto bladeConfigs{data::context(cfg.bladeConfigs_)};
            REQUIRE(bladeConfigs.children().size() == 1);
            auto& bladeConfig{bladeConfigs.child<blades::BladeConfig>(0)};
            auto blades{data::context(bladeConfig.blades_)};
            REQUIRE(blades.children().size() == 7);
        }

        { auto buttons{data::context(cfg.buttons_)};
            REQUIRE(buttons.children().size() == 2);

            const auto& powerButton{buttons.child<buttons::Button>(0)};
            REQUIRE(data::context(powerButton.type_).idx() == eBtn_Type_Pullup);
            REQUIRE(data::context(powerButton.event_).idx() == eBtn_Evt_Power);
            REQUIRE(data::context(powerButton.pin_).val() == "powerButtonPin");
            REQUIRE(data::context(powerButton.name_).val() == "pow");

            const auto& auxButton{buttons.child<buttons::Button>(1)};
            REQUIRE(data::context(auxButton.type_).idx() == eBtn_Type_Pullup);
            REQUIRE(data::context(auxButton.event_).idx() == eBtn_Evt_Aux);
            REQUIRE(data::context(auxButton.pin_).val() == "auxPin");
            REQUIRE(data::context(auxButton.name_).val() == "aux");
        }

        { auto presetArrays{data::context(cfg.presetArrays_)};
            REQUIRE(presetArrays.children().size() == 1);
            auto& presetArray{presetArrays.child<presets::Array>(0)};
            REQUIRE(data::context(presetArray.name_).val() == "presets");

            auto presets{data::context(presetArray.presets_)};
            REQUIRE(presets.children().size() == 35);
            auto& preset{presets.child<presets::Preset>(0)};
            auto styles{data::context(preset.styles_)};
            REQUIRE(styles.children().size() == 9);

            auto& style{styles.child<presets::Style>(6)};
            auto contentStr{data::context(style.content_).val()};
            utils::trimWhitespaceOutsideString(contentStr);
            REQUIRE(contentStr == "StylePtr<PowerRing>()");
        }

        { auto styles{data::context(cfg.styles_)};
            REQUIRE(styles.children().size() == 31);

            auto& style{styles.child<styles::Style>(3)};
            REQUIRE(data::context(style.name_).val() == "PowerRing");
            REQUIRE(data::context(style.comments_).val().empty());
            constexpr cstring CONTENT{"Layers<TransitionLoop<Black,TrConcat<TrWipeIn<600>,RgbArg<BASE_COLOR_ARG,Blue>,TrWipeIn<600>>>>"};
            REQUIRE(style.format(true) == CONTENT);
        }
    }
}

