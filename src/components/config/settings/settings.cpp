#include "settings.hpp"
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

#include "config/config.hpp"
#include "config/priv/io.hpp"
#include "config/strings.hpp"
#include "config/settings/define.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"

using namespace config;

namespace {

bool processDefine(Settings&, settings::Define&, logging::Logger&);
bool processPropDefine(
    std::span<const std::unique_ptr<versions::props::Prop>>, settings::Define&
);

} // namespace

Settings::Settings(Config& parent) :
    Model(parent),
    massStorage_(root()),
    webUsb_(root()),
    bladeAwareness_(*this),
    volume_(root()),
    bootVolume_{.enable_=root(), .value_=root()},
    filter_{.enable_=root(), .cutoff_=root(), .order_=root()},
    clashThreshold_(root()),
    pliOffTime_(root()),
    idleOffTime_(root()),
    motionTimeout_(root()),
    disableColorChange_(root()),
    disableBasicParserStyles_(root()),
    disableTalkie_(root()),
    disableDiagnosticCommands_(root()),
    saveState_(root()),
    enableAllEditOptions_(root()),
    saveVolume_(root()),
    savePreset_(root()),
    saveColorChange_(root()),
    enableOled_(root()),
    orientation_(root()),
    orientationRotation_{.x_=root(), .y_=root(), .z_=root()},
    dynamicBladeDimming_(root()),
    dynamicBladeLength_(root()),
    dynamicClashThreshold_(root()),
    saveBladeDimming_(root()),
    saveClashThreshold_(root()),
    audioClashSuppressionLevel_(root()),
    dontUseGyroForClash_(root()),
    noRepeatRandom_(root()),
    femaleTalkie_(root()),
    killOldPlayers_(root()),
    defines_(root()) {
    CreationScope createScope(this);

    static const auto saveOptTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map(&Settings::onSaveOptSet);
        return table;
    }()};
    amend(saveState_, saveOptTable);
    amend(enableAllEditOptions_, saveOptTable);
    amend(dynamicBladeDimming_, saveOptTable);
    amend(dynamicClashThreshold_, saveOptTable);

    static const auto volumeTable{[] {
        data::hier::Integer::RecvTable table;
        table.onSet_ = data::map(&Settings::onVolume);
        return table;
    }()};
    amend(volume_, volumeTable);

    static const auto bootVolEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map(&Settings::onBootVolumeEnable);
        return table;
    }()};
    amend(bootVolume_.enable_, bootVolEnableTable);

    static const auto filterEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map(&Settings::onFilterEnableSet);
        return table;
    }()};
    amend(filter_.enable_, filterEnableTable);

    static const auto disableTalkieTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map(&Settings::onDisableTalkieSet);
        return table;
    }()};
    amend(disableTalkie_, disableTalkieTable);

    volume_.update({.min_=0, .max_=4000, .inc_=50});
    volume_.set(1000);

    bootVolume_.value_.update({.min_=0, .max_=4000, .inc_=50});
    bootVolume_.value_.set(1000);

    filter_.cutoff_.update({.min_=1, .max_=10000, .inc_=10});
    filter_.cutoff_.set(100);

    filter_.order_.update({.min_=1, .max_=2560});
    filter_.order_.set(8);

    clashThreshold_.update({.min_=0.1, .max_=5, .inc_=0.1});
    clashThreshold_.set(3.0);

    pliOffTime_.update({.min_=1, .max_=3600});
    pliOffTime_.set(10);

    idleOffTime_.update({.min_=1, .max_=30000});
    idleOffTime_.set(10);

    motionTimeout_.update({.min_=1, .max_=30000});
    motionTimeout_.set(15);

    orientation_.update(eOrient_Max);
    orientation_.choose(eOrient_Fets_Towards_Blade);

    orientationRotation_.x_.update({.min_=-90, .max_=90});
    orientationRotation_.y_.update({.min_=-90, .max_=90});
    orientationRotation_.z_.update({.min_=-90, .max_=90});

    audioClashSuppressionLevel_.update({.min_=1, .max_=50});
    audioClashSuppressionLevel_.set(10);
}

Settings::~Settings() = default;

void Settings::onActivate() {
    onSaveOptSet();
    onBootVolumeEnable();
    onFilterEnableSet();
}

auto Settings::children() const -> std::vector<const Model *> {
    return {
		&massStorage_,
		&webUsb_,

		&bladeAwareness_,

		&volume_,
		&bootVolume_.enable_,
		&bootVolume_.value_,

		&filter_.enable_,
		&filter_.cutoff_,
		&filter_.order_,

		&clashThreshold_,

		&pliOffTime_,
		&idleOffTime_,
		&motionTimeout_,

		&disableColorChange_,
		&disableBasicParserStyles_,
		&disableTalkie_,
		&disableDiagnosticCommands_,

		&saveState_,

		&enableAllEditOptions_,

		&saveVolume_,
		&savePreset_,
		&saveColorChange_,

		&enableOled_,

		&orientation_,

		&orientationRotation_.x_,
		&orientationRotation_.y_,
		&orientationRotation_.z_,

		&dynamicBladeDimming_,
		&dynamicBladeLength_,
		&dynamicClashThreshold_,

		&saveBladeDimming_,
		&saveClashThreshold_,

		&audioClashSuppressionLevel_,
		&dontUseGyroForClash_,

		&noRepeatRandom_,
		&femaleTalkie_,
		&killOldPlayers_,

		&defines_,
    };
}

void Settings::processDefines() {
    processAction(std::make_unique<ProcessDefinesAction>());
}

void Settings::onSaveOptSet() {
    auto saveState{data::context(saveState_)};
    auto enableAllEditOptions{data::context(enableAllEditOptions_)};
    auto saveVolume{data::context(saveVolume_)};
    auto savePreset{data::context(savePreset_)};
    auto saveColorChange{data::context(saveColorChange_)};
    auto saveBladeDimming{data::context(saveBladeDimming_)};
    auto saveClashThreshold{data::context(saveClashThreshold_)};
    auto dynamicBladeLength{data::context(dynamicBladeLength_)};
    auto dynamicBladeDimming{data::context(dynamicBladeDimming_)};
    auto dynamicClashThreshold{data::context(dynamicClashThreshold_)};

    bool stateOrAll{saveState.val() or enableAllEditOptions.val()};

    saveVolume |= stateOrAll;
    saveVolume.enable(not stateOrAll);

    savePreset |= saveState.val();
    savePreset.enable(not saveState.val());

    saveColorChange |= stateOrAll;
    saveColorChange.enable(not stateOrAll);

    saveBladeDimming |= stateOrAll and dynamicBladeDimming.val();
    saveBladeDimming.enable(dynamicBladeDimming.val() and not stateOrAll);
    saveClashThreshold |=
        enableAllEditOptions.val() and dynamicClashThreshold.val();
    saveClashThreshold.enable(
        dynamicClashThreshold.val() and not enableAllEditOptions.val()
    );

    dynamicBladeLength |= enableAllEditOptions.val();
    dynamicBladeLength.enable(not enableAllEditOptions.val());
    dynamicBladeDimming |= enableAllEditOptions.val();
    dynamicBladeDimming.enable(not enableAllEditOptions.val());
    dynamicClashThreshold |= enableAllEditOptions.val();
    dynamicClashThreshold.enable(not enableAllEditOptions.val());
}

void Settings::onVolume() {
    auto ctxt{data::context(volume_)};

    auto bootVolume{data::context(bootVolume_.value_)};
    auto params{bootVolume.params()};
    params.max_ = ctxt.val();
    bootVolume.update(params);
}

void Settings::onBootVolumeEnable() {
    auto ctxt{data::context(bootVolume_.enable_)};
    bootVolume_.value_.enable(ctxt.val());
}

void Settings::onFilterEnableSet() {
    auto ctxt{data::context(filter_.enable_)};
    filter_.order_.enable(ctxt.val());
    filter_.cutoff_.enable(ctxt.val());
}

void Settings::onDisableTalkieSet() {
    auto ctxt{data::context(filter_.enable_)};
    femaleTalkie_.enable(not ctxt.val());
}

Settings::ProcessDefinesAction::ProcessDefinesAction() = default;

bool Settings::ProcessDefinesAction::setup() {
    // TODO: Validate if this needs to happen.
    return true;
}

void Settings::ProcessDefinesAction::perform() {
    auto& settings{source<Settings>()};
    auto& logger{logging::Context::getGlobal().createLogger("config::Settings::ProcessDefinesAction")};

    auto defines{data::context(settings.defines_)};

    using namespace priv;

    // First for builtins
    for (auto idx{0}; idx < defines.children().size(); ++idx) {
        auto& define{dynamic_cast<settings::Define&>(
            *defines.children()[idx]
        )};

        if (processDefine(settings, define, logger)) {
            defines.remove(idx);
            --idx;
        }
    }

    auto& config{settings.root<Config>()};
    if (auto propVec{config.propVec()}) {
        for (auto idx{0}; idx < defines.children().size(); ++idx) {
            auto& define{dynamic_cast<settings::Define&>(
                *defines.children()[idx]
            )};

            if (processPropDefine(*propVec, define)) {
                defines.remove(idx);
                --idx;
            }
        }
    }
}

void config::Settings::ProcessDefinesAction::retract() {
    // Nothing to do here. The only thing this action does is cause other
    // actions, and reverting those is the job of the root.
}

namespace {
using namespace config::priv;

// These have their own functions (and own scope) to keep all the access to
// the define contained, so that if the caller removes it, there aren't
// dangling usages.
bool processDefine(
    Settings& settings,
    settings::Define& defModel,
    logging::Logger& logger
) {
    auto define{data::context(defModel.name_)};
    auto value{data::context(defModel.value_)};

    bool processed{true};

    if (
            define.val() == NUM_BLADES_STR or
            define.val() == ENABLE_AUDIO_STR or
            define.val() == ENABLE_MOTION_STR or
            define.val() == ENABLE_WS2811_STR or
            define.val() == ENABLE_SD_STR or
            define.val() == SHARED_POWER_PINS_STR or
            define.val() == KEEP_SAVEFILES_STR or
            define.val() == NUM_BUTTONS_STR
       ) {
        // Do nothing
    // } else if (define.val() == RFID_SERIAL_STR) {
    // TODO: Not Yet Implemented
    } else if (define.val() == BLADE_DETECT_PIN_STR) {
        auto& bladeDetect{settings.bladeAwareness_.bladeDetect_};
        bladeDetect.enable_.set(true);
        bladeDetect.pin_.change(std::string(value.val()));
    } else if (define.val() == BLADE_ID_CLASS_STR) {
        auto& bladeId{settings.bladeAwareness_.bladeId_};
        bladeId.enable_.set(true);

        size mode{0};
        for (; mode < eBIDMode_Max; ++mode) {
            if (define.val().starts_with(BLADEID_MODE_STRS[mode])) break;
        }

        if (mode == eBIDMode_Max) {
            logger.warn("Cannot parse invalid/unrecognized BladeID class");
        } else {
            bladeId.mode_.choose(static_cast<int32>(mode));

            std::string str{value.val()};
            str.erase(0, BLADEID_MODE_STRS[mode].length());

            const auto idPinEnd{str.find(',')};
            bladeId.pin_.change(str.substr(0, idPinEnd));

            if (mode == eBIDMode_External) {
                if (idPinEnd == std::string::npos) {
                    logger.warn("Missing pullup value for external blade id");
                } else {
                    str.erase(0, idPinEnd + 1);

                    auto val{utils::doStringMath(str)};
                    if (val) {
                        bladeId.pullup_.set(static_cast<int32>(*val));
                    } else logger.warn("Failed to parse pullup value for ext blade id");
                }
            } else if (mode == eBIDMode_Bridged) {
                if (idPinEnd == std::string::npos) {
                    logger.warn("Missing bridge pin for blade id");
                } else {
                    str.erase(0, idPinEnd + 1);
                    bladeId.bridgePin_.change(std::string(str));
                }
            }
        }
    } else if (define.val() == ENABLE_POWER_FOR_ID_STR) {
        auto& bladeId{settings.bladeAwareness_.bladeId_};
        bladeId.powerForId_.set(true);

        if (not value.val().starts_with(POWER_PINS_STR)) {
            logger.warn("Failed to parse BladeID PowerPINS");
        } else {
            std::string str{value.val()};
            str.erase(0, POWER_PINS_STR.length());

            auto powerPins{data::context(bladeId.powerPins_)};

            while (not false) {
                const auto endPos{str.find(',')};

                powerPins.select(str.substr(0, endPos));

                if (endPos == std::string::npos) break;

                str.erase(0, endPos + 1);
            }
        }
    } else if (define.val() == BLADE_ID_SCAN_MILLIS_STR) {
        auto& bladeId{settings.bladeAwareness_.bladeId_};
        bladeId.continuous_.enable_.set(true);

        auto val{utils::doStringMath(value.val())};
        if (val) {
            bladeId.continuous_.interval_.set(static_cast<int32>(*val));
        } else logger.warn("Failed to parse blade id scan interval");
    } else if (define.val() == BLADE_ID_TIMES_STR) {
        auto& bladeId{settings.bladeAwareness_.bladeId_};
        bladeId.continuous_.enable_.set(true);

        auto val{utils::doStringMath(value.val())};
        if (val) {
            bladeId.continuous_.times_.set(static_cast<int32>(*val));
        } else logger.warn("Failed to parse blade id scan times");
    } else if (define.val() == VOLUME_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.volume_.set(static_cast<int32>(*val));
        } else logger.warn("Failed to parse volume");
    } else if (define.val() == BOOT_VOLUME_STR) {
        settings.bootVolume_.enable_.set(true);

        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.bootVolume_.value_.set(static_cast<int32>(*val));
        } else logger.warn("Failed to parse boot volume");
    } else if (define.val() == CLASH_THRESHOLD_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.clashThreshold_.set(*val);
        } else logger.warn("Failed to parse clash threshold");
    } else if (define.val() == PLI_OFF_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.pliOffTime_.set(*val / 1000);
        } else logger.warn("Failed to parse PLI off time");
    } else if (define.val() == IDLE_OFF_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            const auto frac{*val / (60 * 1000)};
            settings.idleOffTime_.set(frac);
        } else logger.warn("Failed to parse idle off time");
    } else if (define.val() == MOTION_TIMEOUT_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            const auto frac{*val / (60 * 1000)};
            settings.motionTimeout_.set(frac);
        } else logger.warn("Failed to parse motion timeout");
    } else if (define.val() == DISABLE_COLOR_CHANGE_STR) {
        settings.disableColorChange_.set(true);
    } else if (define.val() == DISABLE_BASIC_PARSERS_STR) {
        settings.disableBasicParserStyles_.set(true);
    } else if (define.val() == DISABLE_DIAG_COMMANDS_STR) {
        settings.disableDiagnosticCommands_.set(true);
    // } else if (define.val() == ENABLE_DEV_COMMANDS_STR) {
    //     enableDeveloperCommands = true;
    } else if (define.val() == SAVE_STATE_STR) {
        settings.saveState_.set(true);
    } else if (define.val() == ENABLE_ALL_EDIT_OPTIONS_STR) {
        settings.enableAllEditOptions_.set(true);
    } else if (define.val() == SAVE_COLOR_STR) {
        settings.saveColorChange_.set(true);
    } else if (define.val() == SAVE_VOLUME_STR) {
        settings.saveVolume_.set(true);
    } else if (define.val() == SAVE_PRESET_STR) {
        settings.savePreset_.set(true);
    } else if (define.val() == ENABLE_OLED_STR) {
        settings.enableOled_.set(true);
    } else if (define.val() == ORIENTATION_STR) {
        size orient{0};
        for (; orient < eOrient_Max; ++orient) {
            if (value.val() == ORIENTATION_STRS[orient]) break;
        }

        if (orient == eOrient_Max) {
            logger.warn("Unknown/invalid orientation");
        } else {
            settings.orientation_.choose(static_cast<int32>(orient));
        }
    } else if (define.val() == ORIENTATION_ROTATION_STR) {
        const auto firstComma{value.val().find(',')};
        const auto secondComma{value.val().find(',', firstComma + 1)};

        if (
                firstComma == std::string::npos or
                secondComma == std::string::npos
           ) {
            logger.warn("Invalid formatting for orientation rotation");
        } else {
            const auto& str{value.val()};
            auto xStr{str.substr(0, firstComma)};
            auto yStr{str.substr(
                firstComma + 1, secondComma - firstComma - 1
            )};
            auto zStr{str.substr(
                secondComma + 1, str.length() - secondComma - 1
            )};

            auto xVal{utils::doStringMath(xStr)};
            auto yVal{utils::doStringMath(yStr)};
            auto zVal{utils::doStringMath(zStr)};

            auto& rot{settings.orientationRotation_};

            if (xVal) rot.x_.set(static_cast<int32>(*xVal));
            else logger.warn("Failed to parse orientation rotation X");

            if (yVal) rot.y_.set(static_cast<int32>(*yVal));
            else logger.warn("Failed to parse orientation rotation Y");

            if (zVal) rot.z_.set(static_cast<int32>(*zVal));
            else logger.warn("Failed to parse orientation rotation Z");
        }
    // } else if (define.val() == SPEAK_TOUCH_VALUES_STR) {
    //     speakTouchValues = true;
    } else if (define.val() == DYNAMIC_BLADE_DIMMING_STR) {
        settings.dynamicBladeDimming_.set(true);
    } else if (define.val() == DYNAMIC_BLADE_LENGTH_STR) {
        settings.dynamicBladeLength_.set(true);
    } else if (define.val() == DYNAMIC_CLASH_THRESHOLD_STR) {
        settings.dynamicClashThreshold_.set(true);
    } else if (define.val() == SAVE_BLADE_DIM_STR) {
        settings.saveBladeDimming_.set(true);
    } else if (define.val() == SAVE_CLASH_THRESHOLD_STR) {
        settings.saveClashThreshold_.set(true);
    } else if (define.val() == FILTER_CUTOFF_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.filter_.enable_.set(true);
            settings.filter_.cutoff_.set(static_cast<int32>(*val));
        } else logger.warn("Failed to parse filter cutoff");
    } else if (define.val() == FILTER_ORDER_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.filter_.enable_.set(true);
            settings.filter_.order_.set(static_cast<int32>(*val));
        } else logger.warn("Failed to parse filter order");
    } else if (define.val() == AUDIO_CLASH_SUPPRESSION_STR) {
        auto val{utils::doStringMath(value.val())};
        if (val) {
            settings.audioClashSuppressionLevel_.set(
                static_cast<int32>(*val)
            );
        } else logger.warn("Failed to parse audio clash suppression");
    } else if (define.val() == DONT_USE_GYRO_FOR_CLASH_STR) {
        settings.dontUseGyroForClash_.set(true);
    } else if (define.val() == NO_REPEAT_RANDOM_STR) {
        settings.noRepeatRandom_.set(true);
    } else if (define.val() == FEMALE_TALKIE_STR) {
        settings.femaleTalkie_.set(true);
    } else if (define.val() == DISABLE_TALKIE_STR) {
        settings.disableTalkie_.set(true);
    } else if (define.val() == KILL_OLD_PLAYERS_STR) {
        settings.killOldPlayers_.set(true);
    } else {
        processed = false;
    }

    return processed;
}

bool processPropDefine(
    std::span<const std::unique_ptr<versions::props::Prop>> propVec,
    settings::Define& define
) {
    auto name{data::context(define.name_)};
    auto value{data::context(define.value_)};
    auto numVal{utils::doStringMath(value.val())};

    bool used{false};
    for (const auto& prop : propVec) {
        auto *setting{prop->find(name.val())};

        using namespace versions::props;

        if (auto *ptr{dynamic_cast<data::hier::Bool *>(setting)}) {
            ptr->set(true);
        } else if (auto *ptr{dynamic_cast<Integer *>(setting)}) {
            if (numVal)
                ptr->set(static_cast<int32>(*numVal));
        } else if (auto *ptr{dynamic_cast<Decimal *>(setting)}) {
            if (numVal)
                ptr->set(*numVal);
        }

        used = true;
    }

    return used;
}

} // namespace

