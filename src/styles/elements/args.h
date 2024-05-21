#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/args.h
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

#include "styles/bladestyle.h"

namespace BladeStyles {

#define ALL_ARGUMENTS \
    AMAP(BASE_COLOR,            "BASE_COLOR",           "Base Color") \
    AMAP(ALT_COLOR,             "ALT_COLOR",            "Alt Color") \
    AMAP(STYLE_OPTION,          "STYLE_OPTION",         "Style Option") \
    AMAP(IGNITION_OPTION,       "IGNITION_OPTION",      "Ignition Option") \
    AMAP(IGNITION_TIME,         "IGNITION_TIME",        "Ignition Time") \
    AMAP(IGNITION_DELAY,        "IGNITION_DELAY",       "Ignition Delay") \
    AMAP(IGNITION_COLOR,        "IGNITION_COLOR",       "Ignition Color") \
    AMAP(IGNITION_POWER_UP,     "IGNITION_POWER_UP",    "Ignition Power Up") \
    AMAP(BLAST_COLOR,           "BLAST_COLOR",          "Blast Color") \
    AMAP(CLASH_COLOR,           "CLASH_COLOR",          "Clash Color") \
    AMAP(LOCKUP_COLOR,          "LOCKUP_COLOR",         "Lockup Color") \
    AMAP(LOCKUP_POSITION,       "LOCKUP_POSITION",      "Lockup Position") \
    AMAP(DRAG_COLOR,            "DRAG_COLOR",           "Drag Color") \
    AMAP(DRAG_SIZE,             "DRAG_SIZE",            "Drag Size") \
    AMAP(LB_COLOR,              "LB_COLOR",             "Lightning Block Color") \
    AMAP(STAB_COLOR,            "STAB_COLOR",           "Stab Color") \
    AMAP(MELT_SIZE,             "MELT_SIZE",            "Melt Size") \
    AMAP(SWING_COLOR,           "SWING_COLOR",          "Swing Color") \
    AMAP(SWING_OPTION,          "SWING_OPTION",         "Swing Option") \
    AMAP(EMITTER_COLOR,         "EMITTER_COLOR",        "Emitter Color") \
    AMAP(EMITTER_SIZE,          "EMITTER_SIZE",         "Emitter Size") \
    AMAP(PREON_COLOR,           "PREON_COLOR",          "Preon Color") \
    AMAP(PREON_OPTION,          "PREON_OPTION",         "Preon Option") \
    AMAP(PREON_SIZE,            "PREON_SIZE",           "Preon Size") \
    AMAP(RETRACTION_OPTION,     "RETRACTION_OPTION",    "Retraction Option") \
    AMAP(RETRACTION_TIME,       "RETRACTION_TIME",      "Retraction Time") \
    AMAP(RETRACTION_DELAY,      "RETRACTION_DELAY",     "Retraction Delay") \
    AMAP(RETRACTION_COLOR,      "RETRACTION_COLOR",     "Retraction Color") \
    AMAP(RETRACTION_COOL_DOWN,  "RETRACTION_COOL_DOWN", "Retraction Cool Down") \
    AMAP(POSTOFF_COLOR,         "POSTOFF_COLOR",        "Postoff Color") \
    AMAP(OFF_COLOR,             "OFF_COLOR",            "Off Color") \
    AMAP(OFF_OPTION,            "OFF_OPTION",           "Off Option") \
    AMAP(ALT_COLOR2,            "ALT_COLOR2",           "Alt Color 2") \
    AMAP(ALT_COLOR3,            "ALT_COLOR3",           "Alt Color 3") \
    AMAP(STYLE_OPTION2,         "STYLE_OPTION2",        "Style Option 2") \
    AMAP(STYLE_OPTION3,         "STYLE_OPTION3",        "Style Option 3") \
    AMAP(IGNITION_OPTION2,      "IGNITION_OPTION2",     "Ignition Option 2") \
    AMAP(RETRACTION_OPTION2,    "RETRACTION_OPTION2",   "Retraction Option 2") \

enum class Argument {
#define AMAP(enum, str, humanStr) enum,

ALL_ARGUMENTS

#undef AMAP
};

class ArgumentStyle : public BladeStyle {
public:
    const Argument arg;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    ArgumentStyle(const char* osName, const char* humanName, const Argument arg, const BladeStyle* parent);

private:
    static const StyleMap map;
};

}
