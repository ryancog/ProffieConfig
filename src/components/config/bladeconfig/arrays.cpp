#include "arrays.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/arrays.cpp
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

#include "ws281x.h"

Config::BladeArrays::BladeArrays() :
    subBladeTypeProxy{Split::TYPE_MAX} {

    arraySelection.setUpdateHandler([this](uint32 id) {
        if (id != arraySelection.ID_SELECTION) return;

        if (arraySelection == -1) {
            bladeSelectionProxy.unbind();
            subBladeSelectionProxy.unbind();

            bladeTypeProxy.unbind();
            powerPinProxy.unbind();
            powerPinNameEntry.hide();

            colorOrder3Proxy.unbind();
            colorOrder4Proxy.unbind();
            hasWhiteProxy.unbind();
            useRGBWithWhiteProxy.unbind();

            dataPinProxy.unbind();
            lengthProxy.unbind();

            subBladeTypeProxy.unbind();
            subBladeLengthProxy.unbind();
            subBladeSegmentsProxy.unbind();

            star1Proxy.ledProxy.unbind();
            star1Proxy.powerPinProxy.unbind();
            star1Proxy.resistanceProxy.unbind();
            star2Proxy.ledProxy.unbind();
            star2Proxy.powerPinProxy.unbind();
            star2Proxy.resistanceProxy.unbind();
            star3Proxy.ledProxy.unbind();
            star3Proxy.powerPinProxy.unbind();
            star3Proxy.resistanceProxy.unbind();
            star4Proxy.ledProxy.unbind();
            star4Proxy.powerPinProxy.unbind();
            star4Proxy.resistanceProxy.unbind();
            return;
        }

        bladeSelectionProxy.bind(array(arraySelection).bladeSelection);
    });
}

void Config::BladeArrays::addArray(string&& name, uint32 id) {
    auto& array{mBladeArrays.emplace_back()};
    array.name = std::move(name);
    array.id = id;
    auto choices{arraySelection.choices()};
    choices.push_back(array.name);
    arraySelection.setChoices(std::move(choices));
}

void Config::BladeArrays::removeArray(uint32 idx) {

}

void Config::BladeArrays::addPowerPinFromEntry() {
    
}

