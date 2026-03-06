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
#include "config/priv/strings.hpp"
#include "config/settings/define.hpp"
#include "data/number.hpp"
#include "utils/string.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"

config::Settings::Settings(Config& parent) :
    data::Node(&parent),
    bladeAwareness_(*this) {
    using namespace priv;

    const auto onSaveOptSet{[](const data::Bool::Context& ctxt) static {
        auto& settings{*ctxt.model().parent<Settings>()};
        using BCtxt = data::Bool::Context;
        BCtxt saveState{settings.saveState_};
        BCtxt enableAllEditOptions{settings.enableAllEditOptions_};
        BCtxt saveVolume{settings.saveVolume_};
        BCtxt savePreset{settings.savePreset_};
        BCtxt saveColorChange{settings.saveColorChange_};
        BCtxt saveBladeDimming{settings.saveBladeDimming_};
        BCtxt saveClashThreshold{settings.saveClashThreshold_};
        BCtxt dynamicBladeLength{settings.dynamicBladeLength_};
        BCtxt dynamicBladeDimming{settings.dynamicBladeDimming_};
        BCtxt dynamicClashThreshold{settings.dynamicClashThreshold_};

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
    }};

    saveState_.responder().onSet_ = onSaveOptSet;
    enableAllEditOptions_.responder().onSet_ = onSaveOptSet;
    dynamicBladeDimming_.responder().onSet_ = onSaveOptSet;
    dynamicClashThreshold_.responder().onSet_ = onSaveOptSet;

    // Call to update, param doesn't matter
    onSaveOptSet(saveState_);

    data::Integer::Context{volume_}.update({
        .min_=0, .max_=4000, .inc_=50
    });
    data::Integer::Context{bootVolume_}.update({
        .min_=0, .max_=4000, .inc_=50
    });

    volume_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& settings{*ctxt.model().parent<Settings>()};
        data::Integer::Context bootVolume{settings.bootVolume_};

        auto params{bootVolume.params()};
        params.max_ = ctxt.val();
        bootVolume.update(params);
    };

    data::Integer::Context{volume_}.set(1000);
    data::Integer::Context{bootVolume_}.set(1000);

    enableBootVolume_.responder().onSet_ = [](
        const data::Bool::Context& ctxt
    ) {
        auto& settings{*ctxt.model().parent<Settings>()};
        data::Integer::Context bootVolume{settings.bootVolume_};
        bootVolume.enable(ctxt.val());
    };
    enableBootVolume_.responder().onSet_(enableBootVolume_);

    enableFiltering_.responder().onSet_ = [](
        const data::Bool::Context& ctxt
    ) {
        auto& settings{*ctxt.model().parent<Settings>()};
        data::Integer::Context order{settings.filterOrder_};
        order.enable(ctxt.val());

        data::Integer::Context cutoff{settings.filterCutoff_};
        cutoff.enable(ctxt.val());
    };
    enableFiltering_.responder().onSet_(enableFiltering_);

    { data::Integer::Context cutoff{filterCutoff_};
        cutoff.update({.min_=1, .max_=10000, .inc_=10});
        cutoff.set(100);
    }

    { data::Integer::Context order{filterOrder_};
        order.update({.min_=1, .max_=2560});
        order.set(8);
    }

    disableTalkie_.responder().onSet_ = [](const data::Bool::Context& ctxt) {
        auto& settings{*ctxt.model().parent<Settings>()};
        data::Bool::Context femaleTalkie{settings.femaleTalkie_};
        femaleTalkie.enable(not ctxt.val());
    };

    /*
    board_.setChoices({
        "Proffieboard V3",
        "Proffieboard V2",
        "Proffieboard V1",
    });
    vector<string> pinDefaults{
        "bladePin",
        "blade2Pin",
        "blade3Pin",
        "blade4Pin",
    };
    bladeId_.mode.setChoices(Utils::createEntries({
        _("Snapshot"),
        _("External Pullup"),
        _("Bridged Pullup")
    }));
        v.insert(v.begin(), "bladeIdentifyPin");
    orientation.setChoices(Utils::createEntries({
        _("FETs Towards Blade"),
        _("USB Towards Blade"),
        _("USB CCW From Blade"),
        _("USB CW From Blade"),
        _("Top Towards Blade"),
        _("Bottom Towards Blade")
    }));
        */

    { data::Decimal::Context clashThresh{clashThreshold_};
        clashThresh.update({.min_=0.1, .max_=5, .inc_=0.1});
        clashThresh.set(3.0);
    }

    { data::Decimal::Context pliOff{pliOffTime_};
        pliOff.update({.min_=1, .max_=3600});
        pliOff.set(10);
    }

    { data::Decimal::Context idleOff{idleOffTime_};
        idleOff.update({.min_=1, .max_=30000});
        idleOff.set(10);
    }

    { data::Decimal::Context motionOff{motionTimeout_};
        motionOff.update({.min_=1, .max_=30000});
        motionOff.set(15);
    }

    { data::Choice::Context orient{orientation_};
        orient.update(eOrient_Max);
        orient.choose(eOrient_Fets_Towards_Blade);
    }

    data::Integer::Context{orientationRotation_.x_}.update({
        .min_=-90, .max_=90
    });

    data::Integer::Context{orientationRotation_.y_}.update({
        .min_=-90, .max_=90
    });

    data::Integer::Context{orientationRotation_.z_}.update({
        .min_=-90, .max_=90
    });

    { data::Integer::Context suppress{audioClashSuppressionLevel_};
        suppress.update({.min_=1, .max_=50});
        suppress.set(10);
    }
}

config::Settings::~Settings() = default;

bool config::Settings::enumerate(const EnumFunc&) {
    assert(0); // TODO
}

data::Model *config::Settings::find(uint64) {
    assert(0); // TODO
}

void config::Settings::processDefines() {
    processAction(std::make_unique<ProcessDefinesAction>());
}

config::Settings::ProcessDefinesAction::ProcessDefinesAction() = default;

bool config::Settings::ProcessDefinesAction::shouldPerform(data::Model&) {
    return true;
}

void config::Settings::ProcessDefinesAction::perform(data::Model& model) {
    auto& settings{static_cast<Settings&>(model)};
    auto& logger{logging::Context::getGlobal().createLogger("config::Settings::ProcessDefinesAction")};

    data::Vector::Context defineVec{settings.defines_};
    const auto& defines{defineVec.children()};

    using namespace priv;

    // First for builtins
    for (auto idx{0}; idx < defines.size(); ++idx) {
        auto& defModel{static_cast<settings::Define&>(*defines[idx])};
        data::String::Context define{defModel.name_};
        data::String::Context value{defModel.value_};

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
            data::Bool::Context{bladeDetect.enable_}.set(true);
            data::String::Context{bladeDetect.pin_}.change(
                std::string{value.val()}, 0
            );
        } else if (define.val() == BLADE_ID_CLASS_STR) {
            auto& bladeId{settings.bladeAwareness_.bladeId_};
            data::Bool::Context{bladeId.enable_}.set(true);

            size mode{0};
            for (; mode < eBIDMode_Max; ++mode) {
                if (define.val().starts_with(BLADEID_MODE_STRS[mode])) break;
            }

            if (mode == eBIDMode_Max) {
                logger.warn("Cannot parse invalid/unrecognized BladeID class");
            } else {
                data::Choice::Context{
                    bladeId.mode_
                }.choose(static_cast<int32>(mode));

                std::string str{value.val()};
                str.erase(0, BLADEID_MODE_STRS[mode].length());

                const auto idPinEnd{str.find(',')};
                data::String::Context{bladeId.pin_}.change(
                    str.substr(0, idPinEnd), 0
                );

                if (mode == eBIDMode_External) {
                    if (idPinEnd == std::string::npos) {
                        logger.warn("Missing pullup value for external blade id");
                    } else {
                        str.erase(0, idPinEnd + 1);

                        auto val{utils::doStringMath(str)};
                        if (val) {
                            data::Integer::Context{
                                bladeId.pullup_
                            }.set(static_cast<int32>(*val));
                        } else logger.warn("Failed to parse pullup value for ext blade id");
                    }
                } else if (mode == eBIDMode_Bridged) {
                    if (idPinEnd == std::string::npos) {
                        logger.warn("Missing bridge pin for blade id");
                    } else {
                        str.erase(0, idPinEnd + 1);
                        data::String::Context{bladeId.bridgePin_}.change(
                            std::string{str}, 0
                        );
                    }
                }
            }
        } else if (define.val() == ENABLE_POWER_FOR_ID_STR) {
            auto& bladeId{settings.bladeAwareness_.bladeId_};
            data::Bool::Context{bladeId.powerForId_}.set(true);

            if (not value.val().starts_with(POWER_PINS_STR)) {
                logger.warn("Failed to parse BladeID PowerPINS");
            } else {
                std::string str{value.val()};
                str.erase(0, POWER_PINS_STR.length());

                data::Selection::Context powerPins{bladeId.powerPins_};

                while (not false) {
                    const auto endPos{str.find(',')};

                    powerPins.select(str.substr(0, endPos));

                    if (endPos == std::string::npos) break;

                    str.erase(0, endPos + 1);
                }
            }
        } else if (define.val() == BLADE_ID_SCAN_MILLIS_STR) {
            auto& bladeId{settings.bladeAwareness_.bladeId_};
            data::Bool::Context{bladeId.continuousScanning_}.set(true);            

            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Integer::Context{
                    bladeId.continuousInterval_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse blade id scan interval");
        } else if (define.val() == BLADE_ID_TIMES_STR) {
            auto& bladeId{settings.bladeAwareness_.bladeId_};
            data::Bool::Context{bladeId.continuousScanning_}.set(true);            

            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Integer::Context{
                    bladeId.continuousTimes_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse blade id scan times");
        } else if (define.val() == VOLUME_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Integer::Context{
                    settings.volume_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse volume");
        } else if (define.val() == BOOT_VOLUME_STR) {
            data::Bool::Context{settings.enableBootVolume_}.set(true);

            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Integer::Context{
                    settings.bootVolume_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse boot volume");
        } else if (define.val() == CLASH_THRESHOLD_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Decimal::Context{settings.clashThreshold_}.set(*val);
            } else logger.warn("Failed to parse clash threshold");
        } else if (define.val() == PLI_OFF_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Decimal::Context{settings.pliOffTime_}.set(*val / 1000);
            } else logger.warn("Failed to parse PLI off time");
        } else if (define.val() == IDLE_OFF_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                const auto frac{*val / (60 * 1000)};
                data::Decimal::Context{settings.idleOffTime_}.set(frac);
            } else logger.warn("Failed to parse idle off time");
        } else if (define.val() == MOTION_TIMEOUT_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                const auto frac{*val / (60 * 1000)};
                data::Decimal::Context{settings.motionTimeout_}.set(frac);
            } else logger.warn("Failed to parse motion timeout");
        } else if (define.val() == DISABLE_COLOR_CHANGE_STR) {
            data::Bool::Context{settings.disableColorChange_}.set(true);
        } else if (define.val() == DISABLE_BASIC_PARSERS_STR) {
            data::Bool::Context{settings.disableBasicParserStyles_}.set(true);
        } else if (define.val() == DISABLE_DIAG_COMMANDS_STR) {
            data::Bool::Context{settings.disableDiagnosticCommands_}.set(true);
        // } else if (define.val() == ENABLE_DEV_COMMANDS_STR) {
        //     enableDeveloperCommands = true;
        } else if (define.val() == SAVE_STATE_STR) {
            data::Bool::Context{settings.saveState_}.set(true);
        } else if (define.val() == ENABLE_ALL_EDIT_OPTIONS_STR) {
            data::Bool::Context{settings.enableAllEditOptions_}.set(true);
        } else if (define.val() == SAVE_COLOR_STR) {
            data::Bool::Context{settings.saveColorChange_}.set(true);
        } else if (define.val() == SAVE_VOLUME_STR) {
            data::Bool::Context{settings.saveVolume_}.set(true);
        } else if (define.val() == SAVE_PRESET_STR) {
            data::Bool::Context{settings.savePreset_}.set(true);
        } else if (define.val() == ENABLE_OLED_STR) {
            data::Bool::Context{settings.enableOled_}.set(true);
        } else if (define.val() == ORIENTATION_STR) {
            size orient{0};
            for (; orient < eOrient_Max; ++orient) {
                if (value.val() == ORIENTATION_STRS[orient]) break;
            }

            if (orient == eOrient_Max) {
                logger.warn("Unknown/invalid orientation");
            } else {
                data::Choice::Context{
                    settings.orientation_
                }.choose(static_cast<int32>(orient));
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

                data::Integer::Context orientX{
                    settings.orientationRotation_.x_
                };
                data::Integer::Context orientY{
                    settings.orientationRotation_.y_
                };
                data::Integer::Context orientZ{
                    settings.orientationRotation_.z_
                };

                if (xVal) orientX.set(static_cast<int32>(*xVal));
                else logger.warn("Failed to parse orientation rotation X");

                if (yVal) orientY.set(static_cast<int32>(*yVal));
                else logger.warn("Failed to parse orientation rotation Y");

                if (zVal) orientZ.set(static_cast<int32>(*zVal));
                else logger.warn("Failed to parse orientation rotation Z");
            }
        // } else if (define.val() == SPEAK_TOUCH_VALUES_STR) {
        //     speakTouchValues = true;
        } else if (define.val() == DYNAMIC_BLADE_DIMMING_STR) {
            data::Bool::Context{settings.dynamicBladeDimming_}.set(true);
        } else if (define.val() == DYNAMIC_BLADE_LENGTH_STR) {
            data::Bool::Context{settings.dynamicBladeLength_}.set(true);
        } else if (define.val() == DYNAMIC_CLASH_THRESHOLD_STR) {
            data::Bool::Context{settings.dynamicClashThreshold_}.set(true);
        } else if (define.val() == SAVE_BLADE_DIM_STR) {
            data::Bool::Context{settings.saveBladeDimming_}.set(true);
        } else if (define.val() == SAVE_CLASH_THRESHOLD_STR) {
            data::Bool::Context{settings.saveClashThreshold_}.set(true);
        } else if (define.val() == FILTER_CUTOFF_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Bool::Context{settings.enableFiltering_}.set(true);
                data::Integer::Context{
                    settings.filterCutoff_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse filter cutoff");
        } else if (define.val() == FILTER_ORDER_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Bool::Context{settings.enableFiltering_}.set(true);
                data::Integer::Context{
                    settings.filterOrder_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse filter order");
        } else if (define.val() == AUDIO_CLASH_SUPPRESSION_STR) {
            auto val{utils::doStringMath(value.val())};
            if (val) {
                data::Integer::Context{
                    settings.audioClashSuppressionLevel_
                }.set(static_cast<int32>(*val));
            } else logger.warn("Failed to parse audio clash suppression");
        } else if (define.val() == DONT_USE_GYRO_FOR_CLASH_STR) {
            data::Bool::Context{settings.dontUseGyroForClash_}.set(true);
        } else if (define.val() == NO_REPEAT_RANDOM_STR) {
            data::Bool::Context{settings.noRepeatRandom_}.set(true);
        } else if (define.val() == FEMALE_TALKIE_STR) {
            data::Bool::Context{settings.femaleTalkie_}.set(true);
        } else if (define.val() == DISABLE_TALKIE_STR) {
            data::Bool::Context{settings.disableTalkie_}.set(true);
        } else if (define.val() == KILL_OLD_PLAYERS_STR) {
            data::Bool::Context{settings.killOldPlayers_}.set(true);
        } else {
            processed = false;
        }

        if (processed) {
            defineVec.remove(idx);
            --idx;
        }
    }

    // Again for props
    struct PropProcDefAction : data::Action {
        PropProcDefAction(std::string def, std::string val) :
            mDef{std::move(def)}, mVal{std::move(val)} {}

        bool shouldPerform(data::Model& model) override {
            // TODO: Can this be setup to actually use the respective action's
            // check rather than trying to emulate it?
            if (auto *ptr = dynamic_cast<data::Bool *>(&model)) {
                return not data::Bool::Context{*ptr}.val();
            }
            if (auto *ptr = dynamic_cast<data::Integer *>(&model)) {
                data::Integer::Context intgr{*ptr};
                return intgr.val() != utils::doStringMath(mVal);
            } 
            if (auto *ptr = dynamic_cast<data::Decimal *>(&model)) {
                data::Decimal::Context intgr{*ptr};
                return intgr.val() != utils::doStringMath(mVal);
            }

            assert(0);
            __builtin_unreachable();
        }

        void perform(data::Model& model) override {
            if (auto *ptr = dynamic_cast<data::Bool *>(&model)) {
                data::Bool::Context{*ptr}.set(true);
                return;
            }

            auto num{utils::doStringMath(mVal)};
            if (auto *ptr = dynamic_cast<data::Integer *>(&model)) {
                data::Integer::Context intgr{*ptr};
                intgr.set(static_cast<int32>(num.value_or(0)));
            } else if (auto *ptr = dynamic_cast<data::Decimal *>(&model)) {
                data::Decimal::Context dec{*ptr};
                dec.set(num.value_or(0));
            }
        }

        void retract(data::Model&) override {
            // We only performed actions. Those will be undone automatically.
        }

    private:
        const std::string mDef;
        const std::string mVal;
    };

    auto& config{*settings.parent<Config>()};
    if (config.props()) {
        data::Vector::ROContext props{*config.props()};
        for (auto idx{0}; idx < defines.size(); ++idx) {
            auto& defModel{static_cast<settings::Define&>(*defines[idx])};
            data::String::Context define{defModel.name_};
            data::String::Context value{defModel.value_};

            bool used{false};
            for (const auto& propModel : props.children()) {
                auto& node{static_cast<Node&>(*propModel)};

                auto action{std::make_unique<PropProcDefAction>(
                    define.val(), value.val()
                )};
                auto res{node.forwardAction(std::move(action))};
            }

            if (used) {
                defineVec.remove(idx);
                --idx;
            }
        }
    }
}

void config::Settings::ProcessDefinesAction::retract(data::Model& model) {
    // Nothing to do here. The only thing this action does is cause other
    // actions, and reverting those is the job of the root.
}

