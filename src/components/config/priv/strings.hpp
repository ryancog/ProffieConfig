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

enum Board {
    eBoard_Proffie_V3,
    eBoard_Proffie_V2,
    eBoard_Proffie_V1,
    eBoard_Max,
};
constexpr std::array<cstring, eBoard_Max> BOARD_STRS{
    "\"proffieboard_v3_config.h\"",
    "\"proffieboard_v2_config.h\"",
    "\"proffieboard_v1_config.h\"",
};

enum ButtonType {
    eBtn_Type_Pullup,
    eBtn_Type_Pulldown,
    eBtn_Type_Latching,
    eBtn_Type_Latching_Inverted,
    eBtn_Type_Touch,

    eBtn_Type_Max,
};
constexpr std::array<cstring, eBtn_Type_Max>
BUTTON_TYPE_STRS{
    "Button",
    "PullDownButton",
    "LatchingButton",
    "InvertedLatchingButton",
    "TouchButton"
};

enum ButtonEvent {
    eBtn_Evt_Power,
    eBtn_Evt_Aux,
    eBtn_Evt_Aux2,
    eBtn_Evt_Up,
    eBtn_Evt_Down,
    eBtn_Evt_Left,
    eBtn_Evt_Right,
    eBtn_Evt_Select,
    eBtn_Evt_Max
};
constexpr std::array<cstring, eBtn_Evt_Max>
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

enum BladeIDMode {
    eBIDMode_Snapshot,
    eBIDMode_External,
    eBIDMode_Bridged,
    eBIDMode_Max
};
constexpr std::array<std::string_view, eBIDMode_Max>
BLADEID_MODE_STRS{
    "SnapshotBladeID<",
    "ExternalPullupBladeID<",
    "BridgedPullupBladeID<",
};

enum Orientation {
    eOrient_Fets_Towards_Blade,
    eOrient_USB_Towards_Blade,
    eOrient_USB_CCW_From_Blade,
    eOrient_USB_CW_From_Blade,
    eOrient_Top_Towards_Blade,
    eOrient_Bottom_Towards_Blade,
    eOrient_Max,

    eOrient_Normal = eOrient_Fets_Towards_Blade,
};
constexpr std::array<cstring, eOrient_Max>
ORIENTATION_STRS{
    "ORIENTATION_FETS_TOWARDS_BLADE",
    "ORIENTATION_USB_TOWARDS_BLADE",
    "ORIENTATION_USB_CCW_FROM_BLADE",
    "ORIENTATION_USB_CW_FROM_BLADE",
    "ORIENTATION_TOP_TOWARDS_BLADE",
    "ORIENTATION_BOTTOM_TOWARDS_BLADE",
};

enum LED {
    eLED_None,

    eLED_Cree_Red,
    eLED_Cree_Green,
    eLED_Cree_Blue,
    eLED_Cree_Amber,
    eLED_Cree_Red_Orange,
    eLED_Cree_White,

    eLED_Red,
    eLED_Green,
    eLED_Blue,

    eLED_Max,

    eLED_Use_Resistance_Start = eLED_Cree_Red,
    eLED_Use_Resistance_End = eLED_Cree_White,
};
constexpr std::array<std::string_view, eLED_Max> LED_STRS{
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

enum ColorOrder3 {
    eOrder3_GRB,
    eOrder3_GBR,
    eOrder3_BGR,
    eOrder3_BRG,
    eOrder3_RGB,
    eOrder3_RBG,
    eOrder3_Max,
};
constexpr std::array<cstring, eOrder3_Max> ORDER_STRS{
    "GRB",
    "GBR",
    "BGR",
    "BRG",
    "RGB",
    "RBG",
};

enum ColorOrder4 {
    eOrder4_GRBW,
    eOrder4_GBRW,
    eOrder4_BGRW,
    eOrder4_BRGW,
    eOrder4_RGBW,
    eOrder4_RBGW,

    eOrder4_WGRB,
    eOrder4_WGBR,
    eOrder4_WBGR,
    eOrder4_WBRG,
    eOrder4_WRGB,
    eOrder4_WRBG,

    eOrder4_Max,

    eOrder4_White_First_Start = eOrder4_WGRB,
    eOrder4_White_First_End = eOrder4_WRBG,
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

