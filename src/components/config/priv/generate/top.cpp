#include "top.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/generate/top.cpp
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

#include "config/blades/bladeconfig.hpp"
#include "config/priv/io.hpp"
#include "config/settings/define.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"

using namespace config;
using namespace config::priv;

namespace {

void forGeneral(std::ostream&, const Config&);
void forProp(std::ostream&, const Config&);

} // namespace

void gen::top(std::ostream& out, const Config& config) {
    out << "#ifdef CONFIG_TOP\n";
    forGeneral(out, config);
    forProp(out, config);
    out << "#endif\n";
}

namespace {

void outputOpt(std::ostream& out, std::string_view opt) {
    out << priv::PC_OPT_STR << opt << '\n';
}

template <typename VAL>
void outputOpt(std::ostream& out, std::string_view opt, const VAL& val) {
    out << priv::PC_OPT_STR << opt << ' ' << val << '\n';
}

void outputDefine(std::ostream& out, std::string_view define) {
    out << priv::DEFINE_STR << define << '\n';
}

template <typename VAL>
void outputDefine(
    std::ostream& out, const std::string& define, const VAL& val
) {
    out << priv::DEFINE_STR << define << ' ' << val << '\n';
}

void forGeneral(std::ostream& out, const Config& config) {
    const auto& settings{config.settings_};

    if (data::context(settings.massStorage_).val()) {
        outputOpt(out, ENABLE_MASS_STORAGE_STR);
    }

    if (data::context(settings.webUsb_).val()) {
        outputOpt(out, ENABLE_WEBUSB_STR);
    }

    auto osVersion{config.os()->version_};
    const auto osIsOrOver8{utils::Version(8).compare(osVersion) <= 0};

    outputOpt(out, OS_VERSION_STR, osVersion.string());

    out << INCLUDE_STR << '"' << config.board()->include_ << '"' << '\n';

    uint32 requiredLedsPerStrip{0};
    auto bladeConfigs{data::context(config.bladeConfigs_)};
    for (const auto& model : bladeConfigs.children()) {
        auto& array{dynamic_cast<blades::BladeConfig&>(*model)};
        auto blades{data::context(array.blades_)};

        for (const auto& model : blades.children()) {
            auto& blade{dynamic_cast<blades::Blade&>(*model)};

            auto type{data::context(blade.type())};
            if (type.choiceIdx() != blades::Blade::eWS281X) continue;

            auto hasWhite{data::context(blade.ws281x().hasWhite_)};
            auto length{data::context(blade.ws281x().length_)};

            auto ledsInStrip{length.val()};
            if (hasWhite.val()) {
                ledsInStrip = std::ceil((length.val() * 4.0) / 3.0);
            }

            requiredLedsPerStrip = std::max<uint32>(
                ledsInStrip, requiredLedsPerStrip
            );
        }
    }

    out << MAX_LEDS_STR << std::max<uint32>(requiredLedsPerStrip, 144) << ";\n";

    if (utils::Version{8}.compare(osVersion) > 0) {
        // No longer needed in OS8 and newer
        outputDefine(out, ENABLE_AUDIO_STR);
        outputDefine(out, ENABLE_MOTION_STR);
        outputDefine(out, ENABLE_WS2811_STR);
        outputDefine(out, ENABLE_SD_STR);
    }

    outputDefine(out, SHARED_POWER_PINS_STR);
    auto numBlades{data::context(config.numBlades())};
    outputDefine(out, NUM_BLADES_STR, numBlades.val());
    auto buttons{data::context(config.buttons_)};
    outputDefine(out, NUM_BUTTONS_STR, buttons.children().size());

    const auto& bladeDetect{settings.bladeAwareness_.bladeDetect_};
    if (data::context(bladeDetect.enable_).val()) {
        auto pin{data::context(bladeDetect.pin_)};
        outputDefine(out, BLADE_DETECT_PIN_STR, pin.val());
    }

    const auto& bladeID{settings.bladeAwareness_.bladeId_};
    if (data::context(bladeID.enable_).val()) {
        auto mode{data::context(bladeID.mode_)};
        std::string idString{BLADEID_MODE_STRS[mode.idx()]};

        idString += data::context(bladeID.pin_).val();
        if (mode.idx() == eBIDMode_External) {
            auto pullup{data::context(bladeID.pullup_)};
            (idString += ", ") += std::to_string(pullup.val());
        } else if (mode.idx() == eBIDMode_Bridged) {
            auto bridgePin{data::context(bladeID.bridgePin_)};
            (idString += ", ") += bridgePin.val();
        }
        idString += ">\n";
        outputDefine(out, BLADE_ID_CLASS_STR, idString);
 
        auto powerPins{data::context(bladeID.powerPins_)};
        if (
                data::context(bladeID.powerForId_).val() and
                std::ranges::any_of(powerPins.selected(), std::identity{})
           ) {
            std::string powerString{POWER_PINS_STR};

            for (auto idx{0}; idx < powerPins.items().size(); ++idx) {
                if (not powerPins.selected()[idx]) continue;

                powerString += powerPins.items()[idx];
                if (idx + 1 < powerPins.items().size()) powerString += ", ";
            }

            powerString += ">\n";
            outputDefine(out, ENABLE_POWER_FOR_ID_STR, powerString);
        }

        if (data::context(bladeID.continuous_.enable_).val()) {
            auto& continuous{bladeID.continuous_};

            auto itvl{data::context(continuous.interval_)};
            auto times{data::context(continuous.times_)};
            outputDefine(out, BLADE_ID_SCAN_MILLIS_STR, itvl.val());
            outputDefine(out, BLADE_ID_TIMES_STR, times.val());

            if (osIsOrOver8) {
                if (data::context(continuous.timeout_.enable_).val()) {
                    auto timeout{data::context(continuous.timeout_.mins_)};
                    outputDefine(
                        out,
                        BLADE_ID_SCAN_TIMEOUT_STR,
                        std::to_string(timeout.val()) + " * 60 * 1000"
                    );
                }

                if (data::context(continuous.stopWhenIgnited_).val())
                    outputDefine(out, BLADE_ID_STOP_SCAN_WHEN_IGNITED_STR);
            }
        }
    }

    auto volume{data::context(settings.volume_)};
    outputDefine(out, VOLUME_STR, volume.val());
    if (data::context(settings.bootVolume_.enable_).val()) {
        auto bootVolume{data::context(settings.bootVolume_.value_)};
        outputDefine(out, BOOT_VOLUME_STR, bootVolume.val());
    }

    auto clashThresh{data::context(settings.clashThreshold_)};
    outputDefine(out, CLASH_THRESHOLD_STR, clashThresh.val());

    auto pliOff{data::context(settings.pliOffTime_)};
    auto pliOffCalced{static_cast<uint32>(std::ceil(pliOff.val() * 1000))};
    outputDefine(out, PLI_OFF_STR, pliOffCalced);

    auto idleOff{data::context(settings.idleOffTime_)};
    auto idleOffCalced{static_cast<uint32>(std::ceil(idleOff.val() * 60))};
    const auto idleOffString{std::to_string(idleOffCalced) + " * 1000"};
    outputDefine(out, IDLE_OFF_STR, idleOffString);

    auto motionOff{data::context(settings.motionTimeout_)};
    auto motionOffCalced{static_cast<uint32>(std::ceil(motionOff.val() * 60))};
    const auto motionOffString{std::to_string(motionOffCalced) + " * 1000"};
    outputDefine(out, MOTION_TIMEOUT_STR, motionOffString);

    if (data::context(settings.disableColorChange_).val()) {
        outputDefine(out, DISABLE_COLOR_CHANGE_STR);
    }

    if (data::context(settings.disableBasicParserStyles_).val()) {
        outputDefine(out, DISABLE_BASIC_PARSERS_STR);
    }

    if (data::context(settings.disableDiagnosticCommands_).val()) {
        outputDefine(out, DISABLE_DIAG_COMMANDS_STR);
    }

    // if (config.settings.enableDeveloperCommands) outputDefine(outFile, ENABLE_DEV_COMMANDS_STR);

    if (data::context(settings.saveState_).val()) {
        outputDefine(out, SAVE_STATE_STR);
    }

    if (data::context(settings.enableAllEditOptions_).val()) {
        outputDefine(out, ENABLE_ALL_EDIT_OPTIONS_STR);
    }

    auto saveVolume{data::context(settings.saveVolume_)};
    if (saveVolume.val() and saveVolume.enabled()) {
        outputDefine(out, SAVE_VOLUME_STR);
    }

    auto savePreset{data::context(settings.savePreset_)};
    if (savePreset.val() and savePreset.enabled()) {
        outputDefine(out, SAVE_PRESET_STR);
    }

    auto saveColor{data::context(settings.saveColorChange_)};
    if (saveColor.val() and saveColor.enabled()) {
        outputDefine(out, SAVE_COLOR_STR);
    }

    if (data::context(settings.enableOled_).val()) {
        outputDefine(out, ENABLE_OLED_STR);
    }

    auto orient{data::context(settings.orientation_)};
    if (orient.idx() != eOrient_Normal) {
        const auto& orientStr{ORIENTATION_STRS[orient.idx()]};
        outputDefine(out, ORIENTATION_STR, orientStr);
    }

    auto orientX{data::context(settings.orientationRotation_.x_)};
    auto orientY{data::context(settings.orientationRotation_.y_)};
    auto orientZ{data::context(settings.orientationRotation_.z_)};
    if (
            orientX.val() != 0 or
            orientY.val() != 0 or
            orientZ.val() != 0
       ) {
        std::string rotationStr;
        rotationStr += std::to_string(orientX.val());
        rotationStr += ", ";
        rotationStr += std::to_string(orientY.val());
        rotationStr += ", ";
        rotationStr += std::to_string(orientZ.val());
        outputDefine(out, ORIENTATION_ROTATION_STR, rotationStr);
    }

    // if (config.settings.speakTouchValues) outputDefine(outFile, SPEAK_TOUCH_VALUES_STR);

    auto dynDim{data::context(settings.dynamicBladeDimming_)};
    if (dynDim.val() and dynDim.enabled()) {
        outputDefine(out, DYNAMIC_BLADE_DIMMING_STR);
    }

    auto dynLen{data::context(settings.dynamicBladeLength_)};
    if (dynLen.val() and dynLen.enabled()) {
        outputDefine(out, DYNAMIC_BLADE_LENGTH_STR);
    }

    auto dynClash{data::context(settings.dynamicClashThreshold_)};
    if (dynClash.val() and dynClash.enabled()) {
        outputDefine(out, DYNAMIC_CLASH_THRESHOLD_STR);
    }

    auto saveDim{data::context(settings.saveBladeDimming_)};
    if (saveDim.val() and saveDim.enabled()) {
        outputDefine(out, SAVE_BLADE_DIM_STR);
    }

    auto saveClash{data::context(settings.saveClashThreshold_)};
    if (saveClash.val() and saveClash.enabled()) {
        outputDefine(out, SAVE_CLASH_THRESHOLD_STR);
    }

    if (data::context(settings.filter_.enable_).val()) {
        auto cutoff{data::context(settings.filter_.cutoff_)};
        auto order{data::context(settings.filter_.order_)};
        outputDefine(out, FILTER_CUTOFF_STR, cutoff.val());
        outputDefine(out, FILTER_ORDER_STR, order.val());
    }

    auto clashSuppress{data::context(settings.audioClashSuppressionLevel_)};
    if (clashSuppress.val() != 10) {
        outputDefine(out, AUDIO_CLASH_SUPPRESSION_STR, clashSuppress.val());
    }

    if (data::context(settings.dontUseGyroForClash_).val()) {
        outputDefine(out, DONT_USE_GYRO_FOR_CLASH_STR);
    }

    if (data::context(settings.femaleTalkie_).val()) {
        outputDefine(out, FEMALE_TALKIE_STR);
    }

    if (data::context(settings.disableTalkie_).val()) {
        outputDefine(out, DISABLE_TALKIE_STR);
    }

    if (osIsOrOver8) {
        if (data::context(settings.enableIdleSound_).val()) {
            outputDefine(out, ENABLE_IDLE_SOUND_STR);
        }

        if (data::context(settings.mountSdSetting_).val()) {
            outputDefine(out, MOUNT_SD_SETTING_STR);
        }

        if (data::context(settings.disableKillOldPlayers_).val()) {
            outputDefine(out, DISABLE_KILL_OLD_PLAYERS_STR);
        }

        if (data::context(settings.disableNoRepeatRandom_).val()) {
            outputDefine(out, DISABLE_NO_REPEAT_RANDOM_STR);
        }
    } else {
        // Default on newer version
        if (data::context(settings.killOldPlayers_).val()) {
            outputDefine(out, KILL_OLD_PLAYERS_STR);
        }

        if (data::context(settings.noRepeatRandom_).val()) {
            outputDefine(out, NO_REPEAT_RANDOM_STR);
        } 
    }

    auto defines{data::context(settings.defines_)};
    for (const auto& model : defines.children()) {
        auto& define{dynamic_cast<settings::Define&>(*model)};

        auto name{data::context(define.name_)};
        if (name.val().empty())
            continue;

        auto value{data::context(define.value_)};
        if (value.val().empty())
            outputDefine(out, name.val());
        else
            outputDefine(out, name.val(), value.val());
    }
}

void forProp(std::ostream& out, const Config& config) {
    auto *prop{config.prop()};

    if (prop == nullptr) return;
    
    for (const auto& setting : prop->settings()) {
        auto output{setting->generateDefineString()};

        if (not output) continue;

        out << priv::DEFINE_STR << *output << '\n';
    }
}

} // namespace

