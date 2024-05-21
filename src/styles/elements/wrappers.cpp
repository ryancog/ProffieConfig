#include "wrappers.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/wrappers.cpp
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
#include "styles/elements/colorstyles.h"

using namespace BladeStyles;

WrapperStyle::WrapperStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent) :
    ColorStyle(osName, humanName, params, parent, WRAPPER) {}


StyleGenerator WrapperStyle::get(const std::string& styleName) {
    const auto& map{getMap()};
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& WrapperStyle::getMap() { return map; }

#define RUN(varname) virtual void run(StylePreview::Blade& varname) override
#define GETCOLOR(varname) virtual ColorData getColor(int32_t varname) const override

#define WRAPPER(osName, humanName, params, ...) \
    class osName : public WrapperStyle { \
    public: \
        osName(const BladeStyle* parent) : WrapperStyle(#osName, humanName, params, parent) {} \
        __VA_ARGS__ \
    }; 

// NEED A BETTER HUMAN NAME for all these
WRAPPER(StylePtr, "StylePtr",
        PARAMS(
            new StyleParam("Style Contents", COLOR, nullptr)
            ),
        RUN(blade) {
            style = STYLECAST(ColorStyle, getParamStyle(0));
            style->run(blade);
        }
        GETCOLOR(led) { return style->getColor(led); }

        private:
            ColorStyle* style;
       )

WRAPPER(ChargingStylePtr, "ChargingStylePtr",
        PARAMS(
            new StyleParam("Style Contents", COLOR, nullptr)
            ),
        RUN(blade) {
            style = STYLECAST(ColorStyle, getParamStyle(0));
            style->run(blade);
        }
        GETCOLOR(led) { return style->getColor(led); }

        private:
            ColorStyle* style;
       )

// WRAPPER(StyleNormalPtr, "StyleNormalPtr",
//         PARAMS(
//         Input("Base Color", COLORF),
//         Input("Clash Color", COLORF),
//         Input("Out Time (ms)", INT),
//         Input("In Time (ms)", INT),
//         Input("Lockup Color", COLORF, Color::WHITE),
//         Input("Blast Color", COLORF, Color::WHITE)
//         ),
// 
//        )
// 
// WRAPPER("NormalPtrX", "StyleNormalPtrX",
//         Input("Base Color", COLORF),
//         Input("Clash Color", COLORF),
//         Input("Out Time (ms)", INT | FUNCTION),
//         Input("In Time (ms)", INT | FUNCTION),
//         Input("Lockup Color", COLORF, Color::WHITE),
//         Input("Blast Color", COLORF, Color::WHITE)
//        ),
// 
// WRAPPER("RainbowPtr", "StyleRainbowPtr",
//         Input("Out Time (ms)", INT),
//         Input("In Time (ms)", INT),
//         Input("Clash Color", COLORF, Color::WHITE),
//         Input("Lockup Color", COLORF, Color::WHITE)
//        ),
// 
// WRAPPER("RainbowPtrX", "StyleRainbowPtrX",
//         Input("Out Time (ms)", INT | FUNCTION),
//         Input("In Time (ms)", INT | FUNCTION),
//         Input("Clash Color", COLORF, Color::WHITE),
//         Input("Lockup Color", COLORF, Color::WHITE)
//        ),
// 
// WRAPPER("StrobePtr", "StyleStrobePtr",
//         Input("Strobe Color", COLORF),
//         Input("Clash Color", COLORF),
//         Input("Frequency", INT),
//         Input("Out Time (ms)", INT),
//         Input("In Time (ms)", INT)
//        ),

const StyleMap WrapperStyle::map {
    STYLEPAIR(StylePtr),
    STYLEPAIR(ChargingStylePtr)
};
