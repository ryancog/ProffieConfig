#include "transitions.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/transitions.cpp
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

#include "proffieconstructs/range.h"
#include "proffieconstructs/utilfuncs.h"
#include "styles/bladestyle.h"
#include "styles/elements/colorstyles.h"
#include "styles/elements/functions.h"
#include "styles/elements/timefunctions.h"
#include "utility/time.h"

using namespace BladeStyles;

static constexpr uint8_t blastHump[32]{
    255, 255, 252, 247, 240, 232, 222, 211,
    199, 186, 173, 159, 145, 132, 119, 106,
    94,   82,  72,  62,  53,  45,  38,  32,
    26,   22,  18,  14,  11,   9,   7,   5
};

TransitionStyle::TransitionStyle(
        const char* osName, 
        const char* humanName, 
        const std::vector<Param*>& params,
        const BladeStyle* parent) :
    BladeStyle(osName, humanName, TRANSITION, params, parent) {}

class TransitionStyleImpl : public TransitionStyle {
public:
    ~TransitionStyleImpl() {
        if (dynamic_cast<AddBend*>(millisFunc)) {
            delete millisFunc;
        }
    }

    /**
     * Wrapper around the per-transition doRun() to do "Helper" stuff
     * This is done with templates and stuff in ProffieOS but this'll
     * do fine for ProffieConfig
     */
    virtual void run(StylePreview::Blade&) override;

    virtual void begin() override;
    virtual bool isDone() const override;
    virtual bool shouldRestart() const override;
    virtual uint32_t getStartMillis() const override;

protected:
    TransitionStyleImpl(const char* osName, const char* humanName, const std::vector<Param*>& params, const size_t millisIndex, const BladeStyle* parent) :
        TransitionStyle(osName, humanName, params, parent), millisIndex(millisIndex) {}

    /**
     * Used to do initial setup/calculations for each frame
     * This is kept hidden/protected because it's wrapped by run()
     */
    virtual void doRun(StylePreview::Blade&) = 0;

    // This is templated in ProffieOS, but most of the time it just takes
    // in an int. The only reason it seems to be templated is because colorcyle
    // makes a single call to this that uses a float.
    //
    // I don't feel like introducing templates into this, so we just always
    // use a double. Ugliness ensues.
    double update(double scale);

private:
    bool restart{false};

    // All transition functions have a time/millis
    // function, and they are needed in update(),
    // so I simply provide the index and the base class
    // handles the rest.
    TimeFunctionStyle* millisFunc{nullptr};
    const int32_t millisIndex;

    uint32_t startMillis{0};
    uint32_t length{0};
};

// Logic stays in the base, and we can (relatively) easily map the parameters
// and logic
template<class T>
class TransitionStyleWrap : public TransitionStyle {
public:
    TransitionStyleWrap(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent) :
        TransitionStyle(osName, humanName, params, parent), base(nullptr) {}

    virtual void begin() override { return base.begin(); }
    virtual bool isDone() const override { return base.isDone();  }
    virtual bool shouldRestart() const override { return base.shouldRestart(); }
    virtual uint32_t getStartMillis() const override { return base.getStartMillis(); }
    virtual ColorData getColor(const ColorData& colorA, const ColorData& colorB, int32_t led) override {
        return base.getColor(colorA, colorB, led);
    }

protected:
    T base;
};


void TransitionStyleImpl::run(StylePreview::Blade& blade) {
    // This type comparison, although it is evaluated at runtime,
    // will always be the same, because the param types are permanent
    // for the object's life, and are also basically baked into the 
    // specific class.

    if (millisIndex == -1) return doRun(blade);
    
    auto timeParam{getParamStyle(millisIndex)};

    // This could change based on given parameter.
    if (timeParam->getType() & TIMEFUNC) {
        if (dynamic_cast<AddBend*>(millisFunc)) delete millisFunc;
        millisFunc = const_cast<TimeFunctionStyle*>(static_cast<const TimeFunctionStyle*>(timeParam));
    } else { // If type is FUNCTION
        if (!dynamic_cast<AddBend*>(millisFunc)) millisFunc = new AddBend();
        millisFunc->setParam(0, const_cast<BladeStyle*>(timeParam));
    }

    millisFunc->run(blade);
    if (restart) {
        startMillis = Utility::getTimeMS();
        length = millisFunc->getInt(0);
        restart = false;
    }

    doRun(blade);
}
double TransitionStyleImpl::update(double scale) {
    if (isDone()) return scale;
    auto timeDelta{Utility::getTimeMS() - startMillis};

    if (timeDelta > length) {
        length = 0;
        return scale;
    }

    // Here the AddBend function bend() is used in ProffieOS
    // TimeFunctions are ugly, extremely ugly because of the templates
    // that are in use. I've done my best to recreate that system in a
    // "cleaner" way... at least something that can be done at runtime
    // and maintain the structure somewhat.
    return millisFunc->bend(timeDelta, length, scale);
}

void TransitionStyleImpl::begin() { restart = true; }

bool TransitionStyleImpl::isDone() const { return length == 0; }

bool TransitionStyleImpl::shouldRestart() const { return restart; }

uint32_t TransitionStyleImpl::getStartMillis() const { return startMillis; }

StyleGenerator TransitionStyle::get(const std::string& styleName) {
    const auto& map{getMap()};
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

const StyleMap& TransitionStyle::getMap() { return map; }

#define RUN(varname) \
    virtual void doRun(StylePreview::Blade& varname) override

#define GETCOLOR(colorAvarname, colorBvarname, ledvarname) \
    virtual ColorData getColor(const ColorData& colorAvarname, const ColorData& colorBvarname, const int32_t ledvarname) override

#define TRANS(osName, humanName, millisIndex, params, ...) \
    class osName : public TransitionStyleImpl { \
    public: \
        osName(const BladeStyle* parent) : TransitionStyleImpl(#osName, humanName, params, millisIndex, parent) {} \
        __VA_ARGS__ \
    }; 

#define RUNW(varname) \
    virtual void run(StylePreview::Blade& varname) override

#define TRANSW(osName, humanName, base, params, ...) \
    class osName : public TransitionStyleWrap<base> { \
    public: \
        osName(const BladeStyle* parent) : TransitionStyleWrap<base>(#osName, humanName, params, parent) {} \
        __VA_ARGS__ \
    };

// Usage: TrJoin<TR1, TR2, ...>
// TR1, TR2: TRANSITION
// return value: TRANSITION
// A little hard to explain, but all the specified
// transitions are run in parallel. Basically, we
// chain transitions like ((A TR1 B) TR2 B)
TRANS(TrJoin, "Join", -1,
        PARAMS(
            new StyleParam("Transition 1", TRANSITION, nullptr),
            new StyleParam("Transition 2", TRANSITION, nullptr),
            new StyleParam("Transition #", TRANSITION | VARIADIC, nullptr)
            ),

        RUN(blade) {
            // All params must be transition
            transitions.clear();
            auto numParams{getParams().size()};
            for (size_t i{0}; i < numParams; i++) {
                transitions.push_back(const_cast<TransitionStyle*>(static_cast<const TransitionStyle*>(getParamStyle(i))));
            }

            for (auto transition : transitions) {
                transition->run(blade);
            }
        }
        GETCOLOR(colorA, colorB, led) {
            ColorData res{transitions.front()->getColor(colorA, colorB, led)};
            // End before back, not end... basically end - 1
            for (auto it{std::next(transitions.begin())}; it != transitions.end(); it++) {
                res = (*it)->getColor(res, colorB, led);
            }

            return res;
        }

        virtual void begin() override {
            for (auto transition : transitions) transition->begin();
        }
        virtual bool isDone() const override {
            for (auto transition : transitions) if (!transition->isDone()) return false;
            return true;
        }

        private:
            std::vector<TransitionStyle*> transitions;
        )


// Usage: TrJoinR<TR1, TR2, ...>
// TR1, TR2: TRANSITION
// return value: TRANSITION
// Similar to TrJoin, but transitions are chained
// to the right instead of to the left. Like:
// (A TR2 (A TR1 B))
TRANS(TrJoinR, "Reverse Join", -1,
        PARAMS(
            new StyleParam("Transition 1", TRANSITION, nullptr),
            new StyleParam("Transition 2", TRANSITION, nullptr),
            new StyleParam("Transition #", TRANSITION | VARIADIC, nullptr)
            ),

        RUN(blade) {
            // All params must be transition
            transitions.clear();
            auto numParams{getParams().size()};
            for (size_t i{0}; i < numParams; i++) {
                transitions.push_back(const_cast<TransitionStyle*>(static_cast<const TransitionStyle*>(getParamStyle(i))));
            }

            for (auto transition : transitions) {
                transition->run(blade);
            }
        }
        GETCOLOR(colorA, colorB, led) {
            ColorData res{transitions.front()->getColor(colorA, colorB, led)};
            // End before back, not end... basically end - 1
            for (auto it{std::next(transitions.begin())}; it != transitions.end(); it++) {
                res = (*it)->getColor(colorA, res, led);
            }

            return res;
        }

        virtual void run(StylePreview::Blade& blade) override { doRun(blade); }
        virtual void begin() override {
            for (auto transition : transitions) transition->begin();
        }
        virtual bool isDone() const override {
            for (auto transition : transitions) if (!transition->isDone()) return false;
            return true;
        }

        private:
            std::vector<TransitionStyle*> transitions;
        )


// Usage: TrBlinkX<MILLIS_FUNCTION, N, WIDTH_FUNCTION>
// or: TrBlink<MILLIS, N, WIDTH>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// N: a number
// WIDTH_FUNCTION: FUNCTION, defaults to Int<16384>
// WIDTH: a number, defaults to 16384
// return value: TRANSITION
// Blinks A-B N times in MILLIS, based on WIDTH (0 ~ 32768)
// If WIDTH = 16384 A and B appear equally, lower decreases length of A, higher increases length of A
TRANS(TrBlinkX, "Blink", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new NumberParam("Number of Blinks"),
            new StyleParam("Distribution", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384)))
            ),
        RUN(blade) {
            auto distStyle{const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(2)))};

            distStyle->run(blade);
            blink = (static_cast<int32_t>(update(32768 * getParamNumber(1))) & 0x7FFF) < distStyle->getInt(0);
        }

        GETCOLOR(colorA, colorB, _) {
            return blink ? colorA : colorB;
        }

        private:
            bool blink;
     )

TRANSW(TrBlink, "Blink", TrBlinkX,
        PARAMS(
            new NumberParam("Time (ms)"),
            new NumberParam("Number of Blinks"),
            new NumberParam("Distribution", 16384)
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.setParam(1, getParamNumber(1));

            // this should be completely unnecessary, given Int is a default
            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(2))->setParam(0, getParamNumber(2));

            base.run(blade);
        }
     )

// Usage: TrBoingX<MILLIS_FUNCTION, N>
// or: TrBoing<MILLIS, N>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// N: a number
// return value: TRANSITION
// Similar to TrFade, but transitions back and forth between the two
// colors several times. (As specified by N). If N is 0, it's equal to
// TrFade. If N is 1 it transitions A-B-A-B, if N is 2, it is A-B-A-B-A-B,
// and so on.
TRANS(TrBoingX, "Boing", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new NumberParam("Number of Boings")
            ),
        RUN() {
            fadeAmount = update(16348 * ((getParamNumber(1) * 2) + 1));
            // Not sure what this is about... haven't investigated
            if (fadeAmount & 0x4000) {
                fadeAmount = 0x4000 - (fadeAmount & 0x3FFF);
            } else {
                fadeAmount &= 0x3FFF;
            }
        }
        GETCOLOR(colorA, colorB, _) { return mixColors(colorA, colorB, fadeAmount, 14); }

        private:
            uint32_t fadeAmount;
     )

TRANSW(TrBoing, "Boing", TrBoingX,
        PARAMS(
            new NumberParam("Time (ms)"),
            new NumberParam("Number of Boings")
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.setParam(1, getParamNumber(1));

            base.run(blade);
        }
     )

// Usage: TrCenterWipeX<POSITION_FUNCTION, MILLIS_FUNCTION>
// or: TrCenterWipe<POSITION, MILLIS>
// POSITION_FUNCTION & MILLIS_FUNCTION: FUNCTION
// POSITION: Int
// MILLIS: a number
// return value: TRANSITION
// In the beginning entire blade is color A, then color B
// starts at the POSTION and extends up and down the blade
// in the specified number of milliseconds.
TRANS(TrCenterWipeX, "Center Wipe", 0,
    PARAMS(
        new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
        new StyleParam("Center Position", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384)))
        ),
    RUN(blade) {
        auto posFunc{const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(1)))};
        posFunc->run(blade);

        uint32_t center{static_cast<uint32_t>((posFunc->getInt(0) * blade.numLeds) >> 7)};
        uint32_t fadeTop{static_cast<uint32_t>(update((256 * blade.numLeds) - center))};
        uint32_t fadeBottom{static_cast<uint32_t>(update(center))};
        uint32_t top{std::clamp<uint32_t>(center + fadeTop, center, 256 * blade.numLeds)};
        uint32_t bottom{std::clamp<uint32_t>(center - fadeBottom, 0, center)};
        range = Range(bottom, top);
    }
    GETCOLOR(colorA, colorB, led) {
        auto mix{(range & Range(led << 8, (led << 8) + 256)).size()};
        return mixColors(colorA, colorB, mix, 8);
    }

    private:
        Range range;
    )

TRANSW(TrCenterWipe, "Center Wipe", TrCenterWipeX,
        PARAMS(
            new NumberParam("Time (ms)"),
            new NumberParam("Center Position", 16384)
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            base.run(blade);
        }
        )

// TrCenterWipeSparkX = TrJoin<
//                          TrCenterWipeX<MILLIS, POSITION>,
//                          TrWaveX<
//                              COLOR, 
//                              // Why not mult w/ 4?
//                              Sum<MILLIS, MILLIS, MILLIS, MILLIS>, 
//                              Int<200>,
//                              // Mult * 2?
//                              Sum<MILLIS, MILLIS>, 
//                              POSITION
//                          >
//                      >;
TRANSW(TrCenterWipeSparkX, "Center Wipe With Spark", TrJoin,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr), 
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new StyleParam("Center Position", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384)))
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, TransitionStyle::get("TrCenterWipeX")(&base, PARAMVEC()));
            auto centerWipeTransition{const_cast<TransitionStyle*>(static_cast<const TransitionStyle*>(base.getParamStyle(0)))};

            centerWipeTransition->setParam(0, const_cast<BladeStyle*>(getParamStyle(1)));
            centerWipeTransition->setParam(1, const_cast<BladeStyle*>(getParamStyle(2)));

            if (!base.getParamStyle(1)) base.setParam(1, TransitionStyle::get("TrWaveX")(&base, PARAMVEC()));
            auto waveTransition{const_cast<TransitionStyle*>(static_cast<const TransitionStyle*>(base.getParamStyle(1)))};

            waveTransition->setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));

            if (!waveTransition->getParamStyle(1)) waveTransition->setParam(1, FunctionStyle::get("Sum")(waveTransition, PARAMVEC()));
            auto timeFunc{const_cast<BladeStyle*>(getParamStyle(1))};
            const_cast<BladeStyle*>(waveTransition->getParamStyle(1))->setParams(PARAMVEC(timeFunc, timeFunc, timeFunc, timeFunc));

            if (!waveTransition->getParamStyle(2)) waveTransition->setParam(2, FunctionStyle::get("Int")(waveTransition, PARAMVEC(200)));

            if (!waveTransition->getParamStyle(3)) waveTransition->setParam(3, FunctionStyle::get("Sum")(waveTransition, PARAMVEC()));
            const_cast<BladeStyle*>(waveTransition->getParamStyle(3))->setParams(PARAMVEC(timeFunc, timeFunc));

            waveTransition->setParam(4, const_cast<BladeStyle*>(getParamStyle(2)));

            base.run(blade);
        }
      )

TRANSW(TrCenterWipeSpark, "Center Wipe With Spark", TrCenterWipeSparkX,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr), 
            new NumberParam("Time (ms)"),
            new NumberParam("Center Position", 16384),
            ),
        RUNW(blade) {
            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));

            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(2))->setParam(0, getParamNumber(2));

            base.run(blade);
        }
      )

// Usage: TrCenterWipeInX<POSITION_FUNCTION, MILLIS_FUNCTION>
// or: TrCenterWipeIn<POSITION, MILLIS>
// POSITION_FUNCTION & MILLIS_FUNCTION: FUNCTION
// POSITION: Int
// MILLIS: a number
// return value: TRANSITION
// In the beginning entire blade is color A, then color B
// starts at the ends and moves toward POSITION
// in the specified number of milliseconds.
TRANS(TrCenterWipeInX, "Center Wipe In", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new StyleParam("Center Position", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384))),
            ),
        RUN(blade) {
            auto posFunc{const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(1)))};
            posFunc->run(blade);

            uint32_t center{static_cast<uint32_t>(posFunc->getInt(0) * blade.numLeds) >> 7};
            uint32_t fadeTop{static_cast<uint32_t>(update((256 * blade.numLeds) - center))};
            uint32_t fadeBottom{static_cast<uint32_t>(update(center))};
            uint32_t top{std::clamp<uint32_t>((256 * blade.numLeds) - fadeTop, center, 256 * blade.numLeds)};
            uint32_t bottom{std::clamp<uint32_t>(fadeBottom, 0, center)};

            range = Range(bottom, top);
        }
        GETCOLOR(colorA, colorB, led) {
            auto mix{(range & Range(led << 8, (led << 8) + 256)).size()};
            return mixColors(colorB, colorA, mix, 8);
        }
        private:
            Range range;
     )

 TRANSW(TrCenterWipeIn, "Center Wipe In", TrCenterWipeInX,
         PARAMS(
             new NumberParam("Time (ms)"),
             new NumberParam("Center Position", 16384)
             ),
         RUNW(blade) {
         if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
         const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

         if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));
         const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

         base.run(blade);
         }
       )

// TrCenterWipeInSparkX =   TrJoin<
//                              TrCenterWipeInX<MILLIS, POSITION>,
//                              TrJoin<
//                                  TrWaveX<
//                                      COLOR, 
//                                      MILLIS, 
//                                      Int<200>, 
//                                      Sum<MILLIS, MILLIS>,
//                                      Int<0>
//                                  >,
//                                  TrWaveX<
//                                      COLOR, 
//                                      MILLIS, 
//                                      Int<200>, 
//                                      Sum<MILLIS, MILLIS>,
//                                      Int<32768>
//                                  >
//                              >
//                          >
TRANSW(TrCenterWipeInSparkX, "Center Wipe In With Spark", TrJoin,
     PARAMS(
         new StyleParam("Spark Color", COLOR, nullptr), 
         new StyleParam("Time (ms)", TIMEFUNC | FUNCTION, nullptr),
         new StyleParam("Center Position", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384)))
         ),
     RUNW(blade) {
        if (!base.getParamStyle(0)) base.setParam(0, TransitionStyle::get("TrCenterWipeInX")(&base, PARAMVEC()));
        auto centerWipeTransition{STYLECAST(TransitionStyle, base.getParamStyle(0))};
        centerWipeTransition->setParam(0, const_cast<BladeStyle*>(getParamStyle(1)));
        centerWipeTransition->setParam(1, const_cast<BladeStyle*>(getParamStyle(2)));

        auto millisFunc{const_cast<BladeStyle*>(getParamStyle(1))};
        if (!base.getParamStyle(1)) base.setParam(1, TransitionStyle::get("TrJoin")(&base, PARAMVEC()));
        auto joinTransition{STYLECAST(TransitionStyle, base.getParamStyle(1))};

        if (!joinTransition->getParamStyle(0)) joinTransition->setParam(0, TransitionStyle::get("TrWaveX")(joinTransition, PARAMVEC()));
        if (!joinTransition->getParamStyle(1)) joinTransition->setParam(1, TransitionStyle::get("TrWaveX")(joinTransition, PARAMVEC()));
        auto waveTr1{STYLECAST(TransitionStyle, joinTransition->getParamStyle(0))};
        auto waveTr2{STYLECAST(TransitionStyle, joinTransition->getParamStyle(1))};

        if (!waveTr1->getParamStyle(2)) waveTr1->setParam(2, FunctionStyle::get("Int")(waveTr1, PARAMVEC(200)));
        if (!waveTr2->getParamStyle(2)) waveTr2->setParam(2, FunctionStyle::get("Int")(waveTr2, PARAMVEC(200)));

        if (!waveTr1->getParamStyle(3)) waveTr1->setParam(3, FunctionStyle::get("Sum")(waveTr1, PARAMVEC()));
        if (!waveTr2->getParamStyle(3)) waveTr2->setParam(3, FunctionStyle::get("Sum")(waveTr2, PARAMVEC()));

        if (!waveTr1->getParamStyle(4)) waveTr1->setParam(4, FunctionStyle::get("Int")(waveTr1, PARAMVEC(0)));
        if (!waveTr2->getParamStyle(4)) waveTr2->setParam(4, FunctionStyle::get("Int")(waveTr2, PARAMVEC(32768)));

        waveTr1->setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
        waveTr1->setParam(1, const_cast<BladeStyle*>(getParamStyle(1)));
        auto wave1Sum{STYLECAST(FunctionStyle, waveTr1->getParamStyle(3))};
        wave1Sum->setParams(PARAMVEC(millisFunc, millisFunc));

        waveTr2->setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
        waveTr2->setParam(1, const_cast<BladeStyle*>(getParamStyle(1)));
        auto wave2Sum{STYLECAST(FunctionStyle, waveTr2->getParamStyle(3))};
        wave2Sum->setParams(PARAMVEC(millisFunc, millisFunc));

        base.run(blade);
     }
   )

TRANSW(TrCenterWipeInSpark, "Center Wipe In With Spark", TrCenterWipeInSparkX,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr), 
            new NumberParam("Time (ms)"),
            new NumberParam("Center Position", 16384)
            ),
        RUNW(blade) {
            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));

            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(2))->setParam(0, getParamNumber(2));

            base.run(blade);
        }
      )

// Usage: TrColorCycle<MILLIS, START_RPM, END_RPM>
// MILLS:  number
// START_RPM: a number (defaults to 0)
// END_RPM: a number (defaults to 6000)
// return value: COLOR
// Tron-like transition.
// When he means tron-like transition, think Identity Disk ignition,
// immediately very erratic then stabilizes.
TRANS(TrColorCycleX, "Color Cycle", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new NumberParam("Start RPM", 0), 
            new NumberParam("End RPM", 6000),
            ),
        RUN(blade) {
            auto now{static_cast<uint32_t>(Utility::getTimeMicros())};
            auto delta{now - lastMicros};
            lastMicros = now;
            if (delta > 1000000) delta = 1;

            fade = update(1.f);

            float currentRpm{(getParamNumber(1) * (1 - fade)) + (getParamNumber(2) * fade)};
            float currentPercentage{100.f * fade};
            pos = ProffieUtils::fract(pos + ((delta / 60000000.0) * currentRpm));
            numLeds = blade.numLeds * 16384;
            start = pos * numLeds;

            if (currentPercentage == 100.f) {
                start = 0;
                end = numLeds;
            } else if (currentPercentage == 0.f) {
                start = 0;
                end = 0;
            } else {
                end = ProffieUtils::fract(pos + (currentPercentage / 100.f)) * numLeds;
            }
        }
        GETCOLOR(colorA, colorB, led) {
            Range ledRange(led * 16384, (led * 16384) + 16384);
            int32_t mix;

            if (start <= end) {
                mix = (Range(start, end) & ledRange).size();
            } else {
                mix = (Range(0, end) & ledRange).size() +
                    (Range(start, numLeds) & ledRange).size();
            }

            return mixColors(colorA, colorB, mix, 14);
        }

        private:
            float fade;
            float pos;
            uint32_t start;
            uint32_t end;
            uint32_t numLeds;
            uint32_t lastMicros{0};
    )

TRANSW(TrColorCycle, "Color Cycle", TrColorCycleX,
        PARAMS(
            new NumberParam("Time (ms)", 1000),
            new NumberParam("Start RPM", 0), 
            new NumberParam("End RPM", 6000),
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.setParam(1, getParamNumber(1));
            base.setParam(2, getParamNumber(2));

            base.run(blade);
        }
      )

// Usage: TrConcat<TRANSITION, INTERMEDIATE, TRANSITION, ...>
// OR:  TrConcat<TRANSITION, TRANSITION, ...>
// TRANSITION: TRANSITION
// INTERMEDIATE: COLOR
// return value: TRANSITION
// Concatenates any number of transitions.
// If an intermediate color is provided, we first transition to that color,
// then we transition away from it in the next transition. If no
// intermediate color is provided, the first and second transition will both
// transition from the same input colors. If for instance both the first and
// second transitions are TrFades, then there will be a jump in the middle
// as the transition will go back and start from the beginning. Using
// TimeReverseX on the second transition will avoid this, as the second
// transition will then run backwards.
TRANS(TrConcat, "Concatenate", -1,
        PARAMS(
            new StyleParam("Transition 1", TRANSITION, nullptr),
            new StyleParam("Transition or Color #", COLOR | TRANSITION, nullptr),
            ),
        RUN(blade) {
            if (runIndex == -1) return;

            auto numParams{getParams().size()};
            auto transitionIsDone{transition->isDone()};
            if (runIndex == static_cast<int32_t>(numParams - 1) && transitionIsDone) {
                runIndex = -1;
                return;
            }

            if (transitionIsDone) {
                auto nextStyle{getParamStyle(++runIndex)};
                if (nextStyle->getType() & COLOR) {
                    // We're hopping over a color
                    // Because of the precheck we know there must be another
                    // transition after it, no bounds check required.
                    prevColorStyle = nextColorStyle;
                    nextColorStyle = nullptr;
                    nextStyle = getParamStyle(++runIndex);
                    // Find next color, if there is one.
                    for (size_t i{static_cast<size_t>(runIndex + 1)}; i < numParams; i++) {
                        auto style{getParamStyle(i)};
                        if (style->getType() & COLOR) {
                            nextColorStyle = STYLECAST(ColorStyle, nextStyle);
                            break;
                        }
                    }
                }
                transition = STYLECAST(TransitionStyle, nextStyle);
                transition->begin();
            }

            if (prevColorStyle) prevColorStyle->run(blade);
            if (nextColorStyle) nextColorStyle->run(blade);
            transition->run(blade);
        }
        GETCOLOR(colorA, colorB, led) {
            return transition->getColor(
                    prevColorStyle ? prevColorStyle->getColor(led) : colorA, 
                    nextColorStyle ? nextColorStyle->getColor(led) : colorB, 
                    led
                    );
        }
        virtual void begin() override {
            runIndex = 0;
            transition = STYLECAST(TransitionStyle, getParamStyle(runIndex));
            prevColorStyle = nullptr;
            auto nextStyle{getParamStyle(runIndex + 1)};
            nextColorStyle = (nextStyle->getType() & COLOR) ? STYLECAST(ColorStyle, nextStyle) : nullptr;
            transition->begin();
        }
        virtual bool isDone() const override {
            return runIndex == -1;
        }
        virtual bool validateParams(std::string* err) const override {
            auto numParams{getParams().size()};

            for (size_t i{0}; i < numParams; i++) {
                if (!getParamStyle(i)) {
                    if (err) {
                        (*err) = "Empty parameter: ";
                        (*err) += getParam(i)->name;
                    }
                    return false;
                }
            }

            if (!(getParamStyle(0)->getType() & TRANSITION && getParamStyle(numParams - 1)->getType() & TRANSITION)) {
                if (err) (*err) = "Must begin and end with a transition!";
                return false;
            }

            bool prevWasColor{false};
            for (size_t i{0}; i < numParams; i++) {
                auto isColor{getParamStyle(i)->getType() & COLOR};

                if (isColor && prevWasColor) {
                    if (err) (*err) = "Cannot have two consecutive colors!";
                    return false;
                }

                prevWasColor = isColor;
            }

            return true;
        }
        
        private:
            int32_t runIndex{-1};
            ColorStyle* prevColorStyle{nullptr};
            ColorStyle* nextColorStyle{nullptr};
            TransitionStyle* transition{nullptr};
     )


// Usage: TrDelayX<MILLIS_FUNCTION>
// or: TrDelay<MILLIS>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// return value: TRANSITION
// Waits for the specified number of milliseconds, then transitions
// to second color. Menant to be used with TrConcat
TRANS(TrDelayX, "Delay", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION, nullptr),
            ),
        RUN() { update(0); }
        GETCOLOR(colorA, colorB, _) {
            return isDone() ? colorB : colorA;
        }
     )

TRANSW(TrDelay, "Delay", TrDelayX,
        PARAMS(
            new NumberParam("Time (ms)", 1000),
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));
            
            base.run(blade);
        }
      )

// Usage: TrDoEffectX<TRANSITION, EFFECT, WAVNUM, LOCATION_CLASS>
// or: TrDoEffects<TRANSITION, EFFECT, WAVNUM, LOCATION>
// TRANSITION: TRANSITION
// EFFECT: effect type
// WAVNUM, LOCATION: a number
// LOCATION_CLASS: INTEGER
// return value: TRANSITION
// Runs the specified TRANSITION and triggers EFFECT (unless the blade is
// off) Can specify WAV file to use for EFFECT with WAVNUM NOTE: 0 is first
// wav file, -1 is random wav LOCATION = -1 is random
TRANS(TrDoEffectX, "Transition With Effect", -1,
        PARAMS(
            new StyleParam("Transition", TRANSITION, nullptr),
            new StyleParam("Effect To Trigger", EFFECT, nullptr),
            new StyleParam("Audio File Number for Effect", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(-1))),
            new StyleParam("Location on Blade", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(-1))),
            ),
        RUN(blade) {
            transition = STYLECAST(TransitionStyle, getParamStyle(0));
            auto audioNumStyle{STYLECAST(FunctionStyle, getParamStyle(2))};
            auto locationStyle{STYLECAST(FunctionStyle, getParamStyle(3))};

            transition->run(blade);
            audioNumStyle->run(blade);
            locationStyle->run(blade);

            // location is a float here in ProffieOS, but there's no point
            // because it's just multiplied by 32768 and cast back to a uint16_t
            auto location{locationStyle->getInt(0)};
            if (location == -1) location = rand() % 32768;

            if (shouldBegin) {
                if (blade.isOn()) {
                    auto wavNum{audioNumStyle->getInt(0)};
                    auto effectStyle{STYLECAST(EffectStyle, getParamStyle(1))};
                    blade.doEffect(effectStyle->effect, location, wavNum);
                }
                shouldBegin = false;
            }

            if (!done && !blade.isOn()) done = true;
        }
        GETCOLOR(colorA, colorB, led) {
            if (done) return colorB;
            return transition->getColor(colorA, colorB, led);
        }
        virtual void begin() override {
            // Move the transition begin into run
            // to guarantee init?
            transition->begin();
            shouldBegin = true;
            done = false;
        }
        virtual bool isDone() const override {
            return done || transition->isDone();
        }

        private:
            TransitionStyle* transition;
            bool shouldBegin{false};
            bool done{true};
     )

TRANSW(TrDoEffect, "Transition With Effect", TrDoEffectX,
        PARAMS(
            new StyleParam("Transition", TRANSITION, nullptr),
            new StyleParam("Effect To Trigger", EFFECT, nullptr),
            new NumberParam("Audio File Number for Effect", -1),
            new NumberParam("Location on Blade", -1),
            ),
        RUNW(blade) {
            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
            base.setParam(1, const_cast<BladeStyle*>(getParamStyle(1)));
            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(getParamStyle(2))->setParam(0, getParamNumber(2));
            if (!base.getParamStyle(3)) base.setParam(3, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(getParamStyle(3))->setParam(0, getParamNumber(3));

            base.run(blade);
        }
      )

// Usage: TrDoEffectAlwaysX<TRANSITION, EFFECT, WAVNUM, LOCATION_CLASS>
// or: TrDoEffectsAlways<TRANSITION, EFFECT, WAVNUM, LOCATION>
// TrDoEffectAlways is the same as TrDoEffectX, but runs even if
// the blade is off.
TRANS(TrDoEffectAlwaysX, "Transition with Effect (Always)", -1,
        PARAMS(
            new StyleParam("Transition", TRANSITION, nullptr),
            new StyleParam("Effect To Trigger", EFFECT, nullptr),
            new StyleParam("Audio File Number for Effect", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(-1))),
            new StyleParam("Location on Blade", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(-1))),
            ),
        RUN(blade) {
            transition = STYLECAST(TransitionStyle, getParamStyle(0));
            auto audioNumStyle{STYLECAST(FunctionStyle, getParamStyle(2))};
            auto locationStyle{STYLECAST(FunctionStyle, getParamStyle(3))};

            auto location{locationStyle->getInt(0)};
            if (location == -1) location = rand() % 32768;
            if (shouldBegin) {
                auto wavNum{audioNumStyle->getInt(0)};
                auto effectStyle{STYLECAST(EffectStyle, getParamStyle(1))};
                blade.doEffect(effectStyle->effect, location, wavNum);
            }
        }
        GETCOLOR(colorA, colorB, led) { return transition->getColor(colorA, colorB, led); }
        virtual void begin() override { transition->begin(); shouldBegin = true; }
        virtual bool isDone() const override { return transition->isDone(); }

        private:
            bool shouldBegin{false};
            TransitionStyle* transition;
     )

TRANSW(TrDoEffectAlways, "Transition With Effect (Always)", TrDoEffectAlwaysX,
        PARAMS(
            new StyleParam("Transition", TRANSITION, nullptr),
            new StyleParam("Effect To Trigger", EFFECT, nullptr),
            new NumberParam("Audio File Number for Effect", -1),
            new NumberParam("Location on Blade", -1),
            ),
        RUNW(blade) {
            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
            base.setParam(1, const_cast<BladeStyle*>(getParamStyle(1)));
            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(getParamStyle(2))->setParam(0, getParamNumber(2));
            if (!base.getParamStyle(3)) base.setParam(3, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(getParamStyle(3))->setParam(0, getParamNumber(3));

            base.run(blade);
        }
      )

// Usage: TrExtendX<MILLIS_FUNCTION, TRANSITION>
// or: TrExtend<MILLIS, TRANSITION>
// MILLIS_FUNCTION: FUNCTION
// TRANSITION: TRANSITION
// MILLIS: a number
// return value: TRANSITION
// Runs the specified transition, then holds the
// last value for some additional time specified by
// MILLIS_FUNCTION.
TRANS(TrExtendX, "Freeze At End", -1,
        PARAMS(
            new StyleParam("Freeze Time (ms)", FUNCTION, nullptr),
            new StyleParam("Transition", TRANSITION, nullptr)
            ),
        RUN(blade) {
            transition = STYLECAST(TransitionStyle, getParamStyle(1));
            transition->run(blade);

            if (transition->isDone() && running) {
                if (extendStartTime == -1) extendStartTime = Utility::getTimeMS();
                auto now{Utility::getTimeMS()};
                auto freezeTime{STYLECAST(FunctionStyle, getParamStyle(0))->getInt(0)};
                if (static_cast<int32_t>(now - extendStartTime) > freezeTime) {
                    extendStartTime = -1;
                    running = false;
                }
            }
        }
        GETCOLOR(colorA, colorB, led) { return transition->getColor(colorA, colorB, led); }
        virtual void begin() override { transition->begin(); running = true; }
        virtual bool isDone() const override { return running; }

        private:
            int64_t extendStartTime{-1};
            bool running{false};
            TransitionStyle* transition;
     )

TRANSW(TrExtend, "Freeze At End", TrExtendX,
        PARAMS(
            new NumberParam("Freeze Time (ms)", 1000),
            new StyleParam("Transition", TRANSITION, nullptr)
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.setParam(1, const_cast<BladeStyle*>(getParamStyle(1)));

            base.run(blade);
            }
      )

// Usage: TrFadeX<MILLIS_FUNCTION>
// or: TrFade<MILLIS>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// return value: TRANSITION
// Linear fading between two colors in specified number of milliseconds.
TRANS(TrFadeX, "Fade", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            ),
        RUN() {
            fade = update(16384);
        }
        GETCOLOR(colorA, colorB, _) { return mixColors(colorA, colorB, fade, 14); }

        private:
            uint32_t fade;
     )

TRANSW(TrFade, "Fade", TrFadeX,
        PARAMS(
            new NumberParam("Time (ms)", 1000),
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.run(blade);
        }
      )

// Usage: TrSmoothFadeX<MILLIS_FUNCTION>
// or: TrSmoothFade<MILLIS>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// return value: TRANSITION
// Similar to TrFade, but uses a cubic fading function
// so fading starts slow, speeds up in the middle, then
// slows down at the end.
TRANS(TrSmoothFadeX, "Smooth Fade", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            ),
        RUN() {
            auto x{static_cast<int32_t>(update(16384))};
            fade = (((x * x) >> 14) * ((3 << 14) - x)) >> 16;
        }
        GETCOLOR(colorA, colorB, _) { return mixColors(colorA, colorB, fade, 14); }

        private:
            uint32_t fade;
     )

TRANSW(TrSmoothFade, "Smooth Fade", TrSmoothFadeX,
        PARAMS(
            new NumberParam("Time (ms)", 1000),
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.run(blade);
        }
      )

// Usage: TrInstant
// return value: TRANSITION
// Instant transition.
TRANS(TrInstant, "Instant", -1,
        PARAMS(),
        RUN() {}
        GETCOLOR(_, colorB,) { return colorB; }
        virtual bool isDone() const override { return true; }
        virtual void begin() override {}
        )

// Usage: TrLoop<TRANSITION>
// TRANSITION: TRANSITION
// Return Value: TRANSITION
// Runs the specified transition in a loop forever.
TRANS(TrLoop, "Loop Forever", -1,
        PARAMS(
            new StyleParam("Transition", TRANSITION, nullptr),
            ),
        RUN(blade) {
            transition = STYLECAST(TransitionStyle, getParamStyle(0));
            if (transition->isDone()) transition->begin();
            transition->run(blade);
        }
        GETCOLOR(colorA, colorB, led) { return transition->getColor(colorA, colorB, led); }
        virtual bool isDone() const override { return false; }
        virtual void begin() override {}

        private:
            TransitionStyle* transition;
     )

// Usage: TrLoopNX<N_FUNCTION, TRANSITION>
// or: TrLoopN<N, TRANSITION>
// N_FUNCTION: FUNCTION (number of Loops)
// N: a number (Loops)
// TRANSITION: TRANSITION
// Return Value: TRANSITION
// Runs the specified transition N times.
TRANS(TrLoopNX, "Loop", -1,
        PARAMS(
            new StyleParam("Number of Loops", FUNCTION, nullptr),
            new StyleParam("Transition", TRANSITION, nullptr)
            ),
        RUN(blade) {
            auto numLoopsStyle{STYLECAST(FunctionStyle, getParamStyle(0))};
            transition = STYLECAST(TransitionStyle, getParamStyle(1));

            numLoopsStyle->run(blade);
            if (loops < 0) loops = numLoopsStyle->getInt(0) + 1;
            if (loops > 0 && transition->isDone()) {
                if (loops > 1) transition->begin();
                loops--;
            }
            transition->run(blade);
        }
        GETCOLOR(colorA, colorB, led) { return transition->getColor(colorA, colorB, led); }
        virtual bool isDone() const override { return loops == 0; }
        virtual void begin() override { transition->begin(); loops = -1; }

        private:
            TransitionStyle* transition;
            int32_t loops{0};
      )

TRANSW(TrLoopN, "Loop", TrLoopNX,
        PARAMS(
            new NumberParam("Number of Loops", 1),
            new StyleParam("Transition", TRANSITION, nullptr)
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(1)));

            base.run(blade);
        }
      )

// Usage: TrLoopUntil<PULSE, TRANSITION, OUT>
// TRANSITION, OUT: TRANSITION
// PULSE: FUNCTION (pulse)
// Return Value: TRANSITION
// Runs the specified transition until the pulse occurs.
// When the pulse occurs, the loop continues, but OUT is used to
// transition away from it, and when OUT is done, the transition is done.
TRANS(TrLoopUntil, "Loop Until", -1,
        PARAMS(
            new StyleParam("End Pulse", FUNCTION, nullptr),
            new StyleParam("Transition", TRANSITION, nullptr),
            new StyleParam("End Transition", TRANSITION, nullptr),
            ),
        RUN(blade) {
            transition = STYLECAST(TransitionStyle, getParamStyle(1));
            outTransition = STYLECAST(TransitionStyle, getParamStyle(2));
            auto endPulseStyle{STYLECAST(FunctionStyle, getParamStyle(0))};

            endPulseStyle->run(blade);
            if (transition->isDone()) transition->begin();
            transition->run(blade);

            if (!pulsed && endPulseStyle->getInt(0)) {
                outTransition->begin();
                pulsed = true;
            }

            if (pulsed) outTransition->run(blade);
        }
        GETCOLOR(colorA, colorB, led) {
            if (pulsed) return outTransition->getColor(transition->getColor(colorA, colorA, led), colorB, led);
            return transition->getColor(colorA, colorA, led);
        }
        virtual bool isDone() const override {
            return pulsed && outTransition->isDone();
        }
        virtual void begin() override {
            transition->begin();
            pulsed = false;
        }

        private:
            bool pulsed{false};
            TransitionStyle* outTransition;
            TransitionStyle* transition;
     )

// Usage: TrRandom<TR1, TR2, ...>
// TR1, TR2: TRANSITION
// return value: TRANSITION
// Each time a new transition is started, a random
// transition is picked from the specified list of
// transitions.
TRANS(TrRandom, "Random", -1,
        PARAMS(
            new StyleParam("Transition 1", TRANSITION, nullptr),
            new StyleParam("Transition #", TRANSITION | VARIADIC, nullptr),
            ),
        RUN(blade) {
            if (shouldBegin) {
                shouldBegin = false;
                // Last variadic will be nullptr
                auto numParams{getParams().size() - 1};
                auto selectIndex{rand() % numParams};
                selected = STYLECAST(TransitionStyle, getParamStyle(selectIndex));
                selected->begin();
            }
            if (selected) selected->run(blade);
        }
        GETCOLOR(colorA, colorB, led) {
            return selected->getColor(colorA, colorB, led);
        }
        virtual bool isDone() const override {
            if (shouldBegin) return false;
            if (!selected) return true;
            return selected->isDone();
        }
        virtual void begin() override {
            shouldBegin = true;
        }

        private:
            bool shouldBegin{false};
            TransitionStyle* selected{nullptr};
     )

// Usage: TrSelect<SELECTION, TR1, TR2, ...>
// SELECTION: FUNCTION
// TR1, TR2: TRANSITION
// return value: TRANSITION
// transition option is picked from the specified list of
// transitions based on Int<>
// with Int<0> representing first transition
TRANS(TrSelect, "Selection", -1,
        PARAMS(
            new StyleParam("Selection", FUNCTION, nullptr),
            new StyleParam("Transition 1", TRANSITION, nullptr),
            new StyleParam("Transition #", TRANSITION | VARIADIC, nullptr),
            ),
        RUN(blade) {
            auto selectStyle{STYLECAST(FunctionStyle, getParamStyle(0))};
            selectStyle->run(blade);

            if (shouldBegin) {
                shouldBegin = false;
                // Last variadic will be nullptr, remove it and selection
                auto numParams{getParams().size() - 2};
                auto selectIndex{selectStyle->getInt(0) % numParams};
                selected = STYLECAST(TransitionStyle, getParamStyle(selectIndex + 1));
                selected->begin();
            }
            if (selected) selected->run(blade);
        }
        GETCOLOR(colorA, colorB, led) {
            return selected->getColor(colorA, colorB, led);
        }
        virtual bool isDone() const override {
            if (shouldBegin) return false;
            if (!selected) return true;
            return selected->isDone();
        }
        virtual void begin() override {
            shouldBegin = true;
        }

        private:
            bool shouldBegin{false};
            TransitionStyle* selected{nullptr};
     )

// Usage: TrSequence<TR1, TR2, ...>
// TR1, TR2: TRANSITION
// return value: TRANSITION
// Each time a new transition is started, a transition is selected
// sequentially, such that the first time a transition is started
// TR1 will be selected, then TR2, etc.
// When the end of the sequence is reached, the selection
// will wrap back around to TR1.
TRANS(TrSequence, "Sequence", -1,
        PARAMS(
            new StyleParam("Transition 1", TRANSITION, nullptr),
            new StyleParam("Transition #", TRANSITION | VARIADIC, nullptr),
            ),
        RUN(blade) {
            if (shouldBegin) {
                shouldBegin = false;
                // Last variadic will be nullptr, remove it
                auto numParams{getParams().size() - 1};
                index = (index + 1) % numParams;
                auto selectIndex{index};
                selected = STYLECAST(TransitionStyle, getParamStyle(selectIndex + 1));
                selected->begin();
            }
            if (selected) selected->run(blade);
        }
        GETCOLOR(colorA, colorB, led) {
            return selected->getColor(colorA, colorB, led);
        }
        virtual bool isDone() const override {
            if (shouldBegin) return false;
            if (!selected) return true;
            return selected->isDone();
        }
        virtual void begin() override {
            shouldBegin = true;
        }

        private:
            bool shouldBegin{false};
            int32_t index{-1};
            TransitionStyle* selected{nullptr};
     )

// TrWave is implements a wave traveling out from a specified point.
// It's based on the Blast effect and is meant to look like a ripple
// starting at a point on the blade. Unlike other transitions, this effect
// starts and ends at the same color, and the wave is drawn using COLOR
// instead of the start/end colors like most transitions do. It's intended
// to be used with TransitionLoopL or TransitionEffectL, which take
// transitions that start and begin with the same color.
TRANS(TrWaveX, "Wave", 1,
        PARAMS(
            new StyleParam("Color", COLOR, nullptr),
            new StyleParam("Fadeout Time (ms)", FUNCTION | TIMEFUNC, FunctionStyle::get("Int")(this, PARAMVEC(200))),
            new StyleParam("Wave Size", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(100))),
            new StyleParam("Wave Time (ms)", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(400))),
            new StyleParam("Wave Position", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384)))
            ),
        RUN(blade) {
            auto waveSizeStyle{STYLECAST(FunctionStyle, getParamStyle(2))};
            auto wavePosStyle{STYLECAST(FunctionStyle, getParamStyle(4))};
            auto waveTimeStyle{STYLECAST(FunctionStyle, getParamStyle(3))};
            colorStyle = STYLECAST(ColorStyle, getParamStyle(0));

            waveSizeStyle->run(blade);
            wavePosStyle->run(blade);
            waveTimeStyle->run(blade);
            colorStyle->run(blade);

            if (shouldRestart()) {
                center = wavePosStyle->getInt(0);
                size = waveSizeStyle->getInt(0);
            }

            mix = 32768 - update(32768);
            numLeds = blade.numLeds;
            auto waveTime{waveTimeStyle->getInt(0)};
            offset = ((Utility::getTimeMS() - getStartMillis()) * 32768) / /* prevent div by 0 */ (waveTime ? waveTime : 1);
        }
        GETCOLOR(colorA, _, led) {
            auto dist{std::abs(center - ((led * 32768) / numLeds))};
            auto index{(std::abs(dist - offset) * size) >> 15};
            auto mixTmp{0};
            if (index < 32) mixTmp = (blastHump[index] * mix) >> 8;
            return mixColors(colorA, colorStyle->getColor(led), mixTmp, 15);
        }

        private:

            ColorStyle* colorStyle;
            int32_t center;
            int32_t size;
            int32_t mix;
            int32_t numLeds;
            int32_t offset;
     )

// TrSparkX generates a wave without Fade over the length of the blade from
// SPARK_CENTER. Unlike other transitions, this effect starts and ends
// at the same color, and the wave is drawn using COLOR instead of the
// start/end colors like most transitions do. It's intended to be used with
// TransitionLoopL or TransitionEffectL, which take transitions that start
// and begin with the same color.
TRANS(TrSparkX, "Spark", 2,
        PARAMS(
            new StyleParam("Color", COLOR, nullptr),
            new StyleParam("Size", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(100))),
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, FunctionStyle::get("Int")(this, PARAMVEC(400))),
            new StyleParam("Position", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(16384)))
            ),
        RUN(blade) {
            auto sizeStyle{STYLECAST(FunctionStyle, getParamStyle(1))};
            auto posStyle{STYLECAST(FunctionStyle, getParamStyle(3))};
            colorStyle = STYLECAST(ColorStyle, getParamStyle(0));

            sizeStyle->run(blade);
            posStyle->run(blade);
            colorStyle->run(blade);

            if (shouldRestart()) {
                center = posStyle->getInt(0);
                size = sizeStyle->getInt(0);
            }

            numLeds = blade.numLeds;
            offset = update(32768);
        }
        GETCOLOR(colorA, _, led) {
            auto dist{std::abs(center - ((led * 32768) / numLeds))};
            auto index{(std::abs(dist - offset) * size) >> 15};
            auto mix{0};
            if (index < 32) mix = blastHump[index] << 7;
            return mixColors(colorA, colorStyle->getColor(led), mix, 15);
        }

        private:
            int32_t center;
            int32_t size;
            int32_t numLeds;
            int32_t offset;
            ColorStyle* colorStyle;
        )

// Usage: TrWipeX<MILLIS_FUNCTION>
// or: TrWipe<MILLIS>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// return value: TRANSITION
// Similar to saber ignition. In the beginning
// entire blade is color A, then color B starts at the base
// and extends up to the tip of the blade in the specified
// number of milliseconds.
TRANS(TrWipeX, "Wipe", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr)
            ),
        RUN(blade) {
            fade = update(256 * blade.numLeds);
        }
        GETCOLOR(colorA, colorB, led) {
            auto mix{(Range(0, fade) & Range(led << 8, (led << 8) + 256)).size()};
            return mixColors(colorA, colorB, mix, 8);
        }

        private:
            int32_t fade;
     )

TRANSW(TrWipe, "Wipe", TrWipeX,
        PARAMS(
            new NumberParam("Time (ms)", 1000)
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.run(blade);
        }
      )

// Usage: TrWipeInX<MILLIS_FUNCTION>
// or: TrWipeIn<MILLIS>
// MILLIS_FUNCTION: FUNCTION
// MILLIS: a number
// return value: TRANSITION
// Like TrWipe, but from tip to base.
TRANS(TrWipeInX, "Wipe In", 0,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr)
            ),
        RUN(blade) {
            fade = Range(
                    (256 * blade.numLeds) - update(256 * blade.numLeds),
                    blade.numLeds * 256
                    );
        }
        GETCOLOR(colorA, colorB, led) {
            auto mix{(fade & Range(led << 8, (led << 8) + 256)).size()};
            return mixColors(colorA, colorB, mix, 8);
        }

        private:
            Range fade;
     )

TRANSW(TrWipeIn, "Wipe In", TrWipeInX,
        PARAMS(
            new NumberParam("Time (ms)", 1000)
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.run(blade);
        }
      )

// Usage: TrWipeSparkTip<SPARK_COLOR, MILLIS, SIZE>
// SPARK_COLOR = COLOR
// MILLIS = a number
// SIZE = a number
// return value: TRANSITION
// Same as TrWipe, but adds a "spark" tip to the
// leading edge of the wipe color.
//
// TrJoin<
//      TrWipeX<MILLIS>,
//      TrSparkX<
//          SPARK_COLOR,
//          SIZE,
//          MILLIS,
//          Int<0>
//      >
// >
TRANSW(TrWipeSparkTipX, "Wipe With Tip Spark", TrJoin,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr),
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new StyleParam("Spark Size", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(400))),
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, TransitionStyle::get("TrWipeX")(&base, PARAMVEC()));
            if (!base.getParamStyle(1)) base.setParam(1, TransitionStyle::get("TrSparkX")(&base, PARAMVEC()));

            auto timeStyle{const_cast<BladeStyle*>(getParamStyle(1))};

            auto wipeStyle{STYLECAST(TransitionStyle, base.getParamStyle(0))};
            wipeStyle->setParam(0, timeStyle);

            auto sparkStyle{STYLECAST(TransitionStyle, base.getParamStyle(1))};
            if (!sparkStyle->getParamStyle(3)) sparkStyle->setParam(3, FunctionStyle::get("Int")(sparkStyle, PARAMVEC(0)));
            sparkStyle->setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
            sparkStyle->setParam(1, const_cast<BladeStyle*>(getParamStyle(2)));
            sparkStyle->setParam(2, timeStyle);

            base.run(blade);
        }
      )

TRANSW(TrWipeSparkTip, "Wipe With Tip Spark", TrWipeSparkTipX,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr),
            new NumberParam("Time (ms)", 1000), 
            new NumberParam("Spark Size", 400)
            ),
        RUNW(blade) {
            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));

            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(2))->setParam(0, getParamNumber(2));

            base.run(blade);
        }
      )

// Usage: TrWipeInSparkTip<SPARK_COLOR, MILLIS, SIZE>
// SPARK_COLOR = COLOR
// MILLIS = a number
// SIZE = a number
// return value: TRANSITION
// Like TrWipeSparkTip, but from tip to base.
// TrJoin<
//      TrWipeInX<MILLIS>,
//      TrSparkX<
//          SPARK_COLOR,
//          SIZE,
//          MILLIS,
//          Int<32768>
//      >
// >
TRANSW(TrWipeInSparkTipX, "Wipe In With Tip Spark", TrJoin,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr),
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, nullptr),
            new StyleParam("Spark Size", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(400))),
            ),
        RUNW(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, TransitionStyle::get("TrWipeX")(&base, PARAMVEC()));
            if (!base.getParamStyle(1)) base.setParam(1, TransitionStyle::get("TrSparkX")(&base, PARAMVEC()));

            auto timeStyle{const_cast<BladeStyle*>(getParamStyle(1))};

            auto wipeStyle{STYLECAST(TransitionStyle, base.getParamStyle(0))};
            wipeStyle->setParam(0, timeStyle);

            auto sparkStyle{STYLECAST(TransitionStyle, base.getParamStyle(1))};
            if (!sparkStyle->getParamStyle(3)) sparkStyle->setParam(3, FunctionStyle::get("Int")(sparkStyle, PARAMVEC(32768)));
            sparkStyle->setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
            sparkStyle->setParam(1, const_cast<BladeStyle*>(getParamStyle(2)));
            sparkStyle->setParam(2, timeStyle);

            base.run(blade);
        }
    )

TRANSW(TrWipeInSparkTip, "Wipe In With Tip Spark", TrWipeInSparkTipX,
        PARAMS(
            new StyleParam("Spark Color", COLOR, nullptr),
            new NumberParam("Time (ms)", 1000), 
            new NumberParam("Spark Size", 400)
            ),
        RUNW(blade) {
            base.setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));

            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            if (!base.getParamStyle(2)) base.setParam(2, FunctionStyle::get("Int")(&base, PARAMVEC()));
            const_cast<BladeStyle*>(base.getParamStyle(2))->setParam(0, getParamNumber(2));

            base.run(blade);
        }
    )


const StyleMap TransitionStyle::map {
    STYLEPAIR(TrJoin), 
    STYLEPAIR(TrJoinR), 
    STYLEPAIR(TrBlinkX), 
    STYLEPAIR(TrBlink), 
    STYLEPAIR(TrBoingX), 
    STYLEPAIR(TrBoing), 
    STYLEPAIR(TrCenterWipeX), 
    STYLEPAIR(TrCenterWipe), 
    STYLEPAIR(TrCenterWipeSparkX), 
    STYLEPAIR(TrCenterWipeSpark), 
    STYLEPAIR(TrCenterWipeInX), 
    STYLEPAIR(TrCenterWipeIn), 
    STYLEPAIR(TrCenterWipeInSparkX), 
    STYLEPAIR(TrCenterWipeInSpark), 
    STYLEPAIR(TrColorCycleX), 
    STYLEPAIR(TrColorCycle), 
    STYLEPAIR(TrConcat), 
    STYLEPAIR(TrDelayX), 
    STYLEPAIR(TrDelay), 
    STYLEPAIR(TrDoEffectX), 
    STYLEPAIR(TrDoEffect), 
    STYLEPAIR(TrDoEffectAlwaysX), 
    STYLEPAIR(TrDoEffectAlways), 
    STYLEPAIR(TrExtendX), 
    STYLEPAIR(TrExtend), 
    STYLEPAIR(TrFadeX), 
    STYLEPAIR(TrFade), 
    STYLEPAIR(TrSmoothFadeX), 
    STYLEPAIR(TrSmoothFade), 
    STYLEPAIR(TrInstant), 
    STYLEPAIR(TrLoop), 
    STYLEPAIR(TrLoopNX), 
    STYLEPAIR(TrLoopN), 
    STYLEPAIR(TrLoopUntil), 
    STYLEPAIR(TrRandom), 
    STYLEPAIR(TrSelect), 
    STYLEPAIR(TrSequence), 
    STYLEPAIR(TrWaveX), 
    STYLEPAIR(TrSparkX), 
    STYLEPAIR(TrWipeX), 
    STYLEPAIR(TrWipe), 
    STYLEPAIR(TrWipeInX), 
    STYLEPAIR(TrWipeIn), 
    STYLEPAIR(TrWipeSparkTipX), 
    STYLEPAIR(TrWipeSparkTip), 
    STYLEPAIR(TrWipeInSparkTipX), 
    STYLEPAIR(TrWipeInSparkTip), 
};
