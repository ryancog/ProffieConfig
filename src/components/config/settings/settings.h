#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
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

#include "ui/controls/checklist.h"
#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"
#include "ui/controls/numeric.h"
#include "ui/notifier.h"
#include "utils/types.h"

#include "../private/export.h"

namespace Config {

struct Config;

struct CONFIG_EXPORT Settings {
    /**
     * Set up defaults and update handlers
     */
    Settings(Config&);

    enum BoardVersion {
        PROFFIEBOARDV3,
        PROFFIEBOARDV2,
        PROFFIEBAORDV1,
    };
    PCUI::ChoiceData board;

    // Do NOT set choices manually
    // Done via Config::refreshVersions
    PCUI::ChoiceData osVersion;
    constexpr static cstring OS_VERSION_STR{"OS_VERSION"};

    PCUI::ToggleData massStorage;
    constexpr static cstring ENABLE_MASS_STORAGE_STR{"ENABLE_MASS_STORAGE"};
    PCUI::ToggleData webUSB;
    constexpr static cstring ENABLE_WEBUSB_STR{"ENABLE_WEBUSB"};

    PCUI::NumericData numButtons;
    constexpr static cstring NUM_BUTTONS_STR{"NUM_BUTTONS"};

    constexpr static cstring NUM_BLADES_STR{"NUM_BLADES"};
    constexpr static cstring EN_AUDIO_STR{"ENABLE_AUDIO"};
    constexpr static cstring EN_MOTION_STR{"ENABLE_MOTION"};
    constexpr static cstring EN_WS2811_STR{"ENABLE_WS2811"};
    constexpr static cstring EN_SD_STR{"ENABLE_SD"};
    constexpr static cstring SHARED_POWER_PINS_STR{"SHARED_POWER_PINS"};
    constexpr static cstring KEEP_SAVEFILES_STR{"KEEP_SAVEFILES_WHEN_PROGRAMMING"};

    // PCUI::ChoiceData rfidSerial;
    constexpr static cstring RFID_SERIAL_STR{"RFID_SERIAL"};

    PCUI::ToggleData bladeDetect;
    PCUI::ComboBoxData bladeDetectPin;
    constexpr static cstring BLADE_DETECT_PIN_STR{"BLADE_DETECT_PIN"};

    struct BladeID {
        PCUI::ToggleData enable;
        PCUI::ComboBoxData pin;
        enum Mode {
            SNAPSHOT = 0,
            EXTERNAL,
            BRIDGED,
        };
        PCUI::ChoiceData mode;
        PCUI::ComboBoxData bridgePin;
        PCUI::NumericData pullup;
        PCUI::ToggleData powerForID;
        PCUI::CheckListData powerPins;
        PCUI::ToggleData continuousScanning;
        PCUI::NumericData continuousInterval;
        PCUI::NumericData continuousTimes;
    } bladeID;
    constexpr static cstring BLADE_ID_CLASS_STR{"BLADE_ID_CLASS"};
    constexpr static cstring ENABLE_POWER_FOR_ID_STR{"ENABLE_POWER_FOR_ID"};
    constexpr static cstring BLADE_ID_SCAN_MILLIS_STR{"BLADE_ID_SCAN_MILLIS"};
    constexpr static cstring BLADE_ID_TIMES_STR{"BLADE_ID_TIMES"};

    PCUI::NumericData volume;
    constexpr static cstring VOLUME_STR{"VOLUME"};
    PCUI::ToggleData enableBootVolume;
    PCUI::NumericData bootVolume;
    constexpr static cstring BOOT_VOLUME_STR{"BOOT_VOLUME"};
    PCUI::DecimalData clashThreshold;
    constexpr static cstring CLASH_THRESHOLD_STR{"CLASH_THRESHOLD_G"};

    // In minutes
    PCUI::DecimalData pliOffTime;
    constexpr static cstring PLI_OFF_STR{"PLI_OFF_TIME"};
    PCUI::DecimalData idleOffTime;
    constexpr static cstring IDLE_OFF_STR{"IDLE_OFF_TIME"};
    PCUI::DecimalData motionOffTime;
    constexpr static cstring MOTION_OFF_STR{"MOTION_TIMEOUT"};

    PCUI::ToggleData disableColorChange;
    constexpr static cstring DISABLE_COLOR_CHANGE_STR{"DISABLE_COLOR_CHANGE"};
    PCUI::ToggleData disableBasicParserStyles;
    constexpr static cstring DISABLE_BASIC_PARSERS_STR{"DISABLE_BASIC_PARSER_STYLES"};
    PCUI::ToggleData disableDiagnosticCommands;
    constexpr static cstring DISABLE_DIAG_COMMANDS_STR{"DISABLE_DIAGNOSTIC_COMMANDS"};
    PCUI::ToggleData enableDeveloperCommands;
    constexpr static cstring ENABLE_DEV_COMMANDS_STR{"ENABLE_DEVELOPER_COMMANDS"};

    // SAVE_STATE Sets:
    // - saveVolume
    // - savePreset
    // - saveColorChange
    // - saveBladeDimming (only if dynamicBladeDimming is set)
    PCUI::ToggleData saveState;
    constexpr static cstring SAVE_STATE_STR{"SAVE_STATE"};

    // ENABLE_ALL_EDIT_OPTIONS Sets:
    // - dynamicBladeLength
    // - dynamicBladeDimming
    // - dynamicClashThreshold
    // - saveVolume
    // - saveColorChange
    // - saveBladeDimming
    // - saveClashThreshold
    PCUI::ToggleData enableAllEditOptions;
    constexpr static cstring ENABLE_ALL_EDIT_OPTIONS_STR{"ENABLE_ALL_EDIT_OPTIONS"};

    PCUI::ToggleData saveColorChange;
    constexpr static cstring SAVE_COLOR_STR{"SAVE_COLOR_CHANGE"};
    PCUI::ToggleData saveVolume;
    constexpr static cstring SAVE_VOLUME_STR{"SAVE_VOLUME"};
    PCUI::ToggleData savePreset;
    constexpr static cstring SAVE_PRESET_STR{"SAVE_PRESET"};

    PCUI::ToggleData enableOLED;
    constexpr static cstring ENABLE_OLED_STR{"ENABLE_SSD1306"};

    enum Orientation {
        FETS_TOWARDS_BLADE,
        USB_TOWARDS_BLADE,
        USB_CCW_FROM_BLADE,
        USB_CW_FROM_BLADE,
        TOP_TOWARDS_BLADE,
        BOTTOM_TOWARDS_BLADE,
    };
    PCUI::ChoiceData orientation;
    constexpr static cstring ORIENTATION_STR{"ORIENTATION"};
    constexpr static cstring ORIENT_FETS_TOWARDS_BLADE_STR{"ORIENTATION_FETS_TOWARDS_BLADE"};
    constexpr static cstring ORIENT_USB_TOWARDS_BLADE_STR{"ORIENTATION_USB_TOWARDS_BLADE"};
    constexpr static cstring ORIENT_USB_CCW_FROM_BLADE_STR{"ORIENTATION_USB_CCW_FROM_BLADE"};
    constexpr static cstring ORIENT_USB_CW_FROM_BLADE_STR{"ORIENTATION_USB_CW_FROM_BLADE"};
    constexpr static cstring ORIENT_TOP_TOWARDS_BLADE_STR{"ORIENTATION_TOP_TOWARDS_BLADE"};
    constexpr static cstring ORIENT_BOTTOM_TOWARDS_BLADE_STR{"ORIENTATION_BOTTOM_TOWARDS_BLADE"};

    struct OrientRotation {
        PCUI::NumericData x;
        PCUI::NumericData y;
        PCUI::NumericData z;
    } orientationRotation;
    constexpr static cstring ORIENTATION_ROTATION_STR{"ORIENTATION_ROTATION"};

    // For debugging touch buttons:
    PCUI::ToggleData speakTouchValues;
    constexpr static cstring SPEAK_TOUCH_VALUES_STR{"SPEAK_TOUCH_VALUES"};
    
    // Make these be enabled by adding such a device to the
    // wiring. Adjust volume there too? That I don't know...
    /*
    bool enableI2SOut{false};
    bool enableSPDIFOut{false};
    int32 lineOutVolume{2000}; // This can be set to `dynamic_mixer.get_volume()` to follow master
    */

    PCUI::ToggleData dynamicBladeDimming;
    constexpr static cstring DYNAMIC_BLADE_DIMMING_STR{"DYNAMIC_BLADE_DIMMING"};
    PCUI::ToggleData dynamicBladeLength;
    constexpr static cstring DYNAMIC_BLADE_LENGTH_STR{"DYNAMIC_BLADE_LENGTH"};
    PCUI::ToggleData dynamicClashThreshold;
    constexpr static cstring DYNAMIC_CLASH_THRESHOLD_STR{"DYNAMIC_CLASH_THRESHOLD"};

    // only should be settable if dynamicBladeDimming is true
    PCUI::ToggleData saveBladeDimming;
    constexpr static cstring SAVE_BLADE_DIM_STR{"SAVE_BLADE_DIMMING"};
    PCUI::ToggleData saveClashThreshold;
    constexpr static cstring SAVE_CLASH_THRESHOLD_STR{"SAVE_CLASH_THRESHOLD"};

    // Does not affect I2C or S/PDIF
    // Cutoff in Hz
    PCUI::NumericData filterCutoff;
    constexpr static cstring FILTER_CUTOFF_STR{"FILTER_CUTOFF_FREQUENCY"};
    PCUI::NumericData filterOrder;
    constexpr static cstring FILTER_ORDER_STR{"FILTER_ORDER"};

    // Useful range is 1~50
    PCUI::NumericData audioClashSuppressionLevel;
    constexpr static cstring AUDIO_CLASH_SUPPRESSION_STR{"AUDIO_CLASH_SUPPRESSION_LEVEL"};
    PCUI::ToggleData dontUseGyroForClash;
    constexpr static cstring DONT_USE_GYRO_FOR_CLASH_STR{"PROFFIEOS_DONT_USE_GYRO_FOR_CLASH"};

    PCUI::ToggleData noRepeatRandom;
    constexpr static cstring NO_REPEAT_RANDOM_STR{"NO_REPEAT_RANDOM"};
    PCUI::ToggleData femaleTalkie;
    constexpr static cstring FEMALE_TALKIE_STR{"FEMALE_TALKIE_VOICE"};
    PCUI::ToggleData disableTalkie;
    constexpr static cstring DISABLE_TALKIE_STR{"DISABLE_TALKIE"};
    PCUI::ToggleData killOldPlayers;
    constexpr static cstring KILL_OLD_PLAYERS_STR{"KILL_OLD_PLAYERS"};

    // POV Data?

    PCUI::NotifierData customOptsNotifier;
    struct CustomOption {
        PCUI::TextData define;
        PCUI::TextData value;
    };
    const list<CustomOption>& customOptions() const { return mCustomOptions; }
    /**
     * Create a new custom option, if an empty one does not already exist.
     *
     * @return If a new option was created. (Empty didn't exist)
     */
    bool addCustomOption();

    /**
     * Remove a custom option by reference.
     *
     * @return If the option was found/removed
     */
    bool removeCustomOption(CustomOption&);

private:
    Config& mParent;
    list<CustomOption> mCustomOptions;
};

} // namespace Config

