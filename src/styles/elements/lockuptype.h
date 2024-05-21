#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/lockuptype.h
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

#include <string>

#include "styles/bladestyle.h"

namespace BladeStyles {

#define LOCKUP_TYPES \
    LTMAP(NONE,              "NONE",              "NONE") \
    LTMAP(NORMAL,            "NORMAL",            "NORMAL") \
    LTMAP(DRAG,              "DRAG",              "DRAG") \
    LTMAP(ARMED,             "ARMED",             "ARMED") \
    LTMAP(AUTOFIRE,          "AUTOFIRE",          "AUTOFIRE") \
    LTMAP(MELT,              "MELT",              "MELT") \
    LTMAP(LIGHTNING_BLOCK,   "LIGHTNING_BLOCK",   "LIGHTNING_BLOCK") \

enum class LockupType {
#	define LTMAP(enum, str, humanStr) enum,

    LOCKUP_TYPES

#	undef LTMAP
};

class LockupTypeStyle : public BladeStyle {
public:
    const LockupType lockupType;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    LockupTypeStyle(const char* osName, const char* humanName, const LockupType lockType, const BladeStyle* parent);

private:
    static const StyleMap map;
};

}
