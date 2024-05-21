#include "colors.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/colors.cpp
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

// Yes, I know primary colors are something else...
// fight me.
#define PRIMARY_COLORS \
    CMAP("RED", VAL(255,0,0)) \
    CMAP("GREEN", VAL(0,255,0)) \
    CMAP("BLUE", VAL(0,0,255)) \
    CMAP("YELLOW", VAL(255,255,0)) \
    CMAP("CYAN", VAL(0,255,255)) \
    CMAP("MAGENTA", VAL(255,0,255)) \
    CMAP("WHITE", VAL(255,255,255)) \
    CMAP("BLACK", VAL(0,0,0)) \

#define OS7_COLORS \
    CMAP("AliceBlue", VAL(223, 239, 255)) \
    CMAP("Aqua", VAL(0, 255, 255)) \
    CMAP("Aquamarine", VAL(55, 255, 169)) \
    CMAP("Azure", VAL(223, 255, 255)) \
    CMAP("Bisque", VAL(255, 199, 142)) \
    CMAP("Black", VAL(0, 0, 0)) \
    CMAP("BlanchedAlmond", VAL(255, 213, 157)) \
    CMAP("Blue", VAL(0, 0, 255)) \
    CMAP("Chartreuse", VAL(55, 255, 0)) \
    CMAP("Coral", VAL(255, 55, 19)) \
    CMAP("Cornsilk", VAL(255, 239, 184)) \
    CMAP("Cyan", VAL(0, 255, 255)) \
    CMAP("DarkOrange", VAL(255, 68, 0)) \
    CMAP("DeepPink", VAL(255, 0, 75)) \
    CMAP("DeepSkyBlue", VAL(0, 135, 255)) \
    CMAP("DodgerBlue", VAL(2, 72, 255)) \
    CMAP("FloralWhite", VAL(255, 244, 223)) \
    CMAP("Fuchsia", VAL(255, 0, 255)) \
    CMAP("GhostWhite", VAL(239, 239, 255)) \
    CMAP("Green", VAL(0, 255, 0)) \
    CMAP("GreenYellow", VAL(108, 255, 6)) \
    CMAP("HoneyDew", VAL(223, 255, 223)) \
    CMAP("HotPink", VAL(255, 36, 118)) \
    CMAP("Ivory", VAL(255, 255, 223)) \
    CMAP("LavenderBlush", VAL(255, 223, 233)) \
    CMAP("LemonChiffon", VAL(255, 244, 157)) \
    CMAP("LightCyan", VAL(191, 255, 255)) \
    CMAP("LightPink", VAL(255, 121, 138)) \
    CMAP("LightSalmon", VAL(255, 91, 50)) \
    CMAP("LightYellow", VAL(255, 255, 191)) \
    CMAP("Lime", VAL(0, 255, 0)) \
    CMAP("Magenta", VAL(255, 0, 255)) \
    CMAP("MintCream", VAL(233, 255, 244)) \
    CMAP("MistyRose", VAL(255, 199, 193)) \
    CMAP("Moccasin", VAL(255, 199, 119)) \
    CMAP("NavajoWhite", VAL(255, 187, 108)) \
    CMAP("Orange", VAL(255, 97, 0)) \
    CMAP("OrangeRed", VAL(255, 14, 0)) \
    CMAP("PapayaWhip", VAL(255, 221, 171)) \
    CMAP("PeachPuff", VAL(255, 180, 125)) \
    CMAP("Pink", VAL(255, 136, 154)) \
    CMAP("Red", VAL(255, 0, 0)) \
    CMAP("SeaShell", VAL(255, 233, 219)) \
    CMAP("Snow", VAL(255, 244, 244)) \
    CMAP("SpringGreen", VAL(0, 255, 55)) \
    CMAP("SteelBlue", VAL(14, 57, 118)) \
    CMAP("Tomato", VAL(255, 31, 15)) \
    CMAP("White", VAL(255, 255, 255)) \
    CMAP("Yellow", VAL(255, 255, 0)) \

#define OS8_COLORS \
    CMAP("ElectricPurple", VAL(127, 0, 255)) \
    CMAP("ElectricViolet", VAL(71, 0, 255)) \
    CMAP("ElectricLime", VAL(156, 255, 0)) \
    CMAP("Amber", VAL(255, 135, 0)) \
    CMAP("CyberYellow", VAL(255, 168, 0)) \
    CMAP("CanaryYellow", VAL(255, 221, 0)) \
    CMAP("PaleGreen", VAL(28, 255, 28)) \
    CMAP("Flamingo", VAL(255, 80, 254)) \
    CMAP("VividViolet", VAL(90, 0, 255)) \
    CMAP("PsychedelicPurple", VAL(186, 0, 255)) \
    CMAP("HotMagenta", VAL(255, 0, 156)) \
    CMAP("BrutalPink", VAL(255, 0, 128)) \
    CMAP("NeonRose", VAL(255, 0, 55)) \
    CMAP("VividRaspberry", VAL(255, 0, 38)) \
    CMAP("HaltRed", VAL(255, 0, 19)) \
    CMAP("MoltenCore", VAL(255, 24, 0)) \
    CMAP("SafetyOrange", VAL(255, 33, 0)) \
    CMAP("OrangeJuice", VAL(255, 55, 0)) \
    CMAP("ImperialYellow", VAL(255, 115, 0)) \
    CMAP("SchoolBus", VAL(255, 176, 0)) \
    CMAP("SuperSaiyan", VAL(255, 186, 0)) \
    CMAP("Star", VAL(255, 201, 0)) \
    CMAP("Lemon", VAL(255, 237, 0)) \
    CMAP("ElectricBanana", VAL(246, 255, 0)) \
    CMAP("BusyBee", VAL(231, 255, 0)) \
    CMAP("ZeusBolt", VAL(219, 255, 0)) \
    CMAP("LimeZest", VAL(186, 255, 0)) \
    CMAP("Limoncello", VAL(135, 255, 0)) \
    CMAP("CathodeGreen", VAL(0, 255, 22)) \
    CMAP("MintyParadise", VAL(0, 255, 128)) \
    CMAP("PlungePool", VAL(0, 255, 156)) \
    CMAP("VibrantMint", VAL(0, 255, 201)) \
    CMAP("MasterSwordBlue", VAL(0, 255, 219)) \
    CMAP("BrainFreeze", VAL(0, 219, 255)) \
    CMAP("BlueRibbon", VAL(0, 33, 255)) \
    CMAP("RareBlue", VAL(0, 13, 255)) \
    CMAP("OverdueBlue", VAL(13, 0, 255)) \
    CMAP("ViolentViolet", VAL(55, 0, 255)) \

#define ALL_COLORS \
    PRIMARY_COLORS \
    OS7_COLORS \
    OS8_COLORS \

#define UNIQUE_COLORS \
    OS7_COLORS \
    OS8_COLORS \

FixedColorStyle::FixedColorStyle(const char* osName, const char* humanName, const ColorData& color, const BladeStyle* parent) :
    ColorStyle(osName, humanName, {}, parent), color(color) {}

void FixedColorStyle::run(StylePreview::Blade&) {}

ColorData FixedColorStyle::getColor(int32_t) const { return color; }

StyleGenerator FixedColorStyle::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& FixedColorStyle::getMap() { return map; }

const StyleMap FixedColorStyle::map {
// For now the humanName is just the osName
#define VAL(red, green, blue) ColorData{red, green, blue}
#define CMAP(name, value) \
    { \
        name, \
        [](const BladeStyle* parent, const std::vector<ParamValue>& params) -> BladeStyle* { \
            if (params.size()) return nullptr; \
            return new FixedColorStyle(name, name, value, parent); \
        } \
    },

    ALL_COLORS

};
