#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/settings/settings.h
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

#include <utils/types.h>

namespace Config {

struct Settings {
    constexpr static cstring NUM_BLADES_STR{"NUM_BLADES"};
    constexpr static cstring NUM_BUTTONS_STR{"NUM_BUTTONS"};
    constexpr static cstring EN_AUDIO_STR{"ENABLE_AUDIO"};
    constexpr static cstring EN_MOTION_STR{"ENABLE_MOTION"};
    constexpr static cstring EN_WS2811_STR{"ENABLE_WS2811"};
    constexpr static cstring EN_SD_STR{"ENABLE_SD"};
    constexpr static cstring SHARED_POWER_PINS_STR{"SHARED_POWER_PINS"};
    constexpr static cstring KEEP_SAVEFILES_STR{"KEEP_SAVEFILES_WHEN_PROGRAMMING"};

    constexpr static cstring SAVE_STATE_STR{"SAVE_STATE"};
    constexpr static cstring ALL_EDIT_OPTS_STR{"ENABLE_ALL_EDIT_OPTIONS"};

    constexpr static cstring RFID_SERIAL_STR{"RFID_SERIAL"};

    // SAVE_STATE Sets:
    // - saveVolume
    // - savePreset
    // - saveColorChange
    // - saveBladeDimming (only if dynamicBladeDimming is set)
    
    // ENABLE_ALL_EDIT_OPTIONS Sets:
    // - dynamicBladeLength
    // - dynamicBladeDimming
    // - dynamicClashThreshold
    // - saveVolume
    // - saveColorChange
    // - saveBladeDimming
    // - saveClashThreshold

    int32 volume{1000};
    constexpr static cstring VOLUME_STR{"VOLUME"};
    int32 bootVolume{-1};
    constexpr static cstring BOOT_VOLUME_STR{"BOOT_VOLUME"};
    float32 clashThreshold{3};
    constexpr static cstring CLASH_THRESHOLD_STR{"CLASH_THRESHOLD_G"};

    // In seconds
    int32 pliOffTime{10};
    constexpr static cstring PLI_OFF_STR{"PLI_OFF_TIME"};
    int32 idleOffTime{10 * 60};
    constexpr static cstring IDLE_OFF_STR{"IDLE_OFF_TIME"};
    int32 motionOffTime{15 * 60};
    constexpr static cstring MOTION_OFF_STR{"MOTION_TIMEOUT"};

    bool disableColorChange{false};
    constexpr static cstring DISABLE_COLOR_CHANGE_STR{"DISABLE_COLOR_CHANGE"};
    bool disableBasicParserStyles{false};
    constexpr static cstring DISABLE_BASIC_PARSERS_STR{"DISABLE_BASIC_PARSER_STYLES"};
    bool disableDiagnosticCommands{false};
    constexpr static cstring DISABLE_DIAG_COMMANDS_STR{"DISABLE_DIAGNOSTIC_COMMANDS"};
    bool enableDeveloperCommands{false};
    constexpr static cstring ENABLE_DEV_COMMANDS_STR{"ENABLE_DEVELOPER_COMMANDS"};

    bool saveColorChange{false};
    constexpr static cstring SAVE_COLOR_STR{"SAVE_COLOR_CHANGE"};
    bool saveVolume{false};
    constexpr static cstring SAVE_VOLUME_STR{"SAVE_VOLUME"};
    bool savePreset{false};
    constexpr static cstring SAVE_PRESET_STR{"SAVE_PRESET"};

    enum class Orientation {
        FETS_TOWARDS_BLADE,
        USB_TOWARDS_BLADE,
        USB_CCW_FROM_BLADE,
        USB_CW_FROM_BLADE,
        TOP_TOWARDS_BLADE,
        BOTTOM_TOWARDS_BLADE,
    } orientation{Orientation::FETS_TOWARDS_BLADE};
    constexpr static cstring ORIENTATION_STR{"ORIENTATION"};
    constexpr static cstring ORIENT_FETS_TOWARDS_BLADE_STR{"ORIENTATION_FETS_TOWARDS_BLADE"};
    constexpr static cstring ORIENT_USB_TOWARDS_BLADE_STR{"ORIENTATION_USB_TOWARDS_BLADE"};
    constexpr static cstring ORIENT_USB_CCW_FROM_BLADE_STR{"ORIENTATION_USB_CCW_FROM_BLADE"};
    constexpr static cstring ORIENT_USB_CW_FROM_BLADE_STR{"ORIENTATION_USB_CW_FROM_BLADE"};
    constexpr static cstring ORIENT_TOP_TOWARDS_BLADE_STR{"ORIENTATION_TOP_TOWARDS_BLADE"};
    constexpr static cstring ORIENT_BOTTOM_TOWARDS_BLADE_STR{"ORIENTATION_BOTTOM_TOWARDS_BLADE"};

    struct OrientRotation {
        int32 x{0};
        int32 y{0};
        int32 z{0};
    } orientationRotation{};
    constexpr static cstring ORIENTATION_ROTATION_STR{"ORIENTATION_ROTATION"};

    // For debugging touch buttons:
    bool speakTouchValues{false};
    constexpr static cstring SPEAK_TOUCH_VALUES_STR{"SPEAK_TOUCH_VALUES"};
    
    // Make these be enabled by adding such a device to the
    // wiring. Adjust volume there too? That I don't know...
    /*
    bool enableI2SOut{false};
    bool enableSPDIFOut{false};
    int32 lineOutVolume{2000}; // This can be set to `dynamic_mixer.get_volume()` to follow master
    */

    bool dynamicBladeDimming{false};
    constexpr static cstring DYN_BLADE_DIM_STR{"DYNAMIC_BLADE_DIMMING"};
    bool dynamicBladeLength{false};
    constexpr static cstring DYN_BLADE_LENGTH_STR{"DYNAMIC_BLADE_LENGTH"};
    bool dynamicClashThreshold{false};
    constexpr static cstring DYN_CLASH_THRESHOLD_STR{"DYNAMIC_CLASH_THRESHOLD"};

    // only should be settable if dynamicBladeDimming is true
    bool saveBladeDimming{false};
    constexpr static cstring SAVE_BLADE_DIM_STR{"SAVE_BLADE_DIMMING"};
    bool saveClashThreshold{false};
    constexpr static cstring SAVE_CLASH_THRESHOLD_STR{"SAVE_CLASH_THRESHOLD"};

    // Does not affect I2C or S/PDIF
    // Cutoff in Hz
    int32 filterCutoff{100};
    constexpr static cstring FILTER_CUTOFF_STR{"FILTER_CUTOFF_FREQUENCY"};
    int32 filterOrder{8};
    constexpr static cstring FILTER_ORDER_STR{"FILTER_ORDER"};

    // Useful range is 1~50
    int8 audioClashSuppressionLevel{10};
    constexpr static cstring AUDIO_CLASH_SUPPRESSION_STR{"AUDIO_CLASH_SUPPRESSION_LEVEL"};
    bool dontUseGyroForClash{false};
    constexpr static cstring DONT_USE_GYRO_FOR_CLASH_STR{"PROFFIEOS_DONT_USE_GYRO_FOR_CLASH"};

    bool noRepeatRandom{false};
    constexpr static cstring NO_REPEAT_RANDOM_STR{"NO_REPEAT_RANDOM"};
    bool femaleTalkie{false};
    constexpr static cstring FEMALE_TALKIE_STR{"FEMALE_TALKIE_VOICE"};
    bool disableTalkie{false};
    constexpr static cstring DISABLE_TALKIE_STR{"DISABLE_TALKIE"};
    bool killOldPlayers{false};
    constexpr static cstring KILL_OLD_PLAYERS_STR{"KILL_OLD_PLAYERS"};

    // POV Data?
};

} // namespace Config

