#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/effects.h
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

#include <cstring>
#include <string>

#include "styles/bladestyle.h"

namespace BladeStyles {

#define GENERAL_EFFECTS \
    EMAP(NONE,                   "None") \
    EMAP(CLASH,                  "Clash") \
    EMAP(CLASH_UPDATE,           "Clash Update") \
    EMAP(BLAST,                  "Blast") \
    EMAP(FORCE,                  "Force") \
    EMAP(STAB,                   "Stab") \
    EMAP(BOOT,                   "Boot") \
    EMAP(LOCKUP_BEGIN,           "Lockup Begin") \
    EMAP(LOCKUP_END,             "Lockup End") \
    EMAP(DRAG_BEGIN,             "Drag Begin") \
    EMAP(DRAG_END,               "Drag End") \
    EMAP(PREON,                  "Preon") \
    EMAP(POSTOFF,                "Postoff") \
    EMAP(IGNITION,               "Ignition") \
    EMAP(RETRACTION,             "Retraction") \
    EMAP(CHANGE,                 "Change") \
    EMAP(NEWFONT,                "New Font") \
    EMAP(LOW_BATTERY,            "Low Battery") \
    EMAP(POWERSAVE,              "Power Save") \
    EMAP(BATTERY_LEVEL,          "Battery Level") \
    EMAP(VOLUME_LEVEL,           "Volume Level") \
    EMAP(ON,                     "On") \
    EMAP(OFF,                    "Off") \
    EMAP(FAST_ON,                "Fast On") \
    EMAP(FAST_OFF,               "Fast Off") \
    EMAP(QUOTE,                  "Quote") \
    EMAP(SECONDARY_IGNITION,     "Secondary Ignition") \
    EMAP(SECONDARY_RETRACTION,   "Secondary Retraction") \
    EMAP(OFF_CLASH,              "Off Clash") \
    EMAP(NEXT_QUOTE,             "Next Quote") \
    EMAP(INTERACTIVE_PREON,      "Interactive Preon") \
    EMAP(INTERACTIVE_BLAST,      "Interactive Blast") \
    EMAP(TRACK,                  "Track") \
    EMAP(BEGIN_BATTLE_MODE,      "Begin Battle Mode") \
    EMAP(END_BATTLE_MODE,        "End Battle Mode") \
    EMAP(BEGIN_AUTO_BLAST,       "Begin Auto Blast") \
    EMAP(END_AUTO_BLAST,         "End Auto Blast") \
    EMAP(ALT_SOUND,              "Alt Sound") \
    EMAP(TRANSITION_SOUND,       "Transition Sound") \
    EMAP(SOUND_LOOP,             "Sound Loop") \

#define BLASTER_EFFECTS \
    EMAP(STUN,       "Stun") \
    EMAP(FIRE,       "Fire") \
    EMAP(CLIP_IN,    "Clip In") \
    EMAP(CLIP_OUT,   "Clip Out") \
    EMAP(RELOAD,     "Reload") \
    EMAP(MODE,       "Mode") \
    EMAP(RANGE,      "Range") \
    EMAP(EMPTY,      "Empty") \
    EMAP(FULL,       "Full") \
    EMAP(JAM,        "Jam") \
    EMAP(UNJAM,      "Unjam") \
    EMAP(PLI_ON,     "PLI On") \
    EMAP(PLI_OFF,    "PLI Off") \

#define MINIGAME_EFFECTS \
    EMAP(GAME_START,         "Game Start") \
    EMAP(GAME_ACTION1,       "Game Action1") \
    EMAP(GAME_ACTION2,       "Game Action2") \
    EMAP(GAME_CHOICE,        "Game Choice") \
    EMAP(GAME_RESPONSE1,     "Game Response1") \
    EMAP(GAME_RESPONSE2,     "Game Response2") \
    EMAP(GAME_RESULT1,       "Game Result1") \
    EMAP(GAME_RESULT2,       "Game Result2") \
    EMAP(GAME_WIN,           "Game Win") \
    EMAP(GAME_LOSE,          "Game Lose") \

#define USER_EFFECTS \
    EMAP(USER1,  "User 1") \
    EMAP(USER2,  "User 2") \
    EMAP(USER3,  "User 3") \
    EMAP(USER4,  "User 4") \
    EMAP(USER5,  "User 5") \
    EMAP(USER6,  "User 6") \
    EMAP(USER7,  "User 7") \
    EMAP(USER8,  "User 8") \

#define ERR_EFFECTS \
    EMAP(SD_CARD_NOT_FOUND,          "SD Card Not Found") \
    EMAP(ERROR_IN_FONT_DIRECTORY,    "Error In Font Directory") \
    EMAP(ERROR_IN_BLADE_ARRAY,       "Error In Blade Array") \
    EMAP(FONT_DIRECTORY_NOT_FOUND,   "Font Directory Not Found") \

#define ALL_EFFECTS \
    GENERAL_EFFECTS \
    BLASTER_EFFECTS \
    MINIGAME_EFFECTS \
    USER_EFFECTS \

enum class Effect {
#	define EMAP(enum, humanStr) enum,

    ALL_EFFECTS

#	undef EMAP
};

class EffectStyle : public BladeStyle {
public:
    const Effect effect;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    EffectStyle(const char* osName, const char* humanName, const Effect effect, const BladeStyle* parent);

private:
    static const StyleMap map;
};

}
