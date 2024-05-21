#include "timefunctions.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/timefunctions.cpp
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
#include "styles/elements/functions.h"
#include <cmath>

using namespace BladeStyles;

TimeFunctionStyle::TimeFunctionStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent) :
    FunctionStyle(osName, humanName, params, parent, TIMEFUNC) {}

const StyleMap& TimeFunctionStyle::getMap() { return map; }

StyleGenerator TimeFunctionStyle::get(const std::string& styleName) { 
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

#define RUN(varname) \
    virtual void run(StylePreview::Blade& varname) override

#define BEND(timevarname, lengthvarname, scalevarname) \
    virtual double bend(uint32_t timevarname, uint32_t lengthvarname, double scalevarname) override

#define GETINT(varname) \
    virtual int32_t getInt(int32_t varname) override

#define TIMEFUNC(osName, humanName, params, ...) \
    class osName : public TimeFunctionStyle { \
    public: \
        osName(const BladeStyle* parent) : TimeFunctionStyle(#osName, humanName, params, parent) {} \
        __VA_ARGS__ \
    }; 

template<class T>
class TimeFunctionStyleW : public TimeFunctionStyle {
public:
    TimeFunctionStyleW(
            const char* osName, 
            const char* humanName, 
            const std::vector<Param*>& params, 
            const BladeStyle* parent) :
        TimeFunctionStyle(osName, humanName, params, parent), base(nullptr) {}

    virtual double bend(uint32_t time, uint32_t length, double scale) override { return base.bend(time, length, scale); }
    virtual int32_t getInt(int32_t led) override { return base.getInt(led); }

protected:
    T base;
};

#define TIMEFUNCW(osName, humanName, base, params, ...) \
    class osName : public TimeFunctionStyleW<base> { \
    public: \
        osName(const BladeStyle* parent) : TimeFunctionStyleW(#osName, humanName, params, parent) {} \
        __VA_ARGS__ \
    };


TIMEFUNC(BendTimePowX, "Exponential Time Bend",
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, FunctionStyle::get("Int")(this, PARAMVEC(1000))),
            new StyleParam("Power", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(65536)))
            ),
        RUN(blade) {
            auto timeParam{getParamStyle(0)};
            if (timeParam->getType() & FUNCTION) {
                if (!dynamic_cast<AddBend*>(millisStyle)) {
                    millisStyle = new AddBend();
                }
                millisStyle->setParam(0, const_cast<BladeStyle*>(timeParam));
            } else {
                if (millisStyle) delete millisStyle;
                millisStyle = const_cast<TimeFunctionStyle*>(static_cast<const TimeFunctionStyle*>(timeParam));
            }

            powerStyle = const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(1)));

            millisStyle->run(blade);
            powerStyle->run(blade);
        }
        BEND(time, length, scale) {
            return scale * powf(millisStyle->bend(time, length, 1.f), exponent);
        }
        GETINT() {
            exponent = powerStyle->getInt(0) / 32768.f;
            return millisStyle->getInt(0);
        }

        ~BendTimePowX() { 
            if (dynamic_cast<AddBend*>(millisStyle)) {
                delete millisStyle;
            }
        }

        private:
            // Non-null could be problematic on init
            TimeFunctionStyle* millisStyle{nullptr};
            FunctionStyle* powerStyle;
            float exponent{0.f};
        )

TIMEFUNC(ReverseTimeX, "Reverse Time",
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, FunctionStyle::get("Int")(this, PARAMVEC(1000)))
            ),
        RUN(blade) {
            auto timeParam{getParamStyle(0)};
            if (timeParam->getType() & FUNCTION) {
                if (!dynamic_cast<AddBend*>(millisStyle)) {
                    millisStyle = new AddBend();
                }
                millisStyle->setParam(0, const_cast<BladeStyle*>(timeParam));
            } else {
                if (millisStyle) delete millisStyle;
                millisStyle = const_cast<TimeFunctionStyle*>(static_cast<const TimeFunctionStyle*>(timeParam));
            }

            millisStyle->run(blade);
        }
        BEND(time, length, scale) {
            return scale - millisStyle->bend(time, length, scale);
        }
        GETINT(led) {
            return millisStyle->getInt(led);
        }

        private:
            TimeFunctionStyle* millisStyle{nullptr};
        )

TIMEFUNCW(BendTimePowInvX, "Inverse Exponential Time Bend", ReverseTimeX,
        PARAMS(
            new StyleParam("Time (ms)", FUNCTION | TIMEFUNC, FunctionStyle::get("Int")(this, PARAMVEC(1000))),
            new StyleParam("Power", FUNCTION, FunctionStyle::get("Int")(this, PARAMVEC(65536)))
            ),
        RUN(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, TimeFunctionStyle::get("BendTimePowX")(&base, PARAMVEC()));
            auto timeBendStyle{STYLECAST(TimeFunctionStyle, base.getParamStyle(0))};

            if (timeBendStyle->getParamStyle(0)) timeBendStyle->setParam(0, TimeFunctionStyle::get("ReverseTimeX")(timeBendStyle, PARAMVEC()));
            const_cast<BladeStyle*>(timeBendStyle->getParamStyle(0))->setParam(0, const_cast<BladeStyle*>(getParamStyle(0)));
            timeBendStyle->setParam(1, const_cast<BladeStyle*>(getParamStyle(1)));
            
            base.run(blade);
        }
        )

TIMEFUNCW(BendTimePow, "Inverse Time Bend", BendTimePowX,
        PARAMS(
            new NumberParam("Millis", 1000),
            new NumberParam("Power", 65536)
            ),
        RUN(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));

            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            base.run(blade);
        }
        )

TIMEFUNCW(BendTimePowInv, "Inverse Exponential Time Bend", BendTimePowInvX,
        PARAMS(
            new NumberParam("Millis", 1000),
            new NumberParam("Power", 65536)
            ),
        RUN(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));
            if (!base.getParamStyle(1)) base.setParam(1, FunctionStyle::get("Int")(&base, PARAMVEC()));

            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));
            const_cast<BladeStyle*>(base.getParamStyle(1))->setParam(0, getParamNumber(1));

            base.run(blade);
        }
        )

TIMEFUNCW(ReverseTime, "Reverse Time", ReverseTimeX,
        PARAMS(
            new NumberParam("Millis", 1000)
            ),
        RUN(blade) {
            if (!base.getParamStyle(0)) base.setParam(0, FunctionStyle::get("Int")(&base, PARAMVEC()));

            const_cast<BladeStyle*>(base.getParamStyle(0))->setParam(0, getParamNumber(0));

            base.run(blade);
        }
        )

const StyleMap TimeFunctionStyle::map {
    STYLEPAIR(BendTimePowX),
    STYLEPAIR(BendTimePowInvX),
    STYLEPAIR(ReverseTimeX),
    STYLEPAIR(BendTimePow),
    STYLEPAIR(BendTimePowInv),
    STYLEPAIR(ReverseTime),
};


