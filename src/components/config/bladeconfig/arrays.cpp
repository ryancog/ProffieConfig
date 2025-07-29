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

#include "../config.h"
#include "bladeconfig.h"
#include "utils/defer.h"
#include "ws281x.h"


Config::BladeArrays::BladeArrays(Config& parent) :
    mParent{parent},
    subBladeTypeProxy{Split::TYPE_MAX} {

    subBladeSelectionProxy.showWhenUnbound(false);

    bladeTypeProxy.showWhenUnbound(false);
    powerPinProxy.showWhenUnbound(false);
    powerPinNameEntry.hide();

    colorOrder3Proxy.showWhenUnbound(false);
    colorOrder4Proxy.showWhenUnbound(false);
    hasWhiteProxy.showWhenUnbound(false);
    useRGBWithWhiteProxy.showWhenUnbound(false);

    dataPinProxy.showWhenUnbound(false);
    lengthProxy.showWhenUnbound(false);

    subBladeTypeProxy.showWhenUnbound(false);
    subBladeLengthProxy.showWhenUnbound(false);
    subBladeSegmentsProxy.showWhenUnbound(false);

    star1Proxy.ledProxy.showWhenUnbound(false);
    star1Proxy.powerPinProxy.showWhenUnbound(false);
    star1Proxy.resistanceProxy.showWhenUnbound(false);
    star2Proxy.ledProxy.showWhenUnbound(false);
    star2Proxy.powerPinProxy.showWhenUnbound(false);
    star2Proxy.resistanceProxy.showWhenUnbound(false);
    star3Proxy.ledProxy.showWhenUnbound(false);
    star3Proxy.powerPinProxy.showWhenUnbound(false);
    star3Proxy.resistanceProxy.showWhenUnbound(false);
    star4Proxy.ledProxy.showWhenUnbound(false);
    star4Proxy.powerPinProxy.showWhenUnbound(false);
    star4Proxy.resistanceProxy.showWhenUnbound(false);

    arraySelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

    arraySelection.setUpdateHandler([this](uint32 id) {
        if (id != arraySelection.ID_SELECTION) return;
        Defer defer{[this]() { notifyData.notify(ID_ARRAY_SELECTION); }};

        if (arraySelection == -1) {
            arrayIssues = BladeConfig::ISSUE_NONE;
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

        auto& selectedArray{array(arraySelection)};
        bladeSelectionProxy.bind(selectedArray.bladeSelection);
        arrayIssues = selectedArray.computeIssues();
    });
}

void Config::BladeArrays::refreshPresetArrays(int32 clearIdx) {
    const auto& choices{mParent.presetArrays.selection.choices()};
    for (auto& array : mBladeArrays) {
        array->presetArray.setChoices(vector{choices});
        if (array->presetArray == clearIdx) array->presetArray = -1;
    }
}

Config::BladeConfig& Config::BladeArrays::addArray(string&& name, uint32 id, string presetArray) {
    auto& array{mBladeArrays.emplace_back(std::make_unique<BladeConfig>(mParent))};
    array->name = std::move(name);
    array->id = id;

    array->presetArray.setChoices(vector{mParent.presetArrays.selection.choices()});
    array->presetArray = presetArray;

    auto bladeArrayChoices{arraySelection.choices()};
    bladeArrayChoices.push_back(array->name);
    arraySelection.setChoices(std::move(bladeArrayChoices));
    mParent.presetArrays.syncAllDisplays();
    return *array;
}

void Config::BladeArrays::removeArray(uint32 idx) {
    assert(idx < mBladeArrays.size());

    mBladeArrays.erase(std::next(mBladeArrays.begin(), idx));
    auto choices{arraySelection.choices()};
    choices.erase(std::next(choices.begin(), idx));
    arraySelection.setChoices(std::move(choices));
    mParent.presetArrays.syncAllDisplays(idx);
    if (arraySelection == idx) arraySelection = -1;
}

void Config::BladeArrays::addPowerPinFromEntry() {
    
}

uint32 Config::BladeArrays::numBLades() {
    uint32 ret{0};
    for (const auto& array : mBladeArrays) {
        uint32 count{0};
        for (const auto& blade : array->blades()) {
            if (blade->type == Blade::SIMPLE) ++count;
            if (blade->type == Blade::WS281X) {
                auto& ws281x{blade->ws281x()};
                if (ws281x.splits.empty()) ++count;
                else {
                    for (const auto& split : ws281x.splits){
                        switch (static_cast<Split::Type>(static_cast<uint32>(split.type))) {
                            case Split::STANDARD:
                            case Split::REVERSE:
                                ++count;
                                break;
                            case Split::STRIDE:
                            case Split::ZIG_ZAG:
                                count += split.segments;
                                break;
                            case Split::TYPE_MAX:
                            default:
                                assert(0);
                        }
                    }
                }
            }
        }
    }

    return ret;
}

