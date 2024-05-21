#include "builtin.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/builtin.cpp
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

using namespace BladeStyles;

BuiltIn::BuiltIn(const char* osName, const char* humanName) :
    ColorStyle(osName, humanName, {}, nullptr, BUILTIN) {}

const StyleMap& BuiltIn::getMap() { return map; }
StyleGenerator BuiltIn::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

#define RUN(varname) virtual void run(StylePreview::Blade& varname) override
#define GETCOLOR(varname) virtual ColorData getColor(int32_t varname) const override

#define BUILTIN(osName, humanName, ...) \
    class osName : public BuiltIn { \
    public: \
        osName() : BuiltIn(#osName, humanName) {} \
        __VA_ARGS__ \
    }; 

BUILTIN(style_charging, "Charging Style",
        RUN(blade) {

        }
        GETCOLOR(led) {

        }
        )

BUILTIN(style_pov, "POV Style",
        RUN(blade) {

        } GETCOLOR(led){

        }
       )

// Not sure if this is a good idea...
#define BUILTINPAIR(style) { \
    #style, \
      [](const BladeStyle*, const std::vector<ParamValue>& paramArgs) -> BladeStyle* { \
        auto ret{new style()}; \
        if (!ret->setParams(paramArgs)) \
          return nullptr; \
        return ret; \
      } \
}

const StyleMap BuiltIn::map{
    BUILTINPAIR(style_charging),
    BUILTINPAIR(style_pov)
};
