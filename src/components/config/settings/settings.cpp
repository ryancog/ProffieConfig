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

#include "../config.h"

Config::Settings::Settings(Config& parent) : mParent{parent} {
    // Asign update handlers
    osVersion.setUpdateHandler([this](uint32 id) {
        if (id != osVersion.ID_SELECTION) return;

        mParent.refreshPropVersions();
    });
    bladeDetect.setUpdateHandler([this](uint32 id) {
        if (id != bladeDetect.ID_VALUE) return;

        if (bladeDetect) {
            bladeDetectPin.enable();
        } else {
            bladeDetectPin.disable();
        }
    });

    bladeID.enable.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.enable.ID_VALUE) return;

        if (bladeID.enable) {
            bladeID.pin.enable();
            bladeID.mode.enable();
            bladeID.powerPins.enable();
            bladeID.bridgePin.enable();
            bladeID.pullup.enable();
        } else {
            bladeID.pin.disable();
            bladeID.mode.disable();
            bladeID.powerPins.disable();
            bladeID.bridgePin.disable();
            bladeID.pullup.disable();
        }
    });

    bladeID.mode.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.mode.ID_SELECTION) return;

        switch (static_cast<BladeID::Mode>(static_cast<uint32>(bladeID.mode))) {
            case BladeID::SNAPSHOT:
                bladeID.bridgePin.hide();
                bladeID.pullup.hide();
                break;
            case BladeID::EXTERNAL:
                bladeID.bridgePin.hide();
                bladeID.pullup.show();
                break;
            case BladeID::BRIDGED:
                bladeID.bridgePin.show();
                bladeID.pullup.hide();
                break;
        }
    });

    bladeID.bridgePin.setUpdateHandler([this](uint32 id) {
        if (id != bladeID.bridgePin.ID_VALUE) return;

        auto pinValue{static_cast<string>(bladeID.bridgePin)};
        Utils::trimUnsafe(pinValue);
        if (static_cast<string>(bladeID.bridgePin) == pinValue) return;
        bladeID.bridgePin = std::move(pinValue);
    });

    // Set defaults
    board.setChoices({
        "Proffieboard V3",
        "Proffieboard V2",
        "Proffieboard V1",
    });
    board = PROFFIEBOARDV3;

    numButtons.setRange(0, 3);
    numButtons = 2;

    bladeDetect = false;
    bladeID.enable = false;
    bladeID.mode.setChoices(Utils::createEntries({
        _("Snapshot"),
        _("External Pullup"),
        _("Bridged Pullup")
    }));
    bladeID.mode = BladeID::SNAPSHOT;

    volume.setRange(0, 4000);
    volume.setIncrement(50);
    volume = 1000;

    enableBootVolume = false;

    clashThreshold.setRange(0.1, 5);
    clashThreshold.setIncrement(0.1);
    clashThreshold = 3.0;

    pliOffTime.setRange(1, 30000);
    pliOffTime = 2;
    idleOffTime.setRange(1, 30000);
    idleOffTime = 10;
    motionOffTime.setRange(1, 30000);
    motionOffTime = 15;

    disableColorChange = false;
    disableBasicParserStyles = false;
    disableDiagnosticCommands = false;
    enableDeveloperCommands = false;

    saveState = false;
    enableAllEditOptions = false;

    saveColorChange = false;    
    saveVolume = false;
    savePreset = false;

    orientation.setChoices(Utils::createEntries({
        _("FETs Towards Blade"),
        _("USB Towards Blade"),
        _("USB CCW From Blade"),
        _("USB CW From Blade"),
        _("Top Towards Blade"),
        _("Bottom Towards Blade")
    }));
    orientation = FETS_TOWARDS_BLADE;

    orientationRotation.x = 0;
    orientationRotation.y = 0;
    orientationRotation.z = 0;

    speakTouchValues = false;

    dynamicBladeDimming = false;
    dynamicBladeLength = false;
    dynamicClashThreshold = false;

    saveBladeDimming = false;
    saveClashThreshold = false;

    filterCutoff = 100;
    filterOrder = 8;

    audioClashSuppressionLevel = 10;
    dontUseGyroForClash = false;

    noRepeatRandom = false;
    femaleTalkie = false;
    disableTalkie = false;
    killOldPlayers = false;
}

bool Config::Settings::addCustomOption() {
    for (auto& opt : mCustomOptions) {
        if (static_cast<string>(opt.define).empty()) return false;
    }

    mCustomOptions.emplace_back();
    customOptsNotifier.getLock().lock();
    customOptsNotifier.notify();
    customOptsNotifier.getLock().unlock();
    return true;
}

bool Config::Settings::removeCustomOption(CustomOption& opt) {
    auto iter{mCustomOptions.begin()};
    for (; iter != mCustomOptions.end(); ++iter) {
        if (&*iter == &opt) break;
    }
    if (iter == mCustomOptions.end()) return false;

    mCustomOptions.erase(iter);
    customOptsNotifier.getLock().lock();
    customOptsNotifier.notify();
    customOptsNotifier.getLock().unlock();
    return true;
}

