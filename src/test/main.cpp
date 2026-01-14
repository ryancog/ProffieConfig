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

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <wx/app.h>

#include "app/app.h"
#include "config/config.h"
#include "config/info.h"
#include "ui/message.h"
#include "utils/crypto.h"

namespace {

const fs::path CONFIG_DIR{CONFIG_DIR_STR};

} // namespace

// Catch2 uses `static` keyword internally
// NOLINTBEGIN(misc-use-anonymous-namespace)

TEST_CASE("Crypto::Hash string") {
    constexpr cstring HASH_STR{"3925fc3ca17db9b69c3ff8d456965ccc838baed6088aab52c1148e757258b077"};
    auto hash{Crypto::Hash::parseString(HASH_STR)};

    REQUIRE(hash);
    REQUIRE(static_cast<string>(*hash) == HASH_STR);
}

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
    }
}

// NOLINTEND(misc-use-anonymous-namespace)


class Test : public wxApp {
public:
    bool OnInit() override {
        if (not App::init("Test")) {
            PCUI::showMessage(_("Test is Already Running"), App::getAppName());
            return false;
        }

        Config::setExecutableVersion(wxSTRINGIZE(VERSION));
        Versions::loadLocal();

        auto res{Catch::Session().run(argc, static_cast<char **>(argv))};

        return false;
    }
};

wxIMPLEMENT_APP(Test);

