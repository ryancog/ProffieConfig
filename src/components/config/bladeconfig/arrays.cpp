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
#include "utils/string.h"
#include "ws281x.h"


Config::BladeArrays::BladeArrays(Config& parent) :
    mParent{parent},
    splitTypeProxy{Split::TYPE_MAX} {

    bladeTypeProxy.showWhenUnbound(false);

    splitStartProxy.showWhenUnbound(false);
    splitEndProxy.showWhenUnbound(false);
    splitLengthProxy.showWhenUnbound(false);
    splitSegmentsProxy.showWhenUnbound(false);
    splitListProxy.showWhenUnbound(false);

    arraySelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

    arraySelection.setUpdateHandler([this](uint32 id) {
        if (id != arraySelection.ID_SELECTION) return;
        Defer defer{[this]() { notifyData.notify(ID_ARRAY_SELECTION); }};

        arrayIssues = BladeConfig::ISSUE_NONE;
        bladeSelectionProxy.unbind();
        unbindBlade();

        if (arraySelection == -1) return;

        auto& selectedArray{array(arraySelection)};
        bladeSelectionProxy.bind(selectedArray.bladeSelection);
        arrayIssues = selectedArray.computeIssues();
    });
    powerPinNameEntry.setUpdateHandler([this](uint32 id) {
        if (id == powerPinNameEntry.ID_ENTER) {
            addPowerPinFromEntry();
        }
        if (id != powerPinNameEntry.ID_VALUE) return;

        auto rawValue{static_cast<string>(powerPinNameEntry)};
        uint32 numTrimmed{};
        auto insertionPoint{powerPinNameEntry.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            {},
            true
        );

        if (rawValue == static_cast<string>(powerPinNameEntry)) {
            return;
        }
        
        powerPinNameEntry = std::move(rawValue);
        powerPinNameEntry.setInsertionPoint(insertionPoint - numTrimmed);
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
    mParent.presetArrays.syncStyleDisplay();
    if (mParent.presetArrays.styleDisplay == -1) mParent.presetArrays.styleDisplay = 0;
    return *array;
}

void Config::BladeArrays::removeArray(uint32 idx) {
    assert(idx < mBladeArrays.size());

    mBladeArrays.erase(std::next(mBladeArrays.begin(), idx));
    auto choices{arraySelection.choices()};
    choices.erase(std::next(choices.begin(), idx));
    arraySelection.setChoices(std::move(choices));
    mParent.presetArrays.syncStyleDisplay(idx);
    if (arraySelection == idx) arraySelection = -1;
}

void Config::BladeArrays::addPowerPinFromEntry() {
    if (static_cast<string>(powerPinNameEntry).empty()) return;
    if (arraySelection == -1) return;
    auto& selectedArray{array(arraySelection)};

    if (selectedArray.bladeSelection == -1) return;
    auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};

    if (selectedBlade.type != Blade::WS281X) return;
    auto& ws281x{selectedBlade.ws281x()};

    auto powerPinItems{ws281x.powerPins.items()};
    powerPinItems.emplace_back(static_cast<string>(powerPinNameEntry));
    ws281x.powerPins.setItems(std::move(powerPinItems));
    ws281x.powerPins.select(ws281x.powerPins.items().size() - 1);
    powerPinNameEntry = "";
}

uint32 Config::BladeArrays::numBlades() {
    uint32 ret{0};
    for (const auto& array : mBladeArrays) {
        uint32 count{0};
        for (const auto& blade : array->blades()) {
            if (blade->type == Blade::SIMPLE) ++count;
            if (blade->type == Blade::WS281X) {
                auto& ws281x{blade->ws281x()};
                if (ws281x.splits().empty()) ++count;
                else {
                    for (const auto& split : ws281x.splits()){
                        switch (static_cast<Split::Type>(static_cast<uint32>(split->type))) {
                            case Split::STANDARD:
                            case Split::REVERSE:
                                ++count;
                                break;
                            case Split::STRIDE:
                            case Split::ZIG_ZAG:
                                count += split->segments;
                                break;
                            case Split::TYPE_MAX:
                            default:
                                assert(0);
                        }
                    }
                }
            }
        }
        if (ret < count) ret = count;
    }

    return ret;
}

void Config::BladeArrays::unbindBlade() {
    bladeTypeProxy.unbind();

    lengthProxy.unbind();
    dataPinProxy.unbind();

    colorOrder3Proxy.unbind();
    colorOrder4Proxy.unbind();
    hasWhiteProxy.unbind();
    useRGBWithWhiteProxy.unbind();
    pixelBrightnessProxy.unbind();

    powerPinProxy.unbind();

    splitSelectionProxy.unbind();
    splitTypeProxy.unbind();
    splitLengthProxy.unbind();
    splitSegmentsProxy.unbind();
    splitBrightnessProxy.unbind();

    star1Proxy.ledProxy.unbind();
    star1Proxy.resistanceProxy.unbind();
    star1Proxy.powerPinProxy.unbind();

    star2Proxy.ledProxy.unbind();
    star2Proxy.resistanceProxy.unbind();
    star2Proxy.powerPinProxy.unbind();

    star3Proxy.ledProxy.unbind();
    star3Proxy.resistanceProxy.unbind();
    star3Proxy.powerPinProxy.unbind();

    star4Proxy.ledProxy.unbind();
    star4Proxy.resistanceProxy.unbind();
    star4Proxy.powerPinProxy.unbind();

    simpleBrightnessProxy.unbind();
}

