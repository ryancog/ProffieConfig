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

Config::Split::Split() {
    type.init(Utils::createEntries({
        _("Standard SubBlade"),
        _("Stride SubBlade"),
        _("ZigZag SubBlade"),         
    }));
    segments.setRange(2, 6);
    // Blissfully ignorant of length handling

}

Config::WS281XBlade::WS281XBlade() {
    hasWhite.setUpdateHandler([this](uint32 id) {
        if (id != hasWhite.ID_VALUE) return;

        colorOrder3.show(not hasWhite);
        if (not hasWhite) {
            auto newOrder3{static_cast<int32>(colorOrder4)};
            if (newOrder3 > ORDER4_WFIRST_START and newOrder3 < ORDER4_WFIRST_END) {
                newOrder3 -= ORDER4_WFIRST_START;
            }

            colorOrder3 = newOrder3;
        }
        colorOrder4.show(hasWhite);
        if (hasWhite) {
            colorOrder4 = static_cast<int32>(colorOrder3);
        }
        useRGBWithWhite.show(hasWhite);
    });
    dataPin.setUpdateHandler([this](uint32 id) {
        if (id != dataPin.ID_VALUE) return;

        auto rawValue{static_cast<string>(dataPin)};
        uint32 numTrimmed{};
        auto insertionPoint{dataPin.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            {},
            true
        );

        if (rawValue == static_cast<string>(dataPin)) {
            return;
        }
        
        dataPin = std::move(rawValue);
        dataPin.setInsertionPoint(insertionPoint - numTrimmed);
    });
    powerPins.setUpdateHandler([this](uint32 id) {
        if (id != powerPins.ID_SELECTION) return;

        auto selected{static_cast<set<uint32>>(powerPins)};
        auto items{powerPins.items()};
        for (auto idx{6}; idx < items.size(); ++idx) {
            if (selected.find(idx) == selected.end()) {
                items.erase(std::next(items.begin(), idx));
                --idx;
            }
        }
        powerPins.setItems(std::move(items));
    });

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
    colorOrder3.setChoices(Utils::createEntries({
        _("GRB"),
        _("GBR"),
        _("BGR"),
        _("BRG"),
        _("RGB"),
        _("RBG"),
    }));
    colorOrder3 = GRB;
    colorOrder4.setChoices(Utils::createEntries({
        _("GRBW"),
        _("GBRW"),
        _("BGRW"),
        _("BRGW"),
        _("RGBW"),
        _("RBGW"),
        _("WGRB"),
        _("WGBR"),
        _("WBGR"),
        _("WBRG"),
        _("WRGB"),
        _("WRBG"),
    }));
    colorOrder4 = GRBW;

    hasWhite.setValue(false);
}

