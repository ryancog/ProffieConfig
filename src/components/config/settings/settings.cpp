#include "settings.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/settings/settings.h
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
#include "versions/versions.h"

#include "../config.h"

Config::Settings::Settings(Config& parent) : mParent{parent} {
    // Asign update handlers
    osVersion.setUpdateHandler([this](uint32 id) {
        if (id != osVersion.ID_SELECTION) return;

        notifyData.notify(ID_OS_VERSION);
        mParent.refreshPropVersions();
    });
    bladeDetect.setUpdateHandler([this](uint32 id) {
        if (id != bladeDetect.ID_VALUE) return;

        bladeDetectPin.enable(bladeDetect);
    });

    bladeID.enable.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.enable.ID_VALUE) return;

        bladeID.pin.enable(bladeID.enable);
        bladeID.mode.enable(bladeID.enable);
        bladeID.powerForID.enable(bladeID.enable);
        bladeID.bridgePin.enable(bladeID.enable);
        bladeID.pullup.enable(bladeID.enable);
        if (not bladeID.enable) bladeID.continuousScanning = false;
        bladeID.continuousScanning.enable(bladeID.enable);
    });

    bladeID.mode.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.mode.ID_SELECTION) return;

        switch (static_cast<BladeID::Mode>(static_cast<uint32>(bladeID.mode))) {
            case BladeID::SNAPSHOT:
                bladeID.bridgePin.show(false, true);
                bladeID.pullup.show(false, true);
                break;
            case BladeID::EXTERNAL:
                bladeID.bridgePin.show(false, true);
                bladeID.pullup.show(true, true);
                break;
            case BladeID::BRIDGED:
                bladeID.bridgePin.show(true, true);
                bladeID.pullup.show(false, true);
                break;
            case BladeID::MODE_MAX:
                assert(0);
        }
    });

    bladeID.bridgePin.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.bridgePin.ID_VALUE) return;

        auto pinValue{static_cast<string>(bladeID.bridgePin)};
        Utils::trimUnsafe(pinValue);
        if (static_cast<string>(bladeID.bridgePin) == pinValue) return;
        bladeID.bridgePin = std::move(pinValue);
    });

    bladeID.continuousScanning.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.enable.ID_VALUE) return;

        bladeID.continuousInterval.enable(bladeID.continuousScanning);
        bladeID.continuousTimes.enable(bladeID.continuousScanning);
    });

    bladeID.powerForID.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.powerForID.ID_VALUE) return;

        bladeID.powerPins.enable(bladeID.powerForID);
        bladeID.powerPinEntry.enable(bladeID.powerForID);
    });

    bladeID.powerPins.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.powerPins.ID_SELECTION) return;

        auto selected{static_cast<set<uint32>>(bladeID.powerPins)};
        auto items{bladeID.powerPins.items()};
        for (auto idx{6}; idx < items.size(); ++idx) {
            if (selected.find(idx) == selected.end()) {
                items.erase(std::next(items.begin(), idx));
                --idx;
            }
        }
        bladeID.powerPins.setItems(std::move(items));
    });

    bladeID.powerPinEntry.setUpdateHandler([this](uint32 id) {
        if (id == bladeID.powerPinEntry.ID_ENTER) {
            bladeID.addPowerPinFromEntry();
        }
        if (id != bladeID.powerPinEntry.ID_VALUE) return;

        auto rawValue{static_cast<string>(bladeID.powerPinEntry)};
        uint32 numTrimmed{};
        auto insertionPoint{bladeID.powerPinEntry.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            {},
            true
        );

        if (rawValue != static_cast<string>(bladeID.powerPinEntry)) {
            bladeID.powerPinEntry = std::move(rawValue);
            bladeID.powerPinEntry.setInsertionPoint(insertionPoint - numTrimmed);
            return;
        }
    });

    const auto updateSaveOptions{[this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;

        bool stateOrAll{saveState or enableAllEditOptions};
        saveVolume |= stateOrAll;
        saveVolume.enable(not stateOrAll);
        savePreset |= saveState;
        savePreset.enable(not saveState);
        saveColorChange |= stateOrAll;
        saveColorChange.enable(not stateOrAll);

        saveBladeDimming |= stateOrAll and dynamicBladeDimming;
        saveBladeDimming.enable(dynamicBladeDimming and not stateOrAll);
        saveClashThreshold |= enableAllEditOptions and dynamicClashThreshold;
        saveClashThreshold.enable(dynamicClashThreshold and not enableAllEditOptions);

        dynamicBladeLength |= enableAllEditOptions;
        dynamicBladeLength.enable(not enableAllEditOptions);
        dynamicBladeDimming |= enableAllEditOptions;
        dynamicBladeDimming.enable(not enableAllEditOptions);
        dynamicClashThreshold |= enableAllEditOptions;
        dynamicClashThreshold.enable(not enableAllEditOptions);
    }};

    saveState.setUpdateHandler(updateSaveOptions);
    enableAllEditOptions.setUpdateHandler(updateSaveOptions);
    dynamicBladeDimming.setUpdateHandler(updateSaveOptions);
    dynamicClashThreshold.setUpdateHandler(updateSaveOptions);

    volume.setUpdateHandler([this](uint32 id) {
        if (id != volume.ID_VALUE) return;
        bootVolume.setRange(0, volume);
    });
    enableBootVolume.setUpdateHandler([this](uint32 id) {
        if (id != enableBootVolume.ID_VALUE) return;
        bootVolume.enable(enableBootVolume);
    });

    enableFiltering.setUpdateHandler([this](uint32 id) {
        if (id != enableFiltering.ID_VALUE) return;
        filterOrder.enable(enableFiltering);
        filterCutoff.enable(enableFiltering);
    });

    disableTalkie.setUpdateHandler([this](uint32 id) {
        if (id != disableTalkie.ID_VALUE) return;
        femaleTalkie.enable(not disableTalkie);
    });

    // Set defaults
    board.setChoices({
        "Proffieboard V3",
        "Proffieboard V2",
        "Proffieboard V1",
    });
    board.setValue(PROFFIEBOARDV3);

    numButtons.setRange(0, 3);
    numButtons.setValue(2);

    vector<string> pinDefaults{
        "bladePin",
        "blade2Pin",
        "blade3Pin",
        "blade4Pin",
    };
    bladeDetect.setValue(false);
    bladeDetectPin.setDefaults(vector{pinDefaults});
    bladeID.enable.setValue(false);
    bladeID.mode.setChoices(Utils::createEntries({
        _("Snapshot"),
        _("External Pullup"),
        _("Bridged Pullup")
    }));
    bladeID.mode.setValue(BladeID::SNAPSHOT);
    bladeID.pin.setDefaults(vector{pinDefaults});
    bladeID.bridgePin.setDefaults(vector{pinDefaults});
    bladeID.continuousScanning.setValue(false);
    bladeID.continuousTimes.setRange(1, 100);
    bladeID.continuousTimes.setValue(10);
    bladeID.continuousInterval.setRange(10, 120000);
    bladeID.continuousInterval.setValue(1000);
    bladeID.powerForID.setValue(false);
    bladeID.powerPins.setItems({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    });

    volume.setRange(0, 4000, false);
    volume.setIncrement(50, false);
    volume.setValue(1000);
    bootVolume.setRange(0, 4000, false);
    bootVolume.setIncrement(50, false);
    enableBootVolume.setValue(false);

    clashThreshold.setRange(0.1, 5);
    clashThreshold.setIncrement(0.1);
    clashThreshold.setValue(3.0);

    pliOffTime.setRange(1, 3600);
    pliOffTime.setValue(10);
    idleOffTime.setRange(1, 30000);
    idleOffTime.setValue(10);
    motionTimeout.setRange(1, 30000);
    motionTimeout.setValue(15);

    disableColorChange.setValue(false);
    disableBasicParserStyles.setValue(false);
    disableDiagnosticCommands.setValue(false);
    // enableDeveloperCommands.setValue(false);

    saveState.setValue(false);
    enableAllEditOptions.setValue(false);

    saveColorChange.setValue(false);
    saveVolume.setValue(false);
    savePreset.setValue(false);

    orientation.setChoices(Utils::createEntries({
        _("FETs Towards Blade"),
        _("USB Towards Blade"),
        _("USB CCW From Blade"),
        _("USB CW From Blade"),
        _("Top Towards Blade"),
        _("Bottom Towards Blade")
    }));
    orientation.setValue(FETS_TOWARDS_BLADE);

    orientationRotation.x.setRange(-90, 90);
    orientationRotation.x.setValue(0);
    orientationRotation.y.setRange(-90, 90);
    orientationRotation.y.setValue(0);
    orientationRotation.z.setRange(-90, 90);
    orientationRotation.z.setValue(0);

    // speakTouchValues.setValue(false);

    dynamicBladeDimming.setValue(false);
    dynamicBladeLength.setValue(false);
    dynamicClashThreshold.setValue(false);

    saveBladeDimming.setValue(false);
    saveClashThreshold.setValue(false);

    filterCutoff.setRange(1, 10000, false);
    filterCutoff.setIncrement(10, false);
    filterCutoff.setValue(100);
    filterOrder.setRange(1, 256, false);
    filterOrder.setValue(8);
    enableFiltering.setValue(false);

    audioClashSuppressionLevel.setRange(1, 50, false);
    audioClashSuppressionLevel.setValue(10);
    dontUseGyroForClash.setValue(false);

    noRepeatRandom.setValue(false);
    femaleTalkie.setValue(false);
    disableTalkie.setValue(false);
    killOldPlayers.setValue(false);
}

Utils::Version Config::Settings::getOSVersion() const {
    if (osVersion == -1) return Utils::Version::invalidObject();
    const auto& osVersions{Versions::getOSVersions()};
    if (osVersion >= osVersions.size()) return Utils::Version::invalidObject();

    return osVersions[osVersion].verNum;
}

bool Config::Settings::addCustomOption(string&& key, string&& value) {
    if (key.empty()) {
        for (auto& opt : mCustomOptions) {
            if (static_cast<string>(opt->define).empty()) return false;
        }
    }

    auto& customOpt{*mCustomOptions.emplace_back(std::make_unique<CustomOption>())};
    customOpt.define = std::move(key);
    customOpt.value = std::move(value);
    customOptsNotifyData.notify();
    return true;
}

bool Config::Settings::removeCustomOption(CustomOption& opt) {
    auto iter{mCustomOptions.begin()};
    for (; iter != mCustomOptions.end(); ++iter) {
        if (&**iter == &opt) break;
    }
    if (iter == mCustomOptions.end()) return false;

    mCustomOptions.erase(iter);
    customOptsNotifyData.notify();
    return true;
}

bool Config::Settings::removeCustomOption(uint32 idx) {
    if (idx >= mCustomOptions.size()) return false;

    mCustomOptions.erase(std::next(mCustomOptions.begin(), idx));
    customOptsNotifyData.notify();
    return true;
}

void Config::Settings::BladeID::addPowerPinFromEntry() {
    if (static_cast<string>(powerPinEntry).empty()) return;

    auto powerPinItems{powerPins.items()};
    uint32 idx{0};
    for (; idx < powerPinItems.size(); ++idx) {
        if (powerPinItems[idx] == static_cast<string>(powerPinEntry)) break;
    }
    if (idx != powerPinItems.size()) {
        powerPins.select(idx);
    } else {
        powerPinItems.emplace_back(static_cast<string>(powerPinEntry));
        powerPins.setItems(std::move(powerPinItems));
        powerPins.select(powerPins.items().size() - 1);
    }
    powerPinEntry = "";
}

