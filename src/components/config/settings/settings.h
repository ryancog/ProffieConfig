#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
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

#include "log/branch.h"
#include "ui/controls/checklist.h"
#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"
#include "ui/controls/numeric.h"
#include "ui/notifier.h"
#include "utils/types.h"
#include "utils/version.h"

#include "config_export.h"

namespace Config {

enum BoardVersion {
    PROFFIEBOARDV3,
    PROFFIEBOARDV2,
    PROFFIEBOARDV1,
    BOARD_MAX,
};
static constexpr array<cstring, BOARD_MAX> BOARD_STRS{
    "\"proffieboard_v3_config.h\"",
    "\"proffieboard_v2_config.h\"",
    "\"proffieboard_v1_config.h\"",
};

enum ButtonType {
    BUTTON,
    PULLDOWN_BUTTON,
    LATCHING_BUTTON,
    INVERTED_LATCHING_BUTTON,
    TOUCH_BUTTON,

    BUTTON_TYPE_MAX,
};
static constexpr array<cstring, BUTTON_TYPE_MAX> BUTTON_TYPE_STRS{
    "Button",
    "PullDownButton",
    "LatchingButton",
    "InvertedLatchingButton",
    "TouchButton"
};

enum ButtonEvent {
    POWER = 0,
    AUX,
    AUX2,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SELECT,
    BUTTON_MAX,
};
static constexpr array<cstring, BUTTON_MAX> BUTTON_EVENT_STRS{
    "BUTTON_POWER",
    "BUTTON_AUX",
    "BUTTON_AUX2",
    "BUTTON_UP",
    "BUTTON_DOWN",
    "BUTTON_LEFT",
    "BUTTON_RIGHT",
    "BUTTON_SELECT",
};

enum BladeIDMode {
    SNAPSHOT = 0,
    EXTERNAL,
    BRIDGED,
    BLADEID_MODE_MAX,
};
static constexpr array<string_view, BLADEID_MODE_MAX> BLADEID_MODE_STRS{
    "SnapshotBladeID<",
    "ExternalPullupBladeID<",
    "BridgedPullupBladeID<",
};

enum Orientation {
    FETS_TOWARDS_BLADE,
    USB_TOWARDS_BLADE,
    USB_CCW_FROM_BLADE,
    USB_CW_FROM_BLADE,
    TOP_TOWARDS_BLADE,
    BOTTOM_TOWARDS_BLADE,
    ORIENTATION_MAX,

    ORIENTATION_NORMAL = FETS_TOWARDS_BLADE,
};
static constexpr array<cstring, ORIENTATION_MAX> ORIENTATION_STRS{
    "ORIENTATION_FETS_TOWARDS_BLADE",
    "ORIENTATION_USB_TOWARDS_BLADE",
    "ORIENTATION_USB_CCW_FROM_BLADE",
    "ORIENTATION_USB_CW_FROM_BLADE",
    "ORIENTATION_TOP_TOWARDS_BLADE",
    "ORIENTATION_BOTTOM_TOWARDS_BLADE",
};

constexpr static cstring OS_VERSION_STR{"OS_VERSION"};
constexpr static cstring ENABLE_MASS_STORAGE_STR{"ENABLE_MASS_STORAGE"};
constexpr static cstring ENABLE_WEBUSB_STR{"ENABLE_WEBUSB"};
constexpr static cstring NUM_BUTTONS_STR{"NUM_BUTTONS"};

constexpr static cstring NUM_BLADES_STR{"NUM_BLADES"};
constexpr static cstring ENABLE_AUDIO_STR{"ENABLE_AUDIO"};
constexpr static cstring ENABLE_MOTION_STR{"ENABLE_MOTION"};
constexpr static cstring ENABLE_WS2811_STR{"ENABLE_WS2811"};
constexpr static cstring ENABLE_SD_STR{"ENABLE_SD"};
constexpr static cstring SHARED_POWER_PINS_STR{"SHARED_POWER_PINS"};
constexpr static cstring KEEP_SAVEFILES_STR{"KEEP_SAVEFILES_WHEN_PROGRAMMING"};

constexpr static cstring RFID_SERIAL_STR{"RFID_SERIAL"};

constexpr static cstring BLADE_DETECT_PIN_STR{"BLADE_DETECT_PIN"};

constexpr static cstring BLADE_ID_CLASS_STR{"BLADE_ID_CLASS"};
constexpr static cstring ENABLE_POWER_FOR_ID_STR{"ENABLE_POWER_FOR_ID"};
constexpr static cstring BLADE_ID_SCAN_MILLIS_STR{"BLADE_ID_SCAN_MILLIS"};
constexpr static cstring BLADE_ID_TIMES_STR{"BLADE_ID_TIMES"};

constexpr static cstring VOLUME_STR{"VOLUME"};
constexpr static cstring BOOT_VOLUME_STR{"BOOT_VOLUME"};
constexpr static cstring AUDIO_CLASH_SUPPRESSION_STR{"AUDIO_CLASH_SUPPRESSION_LEVEL"};
constexpr static cstring DONT_USE_GYRO_FOR_CLASH_STR{"PROFFIEOS_DONT_USE_GYRO_FOR_CLASH"};
constexpr static cstring FILTER_CUTOFF_STR{"FILTER_CUTOFF_FREQUENCY"};
constexpr static cstring FILTER_ORDER_STR{"FILTER_ORDER"};

constexpr static cstring CLASH_THRESHOLD_STR{"CLASH_THRESHOLD_G"};
constexpr static cstring PLI_OFF_STR{"PLI_OFF_TIME"};
constexpr static cstring IDLE_OFF_STR{"IDLE_OFF_TIME"};
constexpr static cstring MOTION_TIMEOUT_STR{"MOTION_TIMEOUT"};
constexpr static cstring DISABLE_COLOR_CHANGE_STR{"DISABLE_COLOR_CHANGE"};
constexpr static cstring DISABLE_BASIC_PARSERS_STR{"DISABLE_BASIC_PARSER_STYLES"};
constexpr static cstring DISABLE_TALKIE_STR{"DISABLE_TALKIE"};
constexpr static cstring DISABLE_DIAG_COMMANDS_STR{"DISABLE_DIAGNOSTIC_COMMANDS"};

constexpr static cstring SAVE_STATE_STR{"SAVE_STATE"};
constexpr static cstring ENABLE_ALL_EDIT_OPTIONS_STR{"ENABLE_ALL_EDIT_OPTIONS"};
constexpr static cstring SAVE_VOLUME_STR{"SAVE_VOLUME"};
constexpr static cstring SAVE_PRESET_STR{"SAVE_PRESET"};
constexpr static cstring SAVE_COLOR_STR{"SAVE_COLOR_CHANGE"};

constexpr static cstring ENABLE_OLED_STR{"ENABLE_SSD1306"};

constexpr static cstring NO_REPEAT_RANDOM_STR{"NO_REPEAT_RANDOM"};
constexpr static cstring FEMALE_TALKIE_STR{"FEMALE_TALKIE_VOICE"};
constexpr static cstring KILL_OLD_PLAYERS_STR{"KILL_OLD_PLAYERS"};

constexpr static cstring ORIENTATION_STR{"ORIENTATION"};
constexpr static cstring ORIENTATION_ROTATION_STR{"ORIENTATION_ROTATION"};

constexpr static cstring DYNAMIC_BLADE_DIMMING_STR{"DYNAMIC_BLADE_DIMMING"};
constexpr static cstring DYNAMIC_BLADE_LENGTH_STR{"DYNAMIC_BLADE_LENGTH"};
constexpr static cstring DYNAMIC_CLASH_THRESHOLD_STR{"DYNAMIC_CLASH_THRESHOLD"};

constexpr static cstring SAVE_BLADE_DIM_STR{"SAVE_BLADE_DIMMING"};
constexpr static cstring SAVE_CLASH_THRESHOLD_STR{"SAVE_CLASH_THRESHOLD"};

struct Config;

struct CONFIG_EXPORT Settings {
    /**
     * Set up defaults and update handlers
     */
    Settings(Config&);

    pcui::Notifier notifyData;

    pcui::ChoiceData board;

    // Do NOT set choices manually
    // Done via Config::refreshVersions
    pcui::ChoiceData osVersion;
    vector<Utils::Version> osVersionMap;

    [[nodiscard]] Utils::Version getOSVersion() const;

    pcui::ToggleData massStorage;
    pcui::ToggleData webUSB;

    struct CONFIG_EXPORT ButtonData {
        ButtonData();

        pcui::ChoiceData type;
        pcui::ChoiceData event;

        pcui::ComboBoxData pin;
        pcui::TextData name;
        pcui::NumericData touch;
    };

    // TODO: For both this and custom opts, this really should be wrapped in 
    // some nicer abstraction, both for races and optimization (e.g. fine-
    // grained add/remove/clear events)
    pcui::Notifier buttonNotifier;
    void addButton(std::unique_ptr<ButtonData>&& = nullptr);
    void removeButton(size);
    void removeButton(const ButtonData&);
    [[nodiscard]] const auto& buttons() const { return mButtons; }
    [[nodiscard]] size numButtons() const { return mButtons.size(); }
    [[nodiscard]] auto& button(size idx) { return mButtons[idx]; }
    [[nodiscard]] const auto& button(size idx) const { return mButtons[idx]; }

    // pcui::ChoiceData rfidSerial;

    pcui::ToggleData bladeDetect;
    pcui::ComboBoxData bladeDetectPin;

    struct BladeID {
        pcui::ToggleData enable;
        pcui::ComboBoxData pin;
        pcui::ChoiceData mode;
        pcui::ComboBoxData bridgePin;
        pcui::NumericData pullup;
        pcui::ToggleData powerForID;
        pcui::CheckListData powerPins;
        pcui::TextData powerPinEntry;
        pcui::ToggleData continuousScanning;
        pcui::NumericData continuousInterval;
        pcui::NumericData continuousTimes;

        void addPowerPinFromEntry();
    } bladeID;

    pcui::NumericData volume;
    pcui::ToggleData enableBootVolume;
    pcui::NumericData bootVolume;

    // Does not affect I2C or S/PDIF
    // Cutoff in Hz
    pcui::ToggleData enableFiltering;
    pcui::NumericData filterCutoff;
    pcui::NumericData filterOrder;

    pcui::DecimalData clashThreshold;

    // In seconds 
    pcui::DecimalData pliOffTime;
    // In Minutes
    pcui::DecimalData idleOffTime;
    pcui::DecimalData motionTimeout;

    pcui::ToggleData disableColorChange;
    pcui::ToggleData disableBasicParserStyles;
    pcui::ToggleData disableTalkie;
    pcui::ToggleData disableDiagnosticCommands;
    // pcui::ToggleData enableDeveloperCommands;
    // constexpr static cstring ENABLE_DEV_COMMANDS_STR{"ENABLE_DEVELOPER_COMMANDS"};

    // SAVE_STATE Sets:
    // - saveVolume
    // - savePreset
    // - saveColorChange
    // - saveBladeDimming (only if dynamicBladeDimming is set)
    pcui::ToggleData saveState;

    // ENABLE_ALL_EDIT_OPTIONS Sets:
    // - dynamicBladeLength
    // - dynamicBladeDimming
    // - dynamicClashThreshold
    // - saveVolume
    // - saveColorChange
    // - saveBladeDimming
    // - saveClashThreshold
    pcui::ToggleData enableAllEditOptions;

    pcui::ToggleData saveVolume;
    pcui::ToggleData savePreset;
    pcui::ToggleData saveColorChange;

    pcui::ToggleData enableOLED;

    pcui::ChoiceData orientation;

    struct OrientRotation {
        pcui::NumericData x;
        pcui::NumericData y;
        pcui::NumericData z;
    } orientationRotation;

    // For debugging touch buttons:
    // pcui::ToggleData speakTouchValues;
    // constexpr static cstring SPEAK_TOUCH_VALUES_STR{"SPEAK_TOUCH_VALUES"};
    
    // Make these be enabled by adding such a device to the
    // wiring. Adjust volume there too? That I don't know...
    /*
    bool enableI2SOut{false};
    bool enableSPDIFOut{false};
    int32 lineOutVolume{2000}; // This can be set to `dynamic_mixer.get_volume()` to follow master
    */

    pcui::ToggleData dynamicBladeDimming;
    pcui::ToggleData dynamicBladeLength;
    pcui::ToggleData dynamicClashThreshold;

    // only should be settable if dynamicBladeDimming is true
    pcui::ToggleData saveBladeDimming;
    pcui::ToggleData saveClashThreshold;

    // Useful range is 1~50
    pcui::NumericData audioClashSuppressionLevel;
    pcui::ToggleData dontUseGyroForClash;

    pcui::ToggleData noRepeatRandom;
    pcui::ToggleData femaleTalkie;
    pcui::ToggleData killOldPlayers;

    // POV Data?

    pcui::Notifier customOptsNotifyData;
    struct CustomOption {
        pcui::TextData define;
        pcui::TextData value;
    };
    [[nodiscard]] const vector<std::unique_ptr<CustomOption>>& customOptions() const { return mCustomOptions; }
    [[nodiscard]] CustomOption& customOption(uint32 idx) const {
        assert(idx < mCustomOptions.size());
        return *mCustomOptions[idx];
    }

    /**
     * Create a new custom option
     *
     * New option is not created if provided key is empty and empty already exists.
     *
     * @return If a new option was created.
     */
    bool addCustomOption(string&& key = {}, string&& value = {});

    /**
     * Remove a custom option by reference.
     *
     * @return If the option was found/removed
     */
    bool removeCustomOption(CustomOption&);

    /**
     * Remove a custom option by idx
     *
     * @return If the option was found/removed
     */
    bool removeCustomOption(uint32);

    /**
     * Make sure all the "custom" defines really are custom.
     *
     * Move them where they belong if not.
     */
    void processCustomDefines(Log::Branch * = nullptr);

private:
    Config& mParent;
    vector<std::unique_ptr<CustomOption>> mCustomOptions;
    vector<std::unique_ptr<ButtonData>> mButtons;
};

} // namespace Config

