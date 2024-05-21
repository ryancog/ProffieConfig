#include "effects.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/effects.cpp
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

using namespace BladeStyles;

EffectStyle::EffectStyle(const char* osName, const char* humanName, const Effect effect, const BladeStyle* parent) :
    BladeStyle(osName, humanName, EFFECT, {}, parent), effect(effect) {}

StyleGenerator EffectStyle::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& EffectStyle::getMap() { return map; }

const StyleMap EffectStyle::map {
#   define EMAP(enum, humanName) \
    { \
        #enum, \
        [](const BladeStyle* parent, const std::vector<ParamValue>& params) -> BladeStyle* { \
            if (params.size()) return nullptr; \
            return new EffectStyle(#enum, humanName, Effect::enum, parent); \
        } \
    },

    ALL_EFFECTS

#   undef EMAP
};

