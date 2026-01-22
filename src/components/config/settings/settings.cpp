#include "settings.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/settings/settings.cpp
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

#include "config/private/io.h"
#include "utils/string.h"
#include "versions/versions.h"

#include "../config.h"

Config::Settings::Settings(Config& parent) : mParent{parent} {
    // Asign update handlers
    osVersion.setPersistence(PCUI::ChoiceData::PERSISTENCE_STRING);
    osVersion.setUpdateHandler([this](uint32 id) {
        if (id == PCUI::ChoiceData::ID_CHOICES) {
            if (not osVersion.choices().empty() and osVersion == -1) osVersion = 0;
        } else if (id != PCUI::ChoiceData::ID_SELECTION) return;

        if (osVersion.choices().size() > 1 and osVersion == 0) {
            // Will trigger another update.
            osVersion = 1;
            return;
        }

        // Show/hide settings
        mParent.refreshPropVersions();
    });
    bladeDetect.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;

        bladeDetectPin.enable(bladeDetect);
    });

    bladeID.enable.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;

        bladeID.pin.enable(bladeID.enable);
        bladeID.mode.enable(bladeID.enable);
        bladeID.powerForID.enable(bladeID.enable);
        bladeID.bridgePin.enable(bladeID.enable);
        bladeID.pullup.enable(bladeID.enable);
        if (not bladeID.enable) bladeID.continuousScanning = false;
        bladeID.continuousScanning.enable(bladeID.enable);
    });

    bladeID.mode.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ChoiceData::ID_SELECTION) return;

        switch (static_cast<BladeIDMode>(static_cast<uint32>(bladeID.mode))) {
            case SNAPSHOT:
                bladeID.bridgePin.show(false, true);
                bladeID.pullup.show(false, true);
                break;
            case EXTERNAL:
                bladeID.bridgePin.show(false, true);
                bladeID.pullup.show(true, true);
                break;
            case BRIDGED:
                bladeID.bridgePin.show(true, true);
                bladeID.pullup.show(false, true);
                break;
            case BLADEID_MODE_MAX:
                assert(0);
        }
    });

    bladeID.bridgePin.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ComboBoxData::ID_VALUE) return;

        auto rawValue{static_cast<string>(bladeID.bridgePin)};
        uint32 numTrimmed{};
        auto insertionPoint{bladeID.bridgePin.getInsertionPoint()};
        Utils::trimCppName(
            rawValue,
            true,
            &numTrimmed,
            insertionPoint
        );

        if (static_cast<string>(bladeID.bridgePin) == rawValue) {
            return;
        }
        
        bladeID.bridgePin = std::move(rawValue);
        bladeID.bridgePin.setInsertionPoint(insertionPoint - numTrimmed);
    });

    bladeID.continuousScanning.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;

        bladeID.continuousInterval.enable(bladeID.continuousScanning);
        bladeID.continuousTimes.enable(bladeID.continuousScanning);
    });

    bladeID.powerForID.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;

        bladeID.powerPins.enable(bladeID.powerForID);
        bladeID.powerPinEntry.enable(bladeID.powerForID);
    });

    bladeID.powerPins.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::CheckListData::ID_SELECTION) return;

        auto selected{static_cast<set<uint32>>(bladeID.powerPins)};
        auto items{bladeID.powerPins.items()};
        for (auto idx{6}; idx < items.size(); ++idx) {
            if (not selected.contains(idx)) {
                items.erase(std::next(items.begin(), idx));
                --idx;
            }
        }
        bladeID.powerPins.setItems(std::move(items));
    });

    bladeID.powerPinEntry.setUpdateHandler([this](uint32 id) {
        if (id == PCUI::TextData::ID_ENTER) {
            bladeID.addPowerPinFromEntry();
        }
        if (id != PCUI::TextData::ID_VALUE) return;

        auto rawValue{static_cast<string>(bladeID.powerPinEntry)};
        uint32 numTrimmed{};
        auto insertionPoint{bladeID.powerPinEntry.getInsertionPoint()};
        Utils::trimCppName(
            rawValue,
            true,
            &numTrimmed,
            insertionPoint
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
        if (id != PCUI::NumericData::ID_VALUE) return;
        bootVolume.setRange(0, volume);
    });
    enableBootVolume.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;
        bootVolume.enable(enableBootVolume);
    });

    enableFiltering.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;
        filterOrder.enable(enableFiltering);
        filterCutoff.enable(enableFiltering);
    });

    disableTalkie.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ToggleData::ID_VALUE) return;
        femaleTalkie.enable(not disableTalkie);
    });

    // Set defaults
    board.setChoices({
        "Proffieboard V3",
        "Proffieboard V2",
        "Proffieboard V1",
    });
    board.setValue(PROFFIEBOARDV3);

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
    bladeID.mode.setValue(SNAPSHOT);
    bladeID.pin.setDefaults([&]() {
        auto v{pinDefaults};
        v.insert(v.begin(), "bladeIdentifyPin");
        return v;
    }());
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
    if (osVersion < 1) return Utils::Version::invalidObject();
    const auto& osVersions{Versions::getOSVersions()};
    if (osVersion - 1 >= osVersions.size()) return Utils::Version::invalidObject();

    return osVersions[osVersion - 1].verNum;
}

Config::Settings::ButtonData::ButtonData() {
    type.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ChoiceData::ID_SELECTION) return;

        touch.show(type == TOUCH_BUTTON);
    });
    pin.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ComboBoxData::ID_VALUE) return;

        auto rawValue{static_cast<string>(pin)};
        uint32 numTrimmed{};
        auto insertionPoint{pin.getInsertionPoint()};
        Utils::trimCppName(
            rawValue,
            true,
            &numTrimmed,
            insertionPoint
        );

        if (rawValue == static_cast<string>(pin)) return;

        pin = std::move(rawValue);
        pin.setInsertionPoint(insertionPoint - numTrimmed);
    });
    name.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::TextData::ID_VALUE) return;

        auto rawValue{static_cast<string>(name)};
        uint32 numTrimmed{};
        auto insertionPoint{name.getInsertionPoint()};
        Utils::trim(
            rawValue,
            {.allowAlpha=true, .allowNum=true},
            &numTrimmed,
            insertionPoint
        );

        if (rawValue == static_cast<string>(name)) return;

        name = std::move(rawValue);
        name.setInsertionPoint(insertionPoint - numTrimmed);
    });

    type.setChoices(Utils::createEntries({
        _("Momentary (Pullup)"),
        _("Momentary (Pulldown)"),
        _("Latching (Pullup)"),
        _("Latching (Pulldown)"),
        _("Touch"),
    }));

    event.setChoices(Utils::createEntries({
        _("Power"),
        _("Aux"),
        _("Aux 2"),
        _("Up/Fire"),
        _("Down/Mode Select"),
        _("Left/Magazine Detect"),
        _("Right/Reload"),
        _("Select/Range"),
    }));

    pin.setDefaults(Utils::createEntries({
        _("powerButtonPin"),
        _("auxPin"),
        _("aux2Pin"),
    }));

    touch.setRange(0, 50000, false);
    touch.setIncrement(10, false);
    touch.setValue(1700);
}

void Config::Settings::addButton(std::unique_ptr<ButtonData>&& ptr) {
    if (ptr) {
        mButtons.push_back(std::move(ptr));
        buttonNotifier.notify();
        return;
    }

    auto& button{*mButtons.emplace_back(std::make_unique<ButtonData>())};
    if (mButtons.size() == 1) {
        button.type = BUTTON;
        button.event = POWER;

        button.pin = string{button.pin.defaults()[0]};
        button.name = "pow";
    } else if (mButtons.size() == 2) {
        button.type = BUTTON;
        button.event = AUX;

        button.pin = string{button.pin.defaults()[1]};
        button.name = "aux";
    } else if (mButtons.size() == 3) {
        button.type = BUTTON;
        button.event = AUX2;

        button.pin = string{button.pin.defaults()[2]};
        button.name = "aux2";
    } else {
        button.type = BUTTON;
        button.event = POWER;
    }

    buttonNotifier.notify();
}

void Config::Settings::removeButton(size idx) {
    if (idx >= mButtons.size()) return;

    mButtons.erase(std::next(mButtons.begin(), static_cast<ssize>(idx)));
    buttonNotifier.notify();
}

void Config::Settings::removeButton(const ButtonData& button) {
    auto iter{mButtons.begin()};
    for (; iter != mButtons.end(); ++iter) {
        if (&**iter == &button) break;
    }
    if (iter == mButtons.end()) return;

    mButtons.erase(iter);
    buttonNotifier.notify();
}

/*
 * TODO: Right now all the config editing happens in the main thread, either
 *       programmatically or from UI updates. Where the config is accessed
 *       concurrently from another thread, it's behind a modal dialog.
 *
 *       Perhaps sometime in the future this may want to change, in that case
 *       manipulating these vectors (I'm sure there's other cases too) will
 *       need to be safeguarded.
 */

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

void Config::Settings::processCustomDefines(Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Config::Settings::processCustomDefines()", lBranch)};
    for (auto idx{0}; idx < mCustomOptions.size(); ++idx) {
        auto& opt{customOption(idx)};

        bool processed{true};

        if (
                opt.define == NUM_BLADES_STR or
                opt.define == ENABLE_AUDIO_STR or 
                opt.define == ENABLE_MOTION_STR or
                opt.define == ENABLE_WS2811_STR or
                opt.define == ENABLE_SD_STR or
                opt.define == SHARED_POWER_PINS_STR or
                opt.define == KEEP_SAVEFILES_STR or
                opt.define == NUM_BUTTONS_STR
           ) {
            // Do nothing
        // } else if (opt.define == RFID_SERIAL_STR) {
        // TODO: Not Yet Implemented
        } else if (opt.define == BLADE_DETECT_PIN_STR) {
            bladeDetect = true;
            bladeDetectPin = static_cast<string>(opt.value);
        } else if (opt.define == BLADE_ID_CLASS_STR) {
            bladeID.enable = true;
            auto idx{0};
            for (; idx < BLADEID_MODE_MAX; ++idx) {
                if (opt.value.startsWith(BLADEID_MODE_STRS[idx])) break;
            }

            if (idx == BLADEID_MODE_MAX) {
                logger.warn("Cannot parse invalid/unrecognized BladeID class");
            } else {
                bladeID.mode = idx;

                string str{opt.value};
                str.erase(0, BLADEID_MODE_STRS[idx].length());

                const auto idPinEnd{str.find(',')};
                bladeID.pin = str.substr(0, idPinEnd);

                if (idx == EXTERNAL) {
                    if (idPinEnd == string::npos) {
                        logger.warn("Missing pullup value for external blade id");
                    } else {
                        str.erase(0, idPinEnd + 1);

                        auto val{Utils::doStringMath(str)};
                        if (val) bladeID.pullup = static_cast<int32>(*val);
                        else logger.warn("Failed to parse pullup value for ext blade id");
                    }
                } else if (idx == BRIDGED) {
                    if (idPinEnd == string::npos) {
                        logger.warn("Missing bridge pin for blade id");
                    } else {
                        str.erase(0, idPinEnd + 1);
                        bladeID.bridgePin = static_cast<string>(str);
                    }
                }
            }
        } else if (opt.define == ENABLE_POWER_FOR_ID_STR) {
            bladeID.powerForID = true;

            if (not opt.value.startsWith(POWER_PINS_STR)) {
                logger.warn("Failed to parse BladeID PowerPINS");
            } else {
                string str{opt.value};
                str.erase(0, POWER_PINS_STR.length());

                while (not false) {
                    const auto endPos{str.find(',')};

                    // Use the entry for processing
                    bladeID.powerPinEntry = str.substr(0, endPos);
                    bladeID.powerPins.select(bladeID.powerPinEntry);

                    if (endPos == string::npos) break;

                    str.erase(0, endPos + 1);
                }
            }
        } else if (opt.define == BLADE_ID_SCAN_MILLIS_STR) {
            bladeID.continuousScanning = true;            

            auto val{Utils::doStringMath(opt.value)};
            if (val) bladeID.continuousInterval = static_cast<int32>(*val);
            else logger.warn("Failed to parse blade id scan interval");
        } else if (opt.define == BLADE_ID_TIMES_STR) {
            bladeID.continuousScanning = true;            

            auto val{Utils::doStringMath(opt.value)};
            if (val) bladeID.continuousTimes = static_cast<int32>(*val);
            else logger.warn("Failed to parse blade id scan times");
        } else if (opt.define == VOLUME_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) volume = static_cast<int32>(*val);
            else logger.warn("Failed to parse volume");
        } else if (opt.define == BOOT_VOLUME_STR) {
            enableBootVolume = true;

            auto val{Utils::doStringMath(opt.value)};
            if (val) bootVolume = static_cast<int32>(*val);
            else logger.warn("Failed to parse boot volume");
        } else if (opt.define == CLASH_THRESHOLD_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) clashThreshold = *val;
            else logger.warn("Failed to parse clash threshold");
        } else if (opt.define == PLI_OFF_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) pliOffTime = *val / 1000;
            else logger.warn("Failed to parse PLI off time");
        } else if (opt.define == IDLE_OFF_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) idleOffTime = *val / (60 * 1000);
            else logger.warn("Failed to parse idle off time");
        } else if (opt.define == MOTION_TIMEOUT_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) motionTimeout = *val / (60 * 1000);
            else logger.warn("Failed to parse motion timeout");
        } else if (opt.define == DISABLE_COLOR_CHANGE_STR) {
            disableColorChange = true;
        } else if (opt.define == DISABLE_BASIC_PARSERS_STR) {
            disableBasicParserStyles = true;
        } else if (opt.define == DISABLE_DIAG_COMMANDS_STR) {
            disableDiagnosticCommands = true;
        // } else if (opt.define == ENABLE_DEV_COMMANDS_STR) {
        //     enableDeveloperCommands = true;
        } else if (opt.define == SAVE_STATE_STR) {
            saveState = true;
        } else if (opt.define == ENABLE_ALL_EDIT_OPTIONS_STR) {
            enableAllEditOptions = true;
        } else if (opt.define == SAVE_COLOR_STR) {
            saveColorChange = true;
        } else if (opt.define == SAVE_VOLUME_STR) {
            saveVolume = true;
        } else if (opt.define == SAVE_PRESET_STR) {
            savePreset = true;
        } else if (opt.define == ENABLE_OLED_STR) {
            enableOLED = true;
        } else if (opt.define == ORIENTATION_STR) {
            auto idx{0};
            for (; idx < ORIENTATION_MAX; ++idx) {
                if (opt.value == ORIENTATION_STRS[idx]) break;
            }

            if (idx == ORIENTATION_MAX) {
                logger.warn("Unknown/invalid orientation");
            } else {
                orientation = idx;
            }
        } else if (opt.define == ORIENTATION_ROTATION_STR) {
            const auto firstComma{opt.value.find(',')};
            const auto secondComma{opt.value.find(',', firstComma + 1)};

            if (firstComma == string::npos or secondComma == string::npos) {
                logger.warn("Invalid formatting for orientation rotation");
            } else {
                string str{opt.value};
                auto xStr{str.substr(0, firstComma)};
                auto yStr{str.substr(firstComma + 1, secondComma - firstComma - 1)};
                auto zStr{str.substr(secondComma + 1, str.length() - secondComma - 1)};

                auto xVal{Utils::doStringMath(xStr)};
                auto yVal{Utils::doStringMath(yStr)};
                auto zVal{Utils::doStringMath(zStr)};

                if (xVal) orientationRotation.x = static_cast<int32>(*xVal);
                else logger.warn("Failed to parse orientation rotation X");

                if (yVal) orientationRotation.y = static_cast<int32>(*yVal);
                else logger.warn("Failed to parse orientation rotation Y");

                if (zVal) orientationRotation.z = static_cast<int32>(*zVal);
                else logger.warn("Failed to parse orientation rotation Z");
            }
        // } else if (opt.define == SPEAK_TOUCH_VALUES_STR) {
        //     speakTouchValues = true;
        } else if (opt.define == DYNAMIC_BLADE_DIMMING_STR) {
            dynamicBladeDimming = true;
        } else if (opt.define == DYNAMIC_BLADE_LENGTH_STR) {
            dynamicBladeLength = true;
        } else if (opt.define == DYNAMIC_CLASH_THRESHOLD_STR) {
            dynamicClashThreshold = true;
        } else if (opt.define == SAVE_BLADE_DIM_STR) {
            saveBladeDimming = true;
        } else if (opt.define == SAVE_CLASH_THRESHOLD_STR) {
            saveClashThreshold = true;
        } else if (opt.define == FILTER_CUTOFF_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) filterCutoff = static_cast<int32>(*val);
            else logger.warn("Failed to parse filter cutoff");
        } else if (opt.define == FILTER_ORDER_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) filterOrder = static_cast<int32>(*val);
            else logger.warn("Failed to parse filter order");
        } else if (opt.define == AUDIO_CLASH_SUPPRESSION_STR) {
            auto val{Utils::doStringMath(opt.value)};
            if (val) audioClashSuppressionLevel = static_cast<int32>(*val);
            else logger.warn("Failed to parse audio clash suppression");
        } else if (opt.define == DONT_USE_GYRO_FOR_CLASH_STR) {
            dontUseGyroForClash = true;
        } else if (opt.define == NO_REPEAT_RANDOM_STR) {
            noRepeatRandom = true;
        } else if (opt.define == FEMALE_TALKIE_STR) {
            femaleTalkie = true;
        } else if (opt.define == DISABLE_TALKIE_STR) {
            disableTalkie = true;
        } else if (opt.define == KILL_OLD_PLAYERS_STR) {
            killOldPlayers = true;
        } else {
            processed = false;
        }

        if (processed) {
            removeCustomOption(idx);
            --idx;
        }
    }
}

