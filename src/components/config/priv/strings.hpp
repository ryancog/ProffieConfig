#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/strings.hpp
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

#include <array>
#include <string_view>

#include "utils/types.hpp"

namespace config::priv {

enum class Board {
    Proffie_V3,
    Proffie_V2,
    Proffie_V1,
    Max,
};
constexpr std::array<cstring, static_cast<size>(Board::Max)> BOARD_STRS{
    "\"proffieboard_v3_config.h\"",
    "\"proffieboard_v2_config.h\"",
    "\"proffieboard_v1_config.h\"",
};

enum class ButtonType {
    Pullup,
    Pulldown,
    Latching,
    Latching_Inverted,
    Touch,

    Max,
};
constexpr std::array<cstring, static_cast<size>(ButtonType::Max)>
BUTTON_TYPE_STRS{
    "Button",
    "PullDownButton",
    "LatchingButton",
    "InvertedLatchingButton",
    "TouchButton"
};

enum class ButtonEvent {
    Power,
    Aux,
    Aux2,
    Up,
    Down,
    Left,
    Right,
    Select,
    Max
};
constexpr std::array<cstring, static_cast<size>(ButtonEvent::Max)>
BUTTON_EVENT_STRS{
    "BUTTON_POWER",
    "BUTTON_AUX",
    "BUTTON_AUX2",
    "BUTTON_UP",
    "BUTTON_DOWN",
    "BUTTON_LEFT",
    "BUTTON_RIGHT",
    "BUTTON_SELECT",
};

enum class BladeIDMode {
    Snapshot,
    External,
    Bridged,
    Max
};
constexpr std::array<std::string_view, static_cast<size>(BladeIDMode::Max)>
BLADEID_MODE_STRS{
    "SnapshotBladeID<",
    "ExternalPullupBladeID<",
    "BridgedPullupBladeID<",
};

enum class Orientation {
    Fets_Towards_Blade,
    USB_Towards_Blade,
    USB_CCW_From_Blade,
    USB_CW_From_Blade,
    Top_Towards_Blade,
    Bottom_Towards_Blade,
    Max,

    Normal = Fets_Towards_Blade,
};
constexpr std::array<cstring, static_cast<size>(Orientation::Max)>
ORIENTATION_STRS{
    "ORIENTATION_FETS_TOWARDS_BLADE",
    "ORIENTATION_USB_TOWARDS_BLADE",
    "ORIENTATION_USB_CCW_FROM_BLADE",
    "ORIENTATION_USB_CW_FROM_BLADE",
    "ORIENTATION_TOP_TOWARDS_BLADE",
    "ORIENTATION_BOTTOM_TOWARDS_BLADE",
};

enum class LED {
    None,

    Cree_Red,
    Cree_Green,
    Cree_Blue,
    Cree_Amber,
    Cree_Red_Orange,
    Cree_White,

    Red,
    Green,
    Blue,

    Max,

    Use_Resistance_Start = Cree_Red,
    Use_Resistance_End = Cree_White,
};
constexpr std::array<std::string_view, static_cast<size>(LED::Max)> LED_STRS{
    "NoLED",
    "CreeXPE2RedTemplate",
    "CreeXPE2GreenTemplate",
    "CreeXPE2BlueTemplate",
    "CreeXPE2AmberTemplate",
    "CreeXPE2RedOrangeTemplate",
    "CreeXPE2WhiteTemplate",
    "CH1LED",
    "CH2LED",
    "CH3LED",
};

enum class ColorOrder3 {
    GRB,
    GBR,
    BGR,
    BRG,
    RGB,
    RBG,
    Max,
};
constexpr std::array<cstring, static_cast<size>(ColorOrder3::Max)> ORDER_STRS{
    "GRB",
    "GBR",
    "BGR",
    "BRG",
    "RGB",
    "RBG",
};

enum ColorOrder4 {
    GRBW,
    GBRW,
    BGRW,
    BRGW,
    RGBW,
    RBGW,

    WGRB,
    WGBR,
    WBGR,
    WBRG,
    WRGB,
    WRBG,

    ORDER4_WFIRST_START = WGRB,
    ORDER4_WFIRST_END = WRBG,
};

constexpr cstring OS_VERSION_STR{"OS_VERSION"};
constexpr cstring ENABLE_MASS_STORAGE_STR{"ENABLE_MASS_STORAGE"};
constexpr cstring ENABLE_WEBUSB_STR{"ENABLE_WEBUSB"};
constexpr cstring NUM_BUTTONS_STR{"NUM_BUTTONS"};

constexpr cstring NUM_BLADES_STR{"NUM_BLADES"};
constexpr cstring ENABLE_AUDIO_STR{"ENABLE_AUDIO"};
constexpr cstring ENABLE_MOTION_STR{"ENABLE_MOTION"};
constexpr cstring ENABLE_WS2811_STR{"ENABLE_WS2811"};
constexpr cstring ENABLE_SD_STR{"ENABLE_SD"};
constexpr cstring SHARED_POWER_PINS_STR{"SHARED_POWER_PINS"};
constexpr cstring KEEP_SAVEFILES_STR{"KEEP_SAVEFILES_WHEN_PROGRAMMING"};

constexpr cstring RFID_SERIAL_STR{"RFID_SERIAL"};

constexpr cstring BLADE_DETECT_PIN_STR{"BLADE_DETECT_PIN"};

constexpr cstring BLADE_ID_CLASS_STR{"BLADE_ID_CLASS"};
constexpr cstring ENABLE_POWER_FOR_ID_STR{"ENABLE_POWER_FOR_ID"};
constexpr cstring BLADE_ID_SCAN_MILLIS_STR{"BLADE_ID_SCAN_MILLIS"};
constexpr cstring BLADE_ID_TIMES_STR{"BLADE_ID_TIMES"};

constexpr cstring VOLUME_STR{"VOLUME"};
constexpr cstring BOOT_VOLUME_STR{"BOOT_VOLUME"};
constexpr cstring AUDIO_CLASH_SUPPRESSION_STR{"AUDIO_CLASH_SUPPRESSION_LEVEL"};
constexpr cstring DONT_USE_GYRO_FOR_CLASH_STR{"PROFFIEOS_DONT_USE_GYRO_FOR_CLASH"};
constexpr cstring FILTER_CUTOFF_STR{"FILTER_CUTOFF_FREQUENCY"};
constexpr cstring FILTER_ORDER_STR{"FILTER_ORDER"};

constexpr cstring CLASH_THRESHOLD_STR{"CLASH_THRESHOLD_G"};
constexpr cstring PLI_OFF_STR{"PLI_OFF_TIME"};
constexpr cstring IDLE_OFF_STR{"IDLE_OFF_TIME"};
constexpr cstring MOTION_TIMEOUT_STR{"MOTION_TIMEOUT"};
constexpr cstring DISABLE_COLOR_CHANGE_STR{"DISABLE_COLOR_CHANGE"};
constexpr cstring DISABLE_BASIC_PARSERS_STR{"DISABLE_BASIC_PARSER_STYLES"};
constexpr cstring DISABLE_TALKIE_STR{"DISABLE_TALKIE"};
constexpr cstring DISABLE_DIAG_COMMANDS_STR{"DISABLE_DIAGNOSTIC_COMMANDS"};
// constexpr cstring ENABLE_DEV_COMMANDS_STR{"ENABLE_DEVELOPER_COMMANDS"};

constexpr cstring SAVE_STATE_STR{"SAVE_STATE"};
constexpr cstring ENABLE_ALL_EDIT_OPTIONS_STR{"ENABLE_ALL_EDIT_OPTIONS"};
constexpr cstring SAVE_VOLUME_STR{"SAVE_VOLUME"};
constexpr cstring SAVE_PRESET_STR{"SAVE_PRESET"};
constexpr cstring SAVE_COLOR_STR{"SAVE_COLOR_CHANGE"};

constexpr cstring ENABLE_OLED_STR{"ENABLE_SSD1306"};

constexpr cstring NO_REPEAT_RANDOM_STR{"NO_REPEAT_RANDOM"};
constexpr cstring FEMALE_TALKIE_STR{"FEMALE_TALKIE_VOICE"};
constexpr cstring KILL_OLD_PLAYERS_STR{"KILL_OLD_PLAYERS"};

constexpr cstring ORIENTATION_STR{"ORIENTATION"};
constexpr cstring ORIENTATION_ROTATION_STR{"ORIENTATION_ROTATION"};

constexpr cstring DYNAMIC_BLADE_DIMMING_STR{"DYNAMIC_BLADE_DIMMING"};
constexpr cstring DYNAMIC_BLADE_LENGTH_STR{"DYNAMIC_BLADE_LENGTH"};
constexpr cstring DYNAMIC_CLASH_THRESHOLD_STR{"DYNAMIC_CLASH_THRESHOLD"};

constexpr cstring SAVE_BLADE_DIM_STR{"SAVE_BLADE_DIMMING"};
constexpr cstring SAVE_CLASH_THRESHOLD_STR{"SAVE_CLASH_THRESHOLD"};

} // namespace config::priv

