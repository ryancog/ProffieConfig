#include "functions.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/functions.cpp
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

#include "proffieconstructs/vector3d.h"
#include "stylepreview/blade.h"
#include "styles/bladestyle.h"
#include "styles/elements/args.h"
#include "styles/elements/effects.h"
#include "styles/elements/lockuptype.h"
#include "utility/time.h"
#include <cmath>

using namespace BladeStyles;

static constexpr uint8_t blastHump[32]{
    255, 255, 252, 247, 240, 232, 222, 211,
    199, 186, 173, 159, 145, 132, 119, 106,
    94,   82,  72,  62,  53,  45,  38,  32,
    26,   22,  18,  14,  11,   9,   7,   5
};
/*
 * For the ones that make sense to be boolean output, TRUE is 32768 and FALSE is 0
 * For those that return a range, the range is nearly always 0 to 32768, same goes
 * for those that expect a range input. Nothing here is based off # of leds.
 */

/*
 * I've noticed X functions seem to be the "core" functions and tend to take INTs,
 * while non-X functions (if there's an X variant) take raw ints, and wrap the X version.
 */

FunctionStyle::FunctionStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent, StyleType typeOverride) :
    BladeStyle(osName, humanName, typeOverride ? typeOverride : BladeStyles::FUNCTION, params, parent) {}

Function3DStyle::Function3DStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent) :
    BladeStyle(osName, humanName, BladeStyles::FUNCTION3D, params, parent) {}

StyleGenerator BladeStyles::FunctionStyle::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& FunctionStyle::getMap() { return map; }

StyleGenerator Function3DStyle::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& Function3DStyle::getMap() { return map; }

#define RUN(varname) virtual void run(StylePreview::Blade& varname) override
#define GETINT(varname) virtual int32_t getInt(int32_t varname) override
#define GETINT3D(varname) virtual int32_t getInt(const Vector3D& varname) override

#define FUNCTEMPLATE(styleType, osName, humanName, params, ...) \
    class osName : public styleType { \
    public: \
        osName(const BladeStyle* parent) : styleType(#osName, humanName, params, parent) {} \
        __VA_ARGS__ \
    }; 
#define FUNC(osName, humanName, params, ...) FUNCTEMPLATE(FunctionStyle, osName, humanName, PARAMS(params), __VA_ARGS__)
#define FUNC3D(osName, humanName, params, ...) FUNCTEMPLATE(Function3DStyle, osName, humanName, PARAMS(params), __VA_ARGS__)

// Usage: Int<N>
// Returns N
// N: a number
// return value: INTEGER
FUNC(Int, "Number",
        PARAMS(
            new NumberParam("")
            ),
    RUN() {}
    GETINT() { return getParamNumber(0); }
    )

// Usage: AltF
// return value: INTEGER
// Returns current_alternative for use in ColorSelect<>, TrSelect<> or IntSelect<>
FUNC(AltF, "Get Current Alt", PARAMS(), 
    RUN() {} 
    GETINT() { return curAlt; }
    
    private:
        uint16_t curAlt{0};
    )

// Usage: SyncAltToVarianceF
// return value: INTEGER (always zero)
// Enables Bidirectional synchronization between ALT and VARIANCE.
// If variance changes, so does alt, if alt changes, so does variance.
FUNC(SyncAltToVarianceF, "Sync Alt and Variance", PARAMS(),
    RUN() {}
    GETINT() { return 0; }
    )

// Usage: SyncAltToVarianceL
// return value: LAYER (transparent)
// Synchronizes alt to variance, just put it somewhere in the layer stack. (but not first)
// LAYER("SyncAltToVarianceL", "Sync Alternative and Variance"),

// Usage: BatteryLevel
// Returns 0-32768 based on battery level.
// returned value: INTEGER
FUNC(BatteryLevel, "Battery Level", PARAMS(),
        RUN(blade) { value = blade.batteryLevel; }
        GETINT() { return value; }

        private: 
            uint16_t value;
    )

// Usage: BladeAngleX<MIN, MAX>
// Returns 0-32768 based on angle of blade
// MIN : FUNCTION (defaults to Int<0>)
// MAX : FUNCTION (defaults to Int<32768>)
// MIN and MAX specifies the range of angles which are used.
// For MIN and MAX 0 means down and 32768 means up and 16384 means
// pointing towards the horizon.
// So if MIN=16484 and MAX=32768, BladeAngle will return zero when you
// point the blade towards the horizon and 32768 when you point it
// straight up. Any angle below the horizon will also return zero.
// returned value: FUNCTION, same for all leds
FUNC(BladeAngle, "Blade Angle",
        PARAMS(
            new NumberParam("Min", 0),
            new NumberParam("Max", 32768)
            ),

        RUN() {}
        GETINT() { return 0; }
    )

FUNC(BladeAngleX, "Blade Angle",
        PARAMS(
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768)))
            ),
        RUN() {}
        GETINT() { return 0; }
    )

// Usage: BlastF<FADEOUT_MS, WAVE_SIZE, WAVE_MS, EFFECT>
// FADOUT_MS: a number (defaults to 200)
// WAVE_SIZE: a number (defaults to 100)
// WAVE_MS: a number (defaults to 400)
// EFFECT: a BladeEffectType (defaults to EFFECT_BLAST)
// returned value: FUNCTION
// This function is intended to Mix<> or AlphaL<>, when a
// a blast  occurs, it makes a wave starting at the blast.
// location (which is currently random) and travels out
// from that direction. At the peak, this function returns
// 32768 and when there is no blash it returns zero.
// The FADOUT_MS controls how long it takes the wave to
// fade out. The WAVE_SIZE controls the width of the wave.
// The WAVE_MS parameter controls the speed of the waves.
// EFFECT can be used to trigger this effect by something
// other than a blast effect.
FUNC(BlastF, "Blast",
        PARAMS(
            new NumberParam("Fadeout (ms)", 200),
            new NumberParam("Wave Size", 100),
            new NumberParam("Wave Time (ms)", 400),
            new StyleParam("Trigger Effect", EFFECT, EffectStyle::get("BLAST")(this, PARAMVEC()))
            ),
        RUN(blade) {
            numLeds = blade.numLeds;
            effects = blade.getEffects();

            fadeTime = getParamNumber(0);
            waveSize = getParamNumber(1);
            waveTime = getParamNumber(2);
            thisEffect = static_cast<const EffectStyle*>(getParamStyle(3))->effect;
        }
        GETINT(led) {
            uint32_t mix{0};
            for (const auto& effect : effects) {
                if (effect.type != thisEffect) continue;
                auto timeDelta{Utility::getTimeMicros() - effect.startMicros};
                // No clue what "M" means...
                auto M{1000 - (timeDelta / fadeTime)};
                if (M > 0) {
                    auto dist{fabsf(effect.location - (led / static_cast<float>(numLeds)))};
                    auto N{static_cast<uint32_t>(fabsf(dist - (timeDelta / (waveTime * 1000.f))) * waveSize)};
                    if (N <= 32) mix += (blastHump[N] * M) / 1000;
                }
            }
            return std::min(mix << 7, static_cast<uint32_t>(32768));
        }

        private:
            uint16_t fadeTime;
            uint16_t waveSize;
            uint16_t waveTime;
            Effect thisEffect;

            uint16_t numLeds;
            std::vector<StylePreview::BladeEffect> effects;
    )

// Usage: BlastFadeoutF<FADEOUT_MS, EFFECT>
// FADEOUT_MS: a number (defaults to 250)
// EFFECT: a BladeEffectType (defaults to EFFECT_BLAST)
// return value: FUNCTION
// NOrmally returns 0, but returns up to 32768 when the
// selected effect occurs. Then if fades back to zero over
// FADEOUT_MS milliseconds.
FUNC(BlastFadeoutF, "Blast Fadeout",
        PARAMS(
            new NumberParam("Fade Time (ms)", 250),
            new StyleParam("Trigger Effect", EFFECT, EffectStyle::get("BLAST")(this, PARAMVEC()))
            ),
        RUN(blade) {
            fadeTime = getParamNumber(0);
            thisEffect = static_cast<const EffectStyle*>(getParamStyle(1))->effect;

            numLeds = blade.numLeds;
            effects = blade.getEffects();
        }
        GETINT() {
            if (effects.size() == 0) return 0;
            uint32_t mix{0};
            for (const auto& effect : effects) {
                if (effect.type != thisEffect) continue;
                auto timeDelta{Utility::getTimeMicros() - effect.startMicros};
                auto M{1000 - (timeDelta / fadeTime)};
                if (M > 0) mix += (32768 * M) / 1000;
            }
            return std::min(mix, static_cast<uint32_t>(32768));
        }
        private:
            uint16_t fadeTime;
            Effect thisEffect{Effect::NONE};

            uint16_t numLeds;
            std::vector<StylePreview::BladeEffect> effects;
    )

// Usage: OriginalBlastF<EFFECT>
// EFFECT: a BladeEffectType (defaults to EFFECT_BLAST)
// return value: FUNCTION
// Original blast function. Normally returns zero, but
// returns up to 32768 when the selected effect occurs.
FUNC(OriginalBlastF, "Original Blast",
        PARAMS(
            new StyleParam("Trigger Effect", EFFECT, EffectStyle::get("BLAST")(this, PARAMVEC()))
            ),
        RUN(blade) {
            thisEffect = static_cast<const EffectStyle*>(getParamStyle(0))->effect;

            numLeds = blade.numLeds;
            effects = blade.getEffects();
        }
        GETINT(led) {
            if (effects.size() == 0) return 0;
            float mix{0.f};
            for (const auto& effect : effects) {
                if (effect.type != thisEffect) continue;
                float x{effect.location - (led / static_cast<float>(numLeds)) * 30.f};
                auto timeDelta{Utility::getTimeMicros() - effect.startMicros};
                auto t{0.5 + (timeDelta / 200000.f)};
                if (x == 0.f) mix += 2.f / (t * t);
                else mix += std::max(0.f, 2.f * (sinf(x / (t * t)) / x));
            }
            return std::min(mix, 1.f) * 32768;
        }
        private:
            Effect thisEffect{Effect::NONE};

            uint16_t numLeds;
            std::vector<StylePreview::BladeEffect> effects;
    )

// Usage: BlinkingF<A, B, BLINK_MILLIS_FUNC, BLINK_PROMILLE_FUNC>
// BLINK_MILLIS: a number
// BLINK_PROMILLE: a number, defaults to 500
// BLINK_MILLIS_FUNC: FUNCTION
// BLINK_PROMILLE_FUNC: FUNCTION
// return value: FUNCTION
// Switches between 0 and 32768
// A full cycle from 0 to 328768 and back again takes BLINK_MILLIS milliseconds.
// If BLINK_PROMILLE is 500, we select A for the first half and B for the
// second half. If BLINK_PROMILLE is smaller, we get less A and more B.
// If BLINK_PROMILLE is 0, we get all 0.
// If BLINK_PROMILLE is 1000 we get all 32768.
//
// This documentation I think is maybe talking about some other layer or style instead?
// It's clearly not right, but based on the actual code the following seems correct:
FUNC(BlinkingF, "Blinking",
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION, get("Int")(this, PARAMVEC(1000))),
            new StyleParam("Distribution", FUNCTION, get("Int")(this, PARAMVEC(500)))
            ),
        RUN(blade) {
            pulseTime = const_cast<decltype(pulseTime)>(static_cast<const FunctionStyle*>(getParamStyle(0)));
            pulseDist = const_cast<decltype(pulseDist)>(static_cast<const FunctionStyle*>(getParamStyle(1)));

            pulseTime->run(blade);
            pulseDist->run(blade);
        }
        GETINT() {
            auto now{Utility::getTimeMicros()};
            auto pulseMillis{static_cast<uint32_t>(pulseTime->getInt(0))};
            if (pulseMillis <= 0) return 0;

            auto pulseProgressMicros{now - pulseStartMicros};
            if (pulseProgressMicros > pulseMillis * 1000) {
                if (pulseProgressMicros < pulseMillis * 2000) pulseStartMicros += pulseMillis * 1000;
                else pulseStartMicros = now;
                pulseProgressMicros = now - pulseStartMicros;
            }
            auto pulseProgress{pulseProgressMicros / pulseMillis};
            return pulseProgress <= static_cast<uint32_t>(pulseDist->getInt(0)) ? 0 : 32768;
        };
        private:
            FunctionStyle* pulseTime{nullptr};
            FunctionStyle* pulseDist{nullptr};

            uint32_t pulseStartMicros{0};
    )

// Usage: BrownNoiseF<GRADE>
// return value: FUNCTION
// Returns a value between 0 and 32768 with nearby pixels being similar.
// GRADE controls how similar nearby pixels are.
FUNC(BrownNoiseF, "Brown Noise",
        PARAMS(
            new StyleParam("Grade", FUNCTION, nullptr)
            ),
        RUN(blade) {
            grade = const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(0)));
            grade->run(blade);

            mix = rand() % 32768; // Not sure what random Fredrik uses... but this isn't inclusive
        }
        GETINT(led) {
            auto gradeValue{grade->getInt(led)};
            auto rawMixValue{static_cast<uint16_t>(mix + (rand() % ((gradeValue * 2) + 1)) - gradeValue)};
            // Fredrik's clamps the negative but here I use a uint, so no need.
            mix = std::min<uint16_t>(32768, rawMixValue);
            return mix;
        }
        private:
            uint16_t mix;
            FunctionStyle* grade{nullptr};
    )

// Usage: SlowNoise<SPEED>
// return value: FUNCTION
// Returns a value between 0 and 32768 which changes randomly up and
// down over time. All pixels gets the same value.
// SPEED controls how quickly the value changes.
FUNC(SlowNoise, "Slow Noise",
        PARAMS(
            new StyleParam("Speed", FUNCTION, nullptr)
            ),
        RUN(blade) {
            // This happens in the ctor in ProffieOS, hence the magic number and assignment
            if (value == 0xFFFF) value = rand() % 32768;

            auto speed{const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(0)))};
            speed->run(blade);
            auto now{Utility::getTimeMS()};
            auto delta{now - lastMillis};
            if (delta > 1000) delta = 1;
            lastMillis = now;
            auto speedValue{speed->getInt(0)};

            while (delta--) value = std::max<uint16_t>(value + ((rand() % ((speedValue * 2)  + 1)) - speedValue), 0);
        }
        GETINT() { return value; }
        private:
            uint16_t value{0xFFFF};
            uint64_t lastMillis{0};
    )

// Usage: Bump<BUMP_POSITION, BUMP_WIDTH_FRACTION>
// Returns different values for each LED, forming a bump shape.
// If BUMP_POSITION is 0, bump will be at the hilt.
// If BUMP_POSITION is 32768, the bump will be at the tip.
// If BUMP_WIDTH_FRACTION is 1, bump will be extremely narrow.
// If BUMP_WIDTH_FRACTION is 32768, it will fill up most/all of the blade.
// BUMP_POSITION, BUMP_WIDTH_FRACTION: INTEGER
FUNC(Bump, "Bump",
        PARAMS(
            new StyleParam("Position", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Width", FUNCTION, get("Int")(this, PARAMVEC(16385)))
            ),
        RUN(blade) {
            pos = const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(0)));
            width = const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(1)));

            pos->run(blade);
            width->run(blade);

            auto widthValue{width->getInt(0)};
            if (widthValue == 0) {
                multiplier = 1;
                location = -10000;
                return;
            }

            multiplier = static_cast<uint16_t>(32 * 2.f * 128 * 32768 / widthValue / blade.numLeds);
            location = (pos->getInt(0) * blade.numLeds * multiplier) / 32768;
        }
        GETINT(led) {
            auto distance{static_cast<uint32_t>(abs((static_cast<int32_t>(led) * multiplier) - location))};
            auto p{distance >> 7};

            if (p >= (sizeof(bumpShape) / sizeof(bumpShape[0]) - 1)) return 0;

            auto m{distance & 0x3F};
            return (bumpShape[p] * (128 - m)) + (bumpShape[p + 1] * m);
        }
        private:
            FunctionStyle* pos;
            FunctionStyle* width;

            int32_t location;
            int32_t multiplier;

            static constexpr uint8_t bumpShape[33] = {
                255, 255, 252, 247, 240, 232, 222, 211,
                199, 186, 173, 159, 145, 132, 119, 106,
                 94,  82,  72,  62,  53,  45,  38,  32,
                 26,  22,  18,  14,  11,   9,   7,   5, 0
            };
    )

// Usage: HumpFlickerFX<FUNCTION>
// or: HumpFlickerF<N>
// FUNCTION: FUNCTION
// N: NUMBER
// return value: INTEGER
// Creates hump shapes that randomize over the blade.
// The returned INTEGER is the size of the humps.
// Large values can give the blade a shimmering look,
// while small values look more like speckles.
FUNC(HumpFlickerF, "Hump Flicker",
        PARAMS(
            new NumberParam("Hump Width", FUNCTION)
            ),
        RUN() {}
        GETINT() {}
    )

FUNC(HumpFlickerFX, "Hump Flicker",
        PARAMS(
            new StyleParam("Hump Width", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Remap<CenterDistF<CENTER>,COLOR>
// Distributes led COLOR from CENTER
// CENTER : FUNCTION (defaults to Int<16384>)
//
//
FUNC(CenterDistF, "Center Distribution",
        PARAMS(
            new StyleParam("Center", FUNCTION, get("Int")(this, PARAMVEC(16384)))
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: ChangeSlowly<F, SPEED>
// Changes F by no more than SPEED values per second.
// F, SPEED: FUNCTION
// return value: FUNCTION, same for all LEDs
FUNC(ChangeSlowly, "Change Slowly",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Speed", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: CircularSectionF<POSITION, FRACTION>
// POSITION: FUNCTION position on the circle or blade, 0-32768
// FRACTION: FUNCTION how much of the blade to light up, 0 = none, 32768 = all of it
// return value: FUNCTION
// Returns 32768 for LEDs near the position with wrap-around.
// Could be used with MarbleF<> for a marble effect, or with
// Saw<> for a spinning/colorcycle type effect.
// Example: If POSITION = 0 and FRACTION = 16384, then this function
// will return 32768 for the first 25% and the last 25% of the blade
// and 0 for the rest of the LEDs.
FUNC(CircularSectionF, "Circular Section",
        PARAMS(
            new StyleParam("Position", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Fraction", FUNCTION, get("Int")(this, PARAMVEC(16384))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: ClampF<F, MIN, MAX>
// Or:    ClampFX<F, MINCLASS, MAXCLASS>
// Clamps value between MIN and MAX
// F, MIN, MAX: INTEGER
// MINCLASS, MAXCLASS: FUNCTION
// return value: INTEGER
FUNC(ClampF, "Clamp",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new NumberParam("Min", 0),
            new NumberParam("Max", 32768)
            ),
        RUN() {}
        GETINT() {}
    )

FUNC(ClampFX, "Clamp",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: ClampF<F, MIN, MAX>
// Or:    ClampFX<F, MINCLASS, MAXCLASS>
// Clamps value between MIN and MAX
// F, MIN, MAX: INTEGER
// MINCLASS, MAXCLASS: FUNCTION
// return value: INTEGER
FUNC(ClashImpactF, "Clash Impact",
        PARAMS(
            new NumberParam("Min", 0),
            new NumberParam("Max", 32768)
            ),
        RUN() {}
        GETINT() {}
    )

FUNC(ClashImpactFX, "Clash Impact",
        PARAMS(
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Divide<F, V>
// Divide F by V
// If V = 0, returns 0
// F, V: FUNCTION,
// return value: FUNCTION
// Please note that Divide<> isn't an exact inverse of Mult<> because mult uses fixed-point mathematics
// (it divides the result by 32768) while Divide<> doesn't, it just returns F / V
FUNC(Divide, "Divide",
        PARAMS(
            new StyleParam("Numerator", FUNCTION, nullptr),
            new StyleParam("Denominator", FUNCTION, nullptr),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: EffectPulse<EFFECT>
// EFFECT: BladeEffectType
// Returns 32768 once for each time the given effect occurs.
FUNC(EffectPulse, "Pulse On Effect",
        PARAMS(
            new StyleParam("Trigger Effect", EFFECT, nullptr)
            
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: LockupPulseF<LOCKUP_TYPE>
// LOCKUP_TYPE: a SaberBase::LockupType
// Returns 32768 once for each time the given lockup occurs.
FUNC(LockupPulseF, "Pulse On Lockup",
        PARAMS(
            new StyleParam("Trigger Lockup", LOCKUPTYPE, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: IncrementWithReset<PULSE, RESET_PULSE, MAX, I>
// PULSE: FUNCTION (pulse type)
// RESET_PULSEE: FUNCTION (pulse type) defaults to Int<0> (no reset)
// MAX, I: FUNCTION
// Starts at zero, increments by I each time the PULSE occurse.
// If it reaches MAX it stays there.
// Resets back to zero when RESET_PULSE occurs.
FUNC(IncrementWithReset, "Increment With Reset",
        PARAMS(
            new StyleParam("Increment Pulse",   FUNCTION, nullptr),
            new StyleParam("Pulse Reset", 	    FUNCTION, nullptr),
            new StyleParam("Max", 			    FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Increment", 		FUNCTION, get("Int")(this, PARAMVEC(1)))
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: EffectIncrementF<EFFECT, MAX, I>
// Increases by value I (up to MAX) each time EFFECT is triggered
// If current value + I = MAX, it returns 0.
// If adding I exceeds MAX, the function returns 0 + any remainder in excesss of MAX
// I, MAX = numbers
// return value: INTEGER
FUNC(EffectIncrementF, "Increment On Effect",
        PARAMS(
            new StyleParam("Trigger Effect", EFFECT, nullptr),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Increment", FUNCTION, get("Int")(this, PARAMVEC(1)))
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: EffectPosition<>
// Or: EffectPosition<EFFECT>
// EFFECT: effect type
// return value: INTEGER
//
// EffectPosition returns the position of a particular effect. 0 = base, 32768 = tip.
// For now, this location is random, but may be set explicitly in the future.
// When used as EffectPosition<> inside a TransitionEffectL whose EFFECT is already specified,
// then it will automatically use the right effect.
FUNC(EffectPosition, "Effect Position",
        PARAMS(
            new StyleParam("Effect", EFFECT, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: HoldPeakF<F, HOLD_MILLIS, SPEED>
// Holds Peak value of F for HOLD_MILLIS.
// then transitions down over SPEED to current F
// F, HOLD_MILLIS and SPEED: FUNCTION
// return value: FUNCTION, same for all LEDs
FUNC(HoldPeakF, "Hold Peak Value",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Hold Time (ms)", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Ramp Down Speed", FUNCTION, get("Int")(this, PARAMVEC(0))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Ifon<A, B>
// Returns A if saber is on, B otherwise.
// A, B: INTEGER
// return value: INTEGER
FUNC(Ifon, "If On",
        PARAMS(
            new StyleParam("Value if On", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Value if Off", FUNCTION, get("Int")(this, PARAMVEC(0))),
            ),
        RUN() {}
        GETINT() {}
    )

// InOutFunc<OUT_MILLIS, IN_MILLIS>
// IN_MILLIS, OUT_MILLIS: a number
// RETURN VALUE: FUNCTION
// 0 when off, 32768 when on, takes OUT_MILLIS to go from 0 to 32768
// takes IN_MILLIS to go from 32768 to 0.
// NEED A BETTER HUMAN NAME
FUNC(InOutFunc, "In Out Func",
        PARAMS(
            new NumberParam("Out Time (ms)", 300),
            new NumberParam("In Time (ms)", 600)
            ),
        RUN() {}
        GETINT() {}
    )

FUNC(InOutFuncX, "In Out Func",
        PARAMS(
            new StyleParam("Out Time (ms)", FUNCTION, get("Int")(this, PARAMVEC(300))),
            new StyleParam("In Time (ms)", FUNCTION, get("Int")(this, PARAMVEC(600))),
            ),
        RUN() {}
        GETINT() {}
    )

// Thermal Detonator?
// NO DOCUMENTATION
// NEED A BETTER HUMAN NAME
FUNC(InOutFuncTD, "In Out Func TD",
        PARAMS(
            new NumberParam("Out Time (ms)"),
            new NumberParam("In Time (ms)"),
            new NumberParam("Explode Time (ms)")
            ),
        RUN() {}
        GETINT() {}
    )

// NO DOCUMENTATION
// NEED A BETTER HUMAN NAME
FUNC(InOutHelperF, "In Out Helper",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new BoolParam("Allow Disable", true)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: IncrementModulo<PULSE, MAX, INCREMENT>
// PULSE: FUNCTION (pulse type)
// MAX: FUNCTION (not zero) defaults to Int<32768>
// INCREMENT: FUNCTION defaults to Int<1>
// Increments by I each time PULSE occurs wraps around when
// it reaches MAX.
// The documentation is incorrect, the name does end in F
FUNC(IncrementModuloF, "Increment With Wrap",
        PARAMS(
            new StyleParam("Pulse Input", FUNCTION, nullptr),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Increment", FUNCTION, get("Int")(this, PARAMVEC(1))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: ThresholdPulseF<F, THRESHOLD, HYST_PERCENT>
// F: FUNCTION
// THRESHOLD: FUNCTION (defaults to Int<32768>)
// HYST_PERCENT: FUNCTION (defaults to Int<66>
// Returns 32768 once when F > THRESHOLD, then waits until
// F < THRESHOLD * HYST_PERCENT / 100 before going back
// to the initial state (waiting for F > THRESHOLD).
// BETTER HUMAN NAME?
FUNC(ThresholdPulseF, "Pulse on Threshold",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Threshold", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Hysteresis %", FUNCTION, get("Int")(this, PARAMVEC(66))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: IncrementF<F, V, MAX, I, HYST_PERCENT>
// Increases by value I (up to MAX) each time F >= V
// Detection resets once F drops below V * HYST_PERCENT
// if greater than MAX returns 0
// F, V, I, MAX = numbers
// HYST_PERCENT = percent (defaults to 66)
// return value: INTEGER
// NOTE: this function is designed to separate "events" for use with *Select styles.
// This function may break up SwingSpeed effects or other continuous responsive functions.
FUNC(IncrementF, "Increment on Input",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Increment Threshold", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Max Value", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            new StyleParam("Increment", FUNCTION, get("Int")(this, PARAMVEC(1))),
            new StyleParam("Hysteresis %", FUNCTION, get("Int")(this, PARAMVEC(66))),
            ),
        RUN() {}
        GETINT() {}
    )

// NO DOCUMENTATION
// I think this is for the edit mode args?
// This is what "Arg" runs through:
// void init(int argnum) {
// 	char default_value[16];
// 	itoa(value_, default_value, 10);
// 	const char* arg = CurrentArgParser->GetArg(argnum", "INT", default_value);
// 	if (arg) {
//     value_ = strtol(arg, NULL, 0);
// 	}
// }
// So maybe it gets an arg from the list based on the arg num?
// UPDATE: I'm almost certain this is what it does based on some investigation,
// RgbArg does the same thing...
FUNC(IntArg, "Number Argument",
        PARAMS(
            new StyleParam("Arg", ARGUMENT, nullptr),
            new NumberParam("Default Value")
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: IntSelect<SELECTION, Int1, Int2...>
// SELECTION: FUNCTION
// Returns SELECTION of N
// If SELECTION is 0, the first integer is returned, if SELECTIOn is 1, the second and so forth.
// N: numbers
// return value: INTEGER
FUNC(IntSelect, "Number Select",
        PARAMS(
            new StyleParam("Selection", FUNCTION, nullptr),
            new NumberParam("Number #", 0, VARIADIC)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: IsBetween<F, BOTTOM, TOP>
// Returns 0 or 32768 based F > BOTTOM and < TOP
// F, BOTTOM, TOP: INTEGER
// return value: INTEGER
FUNC(IsBetween, "Is Between",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: IsLessThan<F, V>
// Returns 0 or 32768 based on V
// If F < V returns 32768, if F >= V returns 0
// F, V: INTEGER
// return value: INTEGER
FUNC(IsLessThan, "Is Less Than",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Compare To", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

FUNC(IsGreaterThan, "Is Greater Than",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Compare To", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: LayerFunctions<F1, F2, ...>
// F1, F2: FUNCTIONS
// return value: FUNCTION
// Returns (32768 - (32768 - F1) * (32768 * F2) / 32768)
// This is the same as 1-(1-F1)*(1-F2), but multiplied by 32768.
// Basically Mix<LayerFunctions<F1, F2>, A, B> is the same as Mix<F2, Mix<F1, A, B>, B>.
//
// This one makes no sense to me... will need to revisit
// FUNC("LayerFunctions", "LayerFunctions", INT | FUNCTION
//
// )

// Usage: LinearSectionF<POSITION, FRACTION>
// POSITION: FUNCTION position on the blade, 0-32768
// FRACTION: FUNCTION how much of the blade to light up, 0 = none
// return value: FUNCTION
// creates a "block" of pixels at POSITION taking up FRACTION of blade
FUNC(LinearSectionF, "Linear Section",
        PARAMS(
            new StyleParam("Position", FUNCTION, get("Int")(this, PARAMVEC(16384))),
            new StyleParam("Section Size", FUNCTION, get("Int")(this, PARAMVEC(16384))),
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: MarbleF<OFFSET, FRICTION, ACCELERATION, GRAVITY>
// OFFSET: FUNCTION  0-32768, adjust until "down" represents is actually down
// FRICTION: FUNCTION, higher values makes the marble slow down, usually a constant
// ACCELERATION: FUNCTION, a function specifying how much speed to add to the marble
// GRAVITY: FUNCTION higher values makes the marble heavier
// return value: FUNCTION  0-32768, representing point on a circle
// This is intended for a small ring of neopixels.
// It runs a simulation of a marble trapped in a circular
// track and returns the position of that marble.
// Meant to be used with CircularSectionF to turn the marble
// position into a lighted up section.
FUNC(MarbleF, "Marble Simulation",
        PARAMS(
            new StyleParam("Direction Offset",  FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Friction",          FUNCTION, get("Int")(this, PARAMVEC(16384))),
            new StyleParam("Acceleration",      FUNCTION, get("Int")(this, PARAMVEC(16384))),
            new StyleParam("Gravity",           FUNCTION, get("Int")(this, PARAMVEC(16384))),
        ),
        RUN() {}
        GETINT() {}
    )

// Usage: ModF<F, MAX>
// F: FUNCTION
// MAX: FUNCTION (not zero)
// When F is greater than MAX, F wraps to 0
// When F is less than 0, F wraps to MAX
// returns Integer
FUNC(ModF, "Modulo",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Divisor", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Mult<F, V>
// Fixed point multiplication of values F * V,
// fixed point 16.15 arithmetic (32768 = 1.0)
// (2*2 would not result in 4),
// (16384 * 16384 = 8192, representation of 0.5*0.5=0.25)
// most blade functions use this method of fixed point calculations
// F, V: INTEGER,
// return value: INTEGER
FUNC(Mult, "Multiply",
        PARAMS(
            new StyleParam("Factor 1", FUNCTION, nullptr),
            new StyleParam("Factor 2", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Percentage<F, V>
// Gets Percentage V of value F,
// Percentages over 100% are allowed and will effectively be a multiplier.
// F, V: INTEGER
// example Percentage<Int<16384>,25>
// this will give you 25% of Int<16384> and returns Int<4096>
// return value: INTEGER
FUNC(Percentage, "Percentage",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Percent", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: OnsparkF<MILLIS>
// MILLIS: FUNCTION (defaults to Int<200>)
// return value: FUNCTION
// When the blade turns on, this function starts returning
// 32768, then fades back to zero over MILLIS milliseconds.
// This is intended to be used with Mix<> or AlphaL<> to
// to create a flash of color or white when the blade ignites.
FUNC(OnSparkF, "On Spark",
        PARAMS(
            new StyleParam("Fade Time (ms)", FUNCTION, get("Int")(this, PARAMVEC(200)))
            ),
        RUN() {}
        GETINT() {}
    )

// Returns led as value between 0 ~ 32768
// Keeps existing mapping for pixels when used with Remap<>
// Example: Remap<RampF,COLOR>
// NEED A BETTER HUMAN NAME
FUNC(RampF, "LED Position", PARAMS(),
        RUN() {}
        GETINT() {}
    )

// Usage: RandomF
// Return value: FUNCTION
// Returns a random number between 0 and 32768.
// All LEDS gets the same value.
FUNC(RandomF, "Random", PARAMS(),
        RUN() {}
        GETINT() {}
    )

// Usage: RandomPerLEDF
// Return value: FUNCTION
// Returns a random number between 0 and 32768.
// Each LED gets a different random value.
FUNC(RandomPerLEDF, "Random Per LED", PARAMS(),
        RUN() {}
        GETINT() {}
    )

// Usage: EffectRandomF<EFFECT>
// Returns a random value between 0 and 32768 each time EVENT is triggered
// return value: INTEGER
//
// Except it's each time an EFFECT is triggered, not EVENT.
// RandomEffect makes more sense for a name to me...
FUNC(EffectRandom, "Random On Effect",
        PARAMS(
            new StyleParam("Trigger Effect", EFFECT, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: RandomBlinkF<MILLIHZ>
// MILLHZ: FUNCTION
// Randomly returns either 0 or 32768 for each LED. The returned value
// is held, but changed to a new random value MILLIHZ * 1000 times per
// second.
// NEED A BETTER HUMAN NAME
FUNC(RandomBlinkF, "Random Per Interval",
        PARAMS(
            new StyleParam("Time (mHz)", FUNCTION, get("Int")(this, PARAMVEC(1)))
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Scale<F, A, B>
// Changes values in range 0 - 32768 to A-B
// F, A, B: INTEGER
// return value: INTEGER
FUNC(Scale, "Scale",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr),
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768)))
            ),
        RUN() {}
        GETINT() {}
    )

// To simplify inverting a function's returned value
// Example InvertF<BladeAngle<>> will return 0 when up and 32768 when down
//
// This actually just uses Scale<F, Int<32768>, Int<0>> under the hood, so
// is an example I suppose...
FUNC(InvertF, "Invert",
        PARAMS(
            new StyleParam("Input", FUNCTION, nullptr)
            ),
        RUN() {}
        GETINT() {}
    )

// usage: SequenceF<millis_per_bits, bits, 0b0000000000000000, ....>
// millis_per_bit: a number, millseconds spent on each bit
// bits: a number, number of bits before we loop around to the beginning
// 0b0000000000000000: 16-bit binary numbers containing the actual sequence.
//
// Returns 32768 if the current bit in the sequence is 1, 0 otherwise.
// The number of 16-bit binary numbers should be at least |bits| / 16, rounded up.
// Note that if not all bits are used within the 16-bit number.
// Example, an SOS pattern:
// SequenceF<100, 37, 0b0001010100011100, 0b0111000111000101, 0b0100000000000000>
FUNC(SequenceF, "Binary Sequence",
        PARAMS(
            new NumberParam("Time Per Bit (ms)", 100),
            new NumberParam("Number of Bits", 16),
            new BitsParam("Bit Section #", 0b1010101010101010, VARIADIC)
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: Sin<RPM, LOW, HIGH>
// pulses between LOW - HIGH RPM times per minute
// LOW: INTEGER (defaults to Int<0>)
// HIGH: INTEGER (defaults to Int<32768>)
// RPM: INTEGER
// return value: INTEGER
FUNC(Sin, "Sin",
        PARAMS(
            new StyleParam("RPM", FUNCTION, get("Int")(this, PARAMVEC(60))),
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768)))
            ),
        RUN() {}
        GETINT() {}
    )

// NO DOCUMENTATION
FUNC(Saw, "Saw",
        PARAMS(
            new StyleParam("RPM", FUNCTION, get("Int")(this, PARAMVEC(60))),
            new StyleParam("Min", FUNCTION, get("Int")(this, PARAMVEC(0))),
            new StyleParam("Max", FUNCTION, get("Int")(this, PARAMVEC(32768)))
            ),
        RUN() {}
        GETINT() {}
    )

// NO DOCUMENTATION
FUNC(PulsingF, "Pulsing",
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION, get("Int")(this, PARAMVEC(100)))
            ),
        RUN() {}
        GETINT() {}
    )

// Usage: SliceF<DENSITY_FUNCTION>
// or: SliceF<DENSITY_FUNCTION, OFFSET>
// DENSITY_FUNCTION: 3DF 
// OFFSET: integer, defaults to 20
// return value: FUNCTION
// The DENSITY_FUNCTION is a 3-dimensional function, f(x, y, z)
// the SliceF function calculates the x/y/z coordinates based on the
// angle of the blade. For now, the only density functions available
// are SmokeDF and FastSmokeDF, which are basically the same thing.
// This is very similar to how the POV blade works, but instead of
// using a large data blob as input, it just uses another function
// as input.
//
// NEED TO ADD DENSITY FUNCTIONS
FUNC(SliceF, "Slice",
        PARAMS(
            new StyleParam("Density Function", FUNCTION3D, nullptr),
            new NumberParam("Offset", 20)
            ),
        RUN() {}
        GETINT() {}
    )


// Usage: SmoothStep<POS, WIDTH>
// POS, WIDTH: FUNCTION
// return value: FUNCTION
// POS: specifies the middle of the smoothstep, 0 = base of blade, 32768=tip
// WIDTH: witdth of transition, 0 = no transition, 32768 = length of blade
// Example: SmoothStep<Int<16384>, Int<16384>> returns 0 up until 25% of the blade.
// From there it has a smooth transition to 32768, which will be reached at 75% of
// the blade. If WIDTH is negative, the transition will go the other way.
FUNC(SmoothStep, "Smooth Step",
        PARAMS(
            new StyleParam("Position", FUNCTION, get("Int")(this, PARAMVEC(16384))),
            new StyleParam("Width", FUNCTION, get("Int")(this, PARAMVEC(16384))),
            ),
        RUN() {}
        GETINT() {}
    )

FUNC3D(SmokeDF, "Smoke",
        PARAMS(),
        RUN() {}
        GETINT3D() {}
      )

FUNC3D(FastSmokeDF, "Fast Smoke",
        PARAMS(),
        RUN() {}
        GETINT3D() {}
      )

const StyleMap FunctionStyle::map {
    STYLEPAIR(BlinkingF),
    STYLEPAIR(Int)
};

const StyleMap Function3DStyle::map {
    STYLEPAIR(SmokeDF),
    STYLEPAIR(FastSmokeDF)
};

