#include "simple.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/simple.cpp
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

#include "utils/string.h"

Config::SimpleBlade::Star::Star() {
    led.setChoices(Utils::createEntries({
        _("<None>"),
        _("Cree Red"),
        _("Cree Green"),
        _("Cree Blue"),
        _("Cree Amber"),
        _("Cree Red-Orange"),
        _("Cree White"),
        _("Red"),
        _("Green"),
        _("Blue"),
    }));
    powerPin.setDefaults(Utils::createEntries({
        "bladePin",
        "blade2Pin",
        "blade3Pin",
        "blade4Pin",
    }));
    resistance.setRange(0, 10000);
}

// wxString BladesPage::ledToConfigStr(LED led) {
//     switch (led) {
//         case CREE_RED:
//             return "CreeXPE2RedTemplate";
//         case CREE_GREEN:
//             return "CreeXPE2GreenTemplate";
//         case CREE_BLUE:
//             return "CreeXPE2BlueTemplate";
//         case CREE_AMBER:
//             return "CreeXPE2AmberTemplate";
//         case CREE_RED_ORANGE:
//             return "CreeXPE2RedOrangeTemplate";
//         case CREE_WHITE:
//             return "CreeXPE2WhiteTemplate";
//         case RED:
//             return "CH1LED";
//         case GREEN:
//             return "CH2LED";
//         case BLUE:
//             return "CH3LED";
//         case NONE:
//         default:
//             return "NoLED";
//     }
// }

