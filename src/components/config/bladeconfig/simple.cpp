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

Config::SimpleBlade::SimpleBlade() {

}

Config::SimpleBlade::Star::Star() {
    led.setUpdateHandler([this](uint32 id) {
        if (id != led.ID_SELECTION) return;

        powerPin.enable(led != NONE);
        resistance.enable(led != NONE and led >= USE_RESISTANCE_START and led <= USE_RESISTANCE_END);
    });

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
    led.setValue(NONE);
    powerPin.setDefaults(Utils::createEntries({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    }));
    resistance.setRange(0, 10000);
    resistance.setIncrement(50);
}
