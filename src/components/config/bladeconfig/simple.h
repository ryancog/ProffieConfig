#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/bladeconfig/simple.h
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

#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/numeric.h"

#include "config_export.h"

namespace Config {

struct CONFIG_EXPORT SimpleBlade {
    SimpleBlade();

    struct Star {
        Star();

        enum LED {
            NONE,

            CREE_RED,
            CREE_GREEN,
            CREE_BLUE,
            CREE_AMBER,
            CREE_RED_ORANGE,
            CREE_WHITE,

            RED,
            GREEN,
            BLUE,

            LED_MAX,

            USE_RESISTANCE_START = CREE_RED,
            USE_RESISTANCE_END = CREE_WHITE,
        };
        static constexpr array<string_view, LED_MAX> LED_STRS{
            "NoLED",
            "CreeXPE2RedTemplate",
            "CreeXPE2GreenTemplate",
            "CreeXPE2BlueTemplate",
            "CreeXPE2AmberTemplate",
            "CreeXPE2RedOrangeTemplate",
            "CreeXPE2WhiteTemplate",
            "CH1LED",
            "CH2LED",
            "CH3LED",
        };
        pcui::ChoiceData led;
        pcui::ComboBoxData powerPin;
        pcui::NumericData resistance;
    };

    Star star1;
    Star star2;
    Star star3;
    Star star4;
};

} // namespace Config

