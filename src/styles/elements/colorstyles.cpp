#include "colorstyles.h"
#include "styles/bladestyle.h"
#include <cmath>
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/colorstyles.cpp
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

ColorData BladeStyles::mixColors(const ColorData& a, const ColorData& b, int32_t x, int32_t shift) {
    return (a * ((1 << shift) - x) + (b * x)) >> shift;
}

bool ColorData::operator==(const ColorData& other) const {
    return (red == other.red) && (green == other.green) && (blue == other.blue);
}

ColorData ColorData::operator*(uint16_t multiplier) const {
    return ColorData{
        .red = red * multiplier,
        .green = green * multiplier,
        .blue = blue * multiplier
    };
}

ColorData ColorData::operator+(const ColorData& other) const {
    return ColorData{
        .red = red + other.red,
        .green = green + other.green,
        .blue = blue + other.blue
    };
}

ColorData ColorData::operator>>(int32_t shift) const {
    return ColorData{
        .red = red >> shift,
        .green = green >> shift,
        .blue = blue >> shift,
    };
}

const StyleMap& ColorStyle::getMap() { return map; }

StyleGenerator ColorStyle::get(const std::string& name) {
    const auto& mapIt{map.find(name)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

ColorStyle::ColorStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent, StyleType typeOverride) :
    BladeStyle(osName, humanName, typeOverride ? typeOverride : COLOR, params, parent) {}

const StyleMap ColorStyle::map{
 //        // Usage: AlphaL<COLOR, ALPHA>
 //        // COLOR: COLOR or LAYER
 //        // ALPHA: FUNCTION
 //        // Return value: LAYER
 //        // This function makes a color transparent. The ALPHA function specifies
 //        // just how opaque it should be.
 //        // If ALPHA is zero, the returned color will be 100% transparent. If Alpha
 //        // is 32768, the returned color will
 //        // be 100% opaque. Note that if COLOR is already transparent, it will be
 //        // made more transparent. Example:
 //        // If COLOR is 50% opaque, and ALPHA returns 16384, the result will be 25%
 //        // opaque.
 //        LAYER("AlphaL", "Transparency", 
 //                Input("Color", COLORF | FUNCTION),
 //                Input("Value", INT | FUNCTION)),

 //        // To enable Gradient/Mixes constricted within Bump<> and SmoothStep<>
 //        // layers
 //        // Example: AlphaMixL<Bump<Int<16384>,Int<16384>>,Red,Green,Blue> will
 //        // produce a gradient within the Bump
 //        //
 //        // I did some testing, this seems to mix the varadic colors based on the
 //        // input in the way as follows:
 //        // The lower the value, this will return a black, then the first color,
 //        // etc. As higher values are approached,
 //        // the color will fade back to black, from the last color.
 //        LAYER("AlphaMixL", "Mix With Transparency",
 //                Input("Input", INTF),
 //                Input("Color #", COLORF | VARIADIC)),

 //        // Usage: AudioFlicker<A, B>
 //        // Or: AudioFlickerL<B>
 //        // A, B: COLOR
 //        // return value: COLOR
 //        // Mixes between A and B based on audio. Quiet audio
 //        // means more A, loud audio means more B.
 //        // Based on a single sample instead of an average to make it flicker.
 //        LAYER("AudioFlickerL", "Audio Flicker",
 //                Input("Flicker Color", COLORF | LAYERF)),

 //        STYLE("AudioFlicker", "Audio Flicker", 
 //                Input("Base Color", COLORF),
 //                Input("Flicker Color", COLORF | LAYERF)),

 //        // NO DOCUMENTATION
 //        // BladeShortenerWrapper

 //        // Usage: Blast<BASE, BLAST, FADEOUT_MS, WAVE_SIZE, WAVE_MS>
 //        // Or: BlastL<BLAST, FADEOUT_MS, WAVE_SIZE, WAVE_MS>
 //        // BASE, BLAST: COLOR
 //        // FADEOUT_MS: a number (defaults to 150)
 //        // WAVE_SIZE: a number (defaults to 100)
 //        // WAVE_MS: a number (defaults to 400)
 //        // return value: COLOR
 //        // Normally shows BASE, but creates a blast effect using
 //        // the color BLAST when a blast is requested. The effect
 //        // is basically two humps moving out from the blast location.
 //        // The size of the humps can be changed with WAVE_SIZE, note
 //        // that smaller values makes the humps bigger. WAVE_MS determines
 //        // how fast the waves travel. Smaller values makes the waves
 //        // travel slower. Finally FADEOUT_MS determines how fast the
 //        // humps fade back to the base color.
 //        STYLE("Blast", "Blast", 
 //                Input("Base Color", COLORF),
 //                Input("Blast Color", COLORF), 
 //                Input("Fadeout Time (ms)", INT, 200),
 //                Input("Wave Size", INT, 100), 
 //                Input("Wave Time (ms)", INT, 400),
 //                Input("Trigger Effect", EFFECT, Effect::BLAST)),

 //        LAYER("BlastL", "Blast", 
 //                Input("Blast Color", COLORF),
 //                Input("Fadeout Time (ms)", INT, 200), 
 //                Input("Wave Size", INT, 100),
 //                Input("Wave Time (ms)", INT, 400),
 //                Input("Trigger Effect", EFFECT, Effect::BLAST)),

 //        // Usage: BlastFadeout<BASE, BLAST, FADEOUT_MS>
 //        // Or: BlastFadeoutL<BLAST, FADEOUT_MS>
 //        // BASE, BLAST: COLOR
 //        // FADEOUT_MS: a number (defaults to 250)
 //        // return value: COLOR
 //        // Normally shows BASE, but swiches to BLAST when a blast
 //        // is requested and then fades back to BASE. FADEOUT_MS
 //        // specifies out many milliseconds the fade takes.
 //        LAYER("BlastFadeoutL", "Blast Fadeout", 
 //                Input("Blast Color", COLORF),
 //                Input("Fadeout Time (ms)", INT, 250),
 //                Input("Trigger Effect", EFFECT, Effect::BLAST)),

 //        STYLE("BlastFadeout", "Blast Fadeout", 
 //                Input("Base Color", COLORF),
 //                Input("Blast Color", COLORF), 
 //                Input("Fadeout Time (ms)", INT, 250),
 //                Input("Trigger Effect", EFFECT, Effect::BLAST)),

 //        // Usage: OriginalBlast<BASE, BLAST>
 //        // Or: OriginalBlastL<BLAST>
 //        // BASE, BLAST: COLOR
 //        // return value: COLOR
 //        // Normally shows BASE, but creates a blast effect using
 //        // the color BLAST when a blast is requested.
 //        // This was the original blast effect, but it is slow and not
 //        // very configurable.
 //        LAYER("OriginalBlastL", "Original Blast", Input("Blast Color", COLORF),
 //              Input("Trigger Effect", EFFECT, Effect::BLAST)),

 //        STYLE("OriginalBlast", "Original Blast", Input("Base Color", COLORF),
 //              Input("Blast Color", COLORF),
 //              Input("Trigger Effect", EFFECT, Effect::BLAST)),

 //        // Usage: Blinking<A, B, BLINK_MILLIS, BLINK_PROMILLE>
 //        // or: BlinkingX<A, B, BLINK_MILLIS_FUNC, BLINK_PROMILLE_FUNC>
 //        // or: BlinkingL<B, BLINK_MILLIS_FUNC, BLINK_PROMILLE_FUNC>
 //        // A, B: COLOR
 //        // BLINK_MILLIS: a number
 //        // BLINK_PROMILLE: a number, defaults to 500
 //        // BLINK_MILLIS_FUNC: FUNCTION
 //        // BLINK_PROMILLE_FUNC: FUNCTION
 //        // return value: COLOR
 //        // Switches between A and B.
 //        // A full cycle from A to B and back again takes BLINK_MILLIS
 //        // milliseconds.
 //        // If BLINK_PROMILLE is 500, we select A for the first half and B for the
 //        // second half. If BLINK_PROMILLE is smaller, we get less A and more B.
 //        // If BLINK_PROMILLE is 0, we get all B.
 //        // If BLINK_PROMILLE is 1000 we get all A.
 //        LAYER("BlinkingL", "Blinking", 
 //                Input("Blink Color", COLORF),
 //                Input("Blink Time (ms)", INT | FUNCTION),
 //                Input("Distribution", INT | FUNCTION)),
 //        STYLE("BlinkingX", "Blinking", 
 //                Input("Color 1", COLORF),
 //                Input("Color 2", COLORF), 
 //                Input("Blink Time (ms)", INT | FUNCTION),
 //                Input("Distribution", INT | FUNCTION)),
 //        STYLE("Blinking", "Blinking", 
 //                Input("Color 1", COLORF),
 //                Input("Color 1", COLORF), 
 //                Input("Blink Time (ms)", INT),
 //                Input("Distribution", INT, 500)),

 //        // Usage: BrownNoiseFlicker<A, B, grade>
 //        // Or: BrownNoiseFlickerL<B, grade>
 //        // A, B: COLOR
 //        // grade: int
 //        // return value: COLOR
 //        // Randomly selects between A and B, but keeps nearby
 //        // pixels looking similar.
 //        LAYER("BrownNoiseFlickerL", "Brown Noise Flicker",
 //              Input("Flicker Color", COLORF), 
 //              Input("Grade", INT | FUNCTION)),

 //        STYLE("BrownNoiseFlicker", "Brown Noise Flicker",
 //                Input("Base Color", COLORF), 
 //                Input("Flicker Color", COLORF),
 //                Input("GRADE", INT) // For some reason the non-L version also has its
 //                                  // raw int multiplied by 128
 //             ),

 //        // Usage: ByteOrderStyle<BYTEORDER, COLOR>
 //        // BYTEORDER: Color8::RGB, or one of the other byte orders
 //        // COLOR: COLOR
 //        // return value: COLOR
 //        //
 //        // This shuffles the RGB values around. It's meant to be used
 //        // when you are mixing different kind of pixels on one string.
 //        // While it's not recommended to do so, this should make it
 //        // easier to compensate by re-ordering the channels.
 //        //
 //        // I'll need to revisit this to create a color order type
 //        // STYLE("ByteOrderStyle", "Rearrange Color Bytes",
 //        //       Arg("Color Order", COLORORDER),
 //        //       Arg("Color", COLOR)
 //        //       )

 //        // Usage: SimpleClash<BASE, CLASH_COLOR, CLASH_MILLIS>
 //        // Or: SimpleClashL<CLASH_COLOR, CLASH_MILLIS>
 //        // BASE: COLOR
 //        // CLASH_COLOR: COLOR (defaults to white)
 //        // CLASH_MILLIS: a number (defaults to 40)
 //        // return value: COLOR
 //        // Turns the blade to CLASH_COLOR for CLASH_MILLIS millseconds
 //        // when a clash occurs.
 //        LAYER("SimpleClashL", "Simple Clash", 
 //                Input("Clash Color", COLORF),
 //                Input("Clash Time (ms)", INT, 40),
 //                Input("Trigger Effect", EFFECT, Effect::CLASH),
 //                Input("Stab Shape", INT | FUNCTION, Generator::get("SmoothStep")({ Generator::get("Int")({16384}), Generator::get("Int")({24000})})) // Your guess is as good as mine as to what
 //                // this does...
 //              ),

 //        STYLE("SimpleClash", "Simple Clash",
 //                Input("Base Color", COLORF),
 //                Input("Clash Color", COLORF, Color ::WHITE),
 //                Input("Clash Time (ms)", INT, 40),
 //                Input("Trigger Effect", EFFECT, Effect ::CLASH),
 //                Input("Stab Shape", INT | FUNCTION, Generator ::get("SmoothStep")({ Generator ::get("Int")({16384}), Generator ::get("Int")({24000})})),
 //             ),

 //        // Usage: LocalizedClash<BASE, CLASH_COLOR, CLASH_MILLIS,
 //        // CLASH_WIDTH_PERCENT=50>
 //        // Usage: LocalizedClashL<CLASH_COLOR, CLASH_MILLIS,
 //        // CLASH_WIDTH_PERCENT=50>
 //        // BASE: COLOR
 //        // CLASH_COLOR: COLOR (defaults to white)
 //        // CLASH_MILLIS: a number (defaults to 40)
 //        // return value: COLOR
 //        // Similar to SimpleClash, but lights up a portion of the blade.
 //        // The fraction of the blade is defined by CLASH_WIDTH_PERCENT
 //        // The location of the clash is random within the middle half of the
 //        // blade.
 //        // Localized clashes should work well with stabs with no modifications.
 //        LAYER("LocalizedClashL", "Localized Clash", 
 //                Input("Clash Color", COLORF),
 //                Input("Clash Time (ms)", INT, 40), 
 //                Input("Clash Width (%)", INT, 50),
 //                Input("Trigger Effect", EFFECT, Effect::CLASH)),

 //        STYLE("LocalizedClash", "Localized Clash", 
 //                Input("Base Color", COLORF),
 //                Input("Clash Color", COLORF), 
 //                Input("Clash Time (ms)", INT, 40),
 //                Input("Clash Width (%)", INT, 50),
 //                Input("Trigger Effect", EFFECT, Effect::CLASH)
 //                ),

 //        // Usage: ColorCycle<COLOR, PERCENT, RPM>
 //        // or: ColorCycle<COLOR, PERCENT, RPM, ON_COLOR, ON_PERCENT, ON_RPM,
 //        // FADE_TIME_MILLIS, OFF_COLOR>
 //        // COLOR, ON_COLOR, OFF_COLOR: COLOR
 //        // RPM, PERCENT, ON_PERCENT, ON_RPM, FADE_TIME_MILLIS: a number
 //        // return value: COLOR
 //        // This is intended for a small ring of neopixels
 //        // A section of the ring is lit at the specified color
 //        // and rotates at the specified speed. The size of the
 //        // lit up section is defined by "percentage".
 //        // The arguments for this style are divided into two groups of
 //        // { COLOR, PERCENT, RPM }, one for while the blade is OFF, and the other
 //        // while it's ON.
 //        // BASE_COLOR is the color of pixels not part of the lit section, so if
 //        // PERCENT
 //        // is 0, the entire blade will be BASE_COLOR, and if PERCENT is 100, the
 //        // entire blade
 //        // will be OFF_COLOR or ON_COLOR (depending on blade state).
 //        // FADE_TIME_MILLIS is the time taken to transition between the OFF and ON
 //        // groups
 //        // of values.
 //        STYLE("ColorCycle", "Color Cycle", 
 //                Input("Off Color", COLORF),
 //                Input("Off Size (%)", INT), 
 //                Input("Off RPM", INT),
 //                Input("On Color", COLORF | REFARG_1),
 //                Input("On Size (%)", INT | REFARG_2), 
 //                Input("On RPM", INT | REFARG_3),
 //                Input("Fade Time (ms)", INT, 1),
 //                Input("Base Color", COLORF, Color::BLACK)
 //                ),

 //        // Usage: ColorSelect<SELECTION, TRANSITION, COLOR1, COLOR2, ...>
 //        // SELECTION: function
 //        // TRANSITION: transition
 //        // COLOR1, COLOR2, ...:  COLOR
 //        // Return value: COLOR
 //        // Decides what color to return based on the current selection.
 //        // The returned color will be selection % N (where N is the number of
 //        // colors arguments).
 //        // When the selection changes, the transition will be used to change from
 //        // the old color to the new color.
 //        STYLE("ColorSelect", "Color Select", 
 //                Input("Selection", INTF),
 //                Input("Transition", TRANSITIONF), 
 //                Input("Color 1", COLORF),
 //                Input("Color #", COLORF | VARIADIC)
 //                ),

 //        // Usage: ColorChange<TRANSITION, COLOR1, COLOR2, ...>
 //        // TRANSITION: transition
 //        // COLOR1, COLOR2, ...:  COLOR
 //        // Return value: COLOR
 //        // Decides what color to return based on the current variation.
 //        // The returned color will be current_variation % N (where N is the number
 //        // of colors arguments).
 //        // When the variation changes, the transition will be used to change from
 //        // the old color to the new color.
 //        STYLE("ColorChange", "Color Change", 
 //                Input("Transition", TRANSITIONF),
 //                Input("Color 1", COLORF), 
 //                Input("Color #", COLORF | VARIADIC)
 //                ),

 //        // Usage: Cylon<COLOR, PERCENT, RPM>
 //        // or: ColorCycle<COLOR, PERCENT, RPM, ON_COLOR, ON_PERCENT, ON_RPM,
 //        // FADE_TIME_MILLIS, OFF_COLOR>
 //        // COLOR, ON_COLOR, OFF_COLOR: COLOR
 //        // RPM, PERCENT, ON_PERCENT, ON_RPM, FADE_TIME_MILLIS: a number
 //        // return value: COLOR
 //        // Cylon/Knight Rider effect, a section of the strip is
 //        // lit up and moves back and forth. Speed, color and fraction
 //        // illuminated can be configured separately for on and off
 //        // states.
 //        // The arguments for this style are divided into two groups of
 //        // { COLOR, PERCENT, RPM }, one for while the blade is OFF, and the other
 //        // while it's ON.
 //        // BASE_COLOR is the color of pixels not part of the lit section, so if
 //        // PERCENT
 //        // is 0, the entire blade will be BASE_COLOR, and if PERCENT is 100, the
 //        // entire blade
 //        // will be OFF_COLOR or ON_COLOR (depending on blade state).
 //        // FADE_TIME_MILLIS is the time taken to transition between the OFF and ON
 //        // groups
 //        // of values.
 //        STYLE("Cylon", "Cylon", 
 //                Input("Off Color", COLORF),
 //                Input("Off Size (%)", INT), 
 //                Input("Off RPM", INT),
 //                Input("On Color", COLORF | REFARG_1),
 //                Input("On Size (%)", INT | REFARG_2), 
 //                Input("On RPM", INT | REFARG_3),
 //                Input("Fade Time (ms)", INT, 1),
 //                Input("Base Color", COLORF, Color::BLACK)
 //                ),
};

