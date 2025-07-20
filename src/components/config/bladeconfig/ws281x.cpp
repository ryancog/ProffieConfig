#include "ws281x.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/ws281x.cpp
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

Config::WS281XBlade::WS281XBlade() {
    powerPins.setItems(Utils::createEntries({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    }));
    dataPin.setDefaults(Utils::createEntries({
        "bladePin",
        "blade2Pin",
        "blade3Pin",
        "blade4Pin"
    }));
    length.setRange(0, 1000);
    length = 144;
}

Config::Split::Split() {
    type.init(Utils::createEntries({
        _("Standard SubBlade"),
        _("Stride SubBlade"),
        _("ZigZag SubBlade"),         
    }));
    segments.setRange(2, 6);
    // Blissfully ignorant of length handling

}

