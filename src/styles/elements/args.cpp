#include "args.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/args.cpp
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

#include <cstdio>

using namespace BladeStyles;

ArgumentStyle::ArgumentStyle(const char* osName, const char* humanName, const Argument arg, const BladeStyle* parent) :
    BladeStyle(osName, humanName, ARGUMENT, {}, parent), arg(arg) {}

StyleGenerator ArgumentStyle::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& ArgumentStyle::getMap() { return map; }

const StyleMap ArgumentStyle::map {
#define AMAP(enum, str, humanStr) \
    { \
        str, \
        [](const BladeStyle* parent, const std::vector<ParamValue>& params) -> BladeStyle* { \
            if (params.size()) return nullptr; \
            return new ArgumentStyle(str, humanStr, Argument::enum, parent); \
        } \
    },

    ALL_ARGUMENTS

};



