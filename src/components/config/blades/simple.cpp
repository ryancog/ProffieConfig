#include "simple.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/blades/simple.cpp
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

#include "config/strings.hpp"
#include "data/number.hpp"

using namespace config::blades;

namespace {

constexpr std::string_view LED1_STR{"LED1"};
constexpr std::string_view LED2_STR{"LED2"};
constexpr std::string_view LED3_STR{"LED3"};
constexpr std::string_view LED4_STR{"LED4"};

constexpr std::string_view PROFILE_STR{"Profile"};
constexpr std::string_view POWERPIN_STR{"PowerPin"};
constexpr std::string_view RESISTANCE_STR{"Resistance"};

} // namespace

Simple::Simple(data::Node *parent) :
    data::Node(parent),
    led1_(*this),
    led2_(*this),
    led3_(*this),
    led4_(*this) {
    CreationScope(*this, true);
}

Simple::~Simple() = default;

bool Simple::enumerate(const EnumFunc& func) {
    if (func(led1_, strID(LED1_STR), LED1_STR)) return true;
    if (func(led2_, strID(LED2_STR), LED2_STR)) return true;
    if (func(led3_, strID(LED3_STR), LED3_STR)) return true;
    if (func(led4_, strID(LED4_STR), LED4_STR)) return true;
    return false;
}

data::Model *Simple::find(uint64 id) {
    if (id == strID(LED1_STR)) return &led1_;
    if (id == strID(LED2_STR)) return &led2_;
    if (id == strID(LED3_STR)) return &led3_;
    if (id == strID(LED4_STR)) return &led4_;
    return nullptr;
}

Simple::LED::LED(Simple& simple) :
    data::Node(&simple),
    profile_(this),
    powerPin_(this),
    resistance_(this) {
    CreationScope createScope(*this);

    profile_.responder().onChoice_ = [](const data::Choice::ROContext& ctxt) {
        auto& led{*ctxt.model().parent<LED>()};
        data::String::Context{led.powerPin_}.enable(
            ctxt.idx() != eLED_None
        );

        data::Integer::Context{led.resistance_}.enable(
            ctxt.idx() >= eLED_Use_Resistance_Start and
            ctxt.idx() <= eLED_Use_Resistance_End
        );
    };

    data::Choice::Context led{profile_};
    led.update(eLED_Max);
    led.choose(eLED_None);

    data::Integer::Context resistance{resistance_};
    resistance.update({.min_=0, .max_=10000, .inc_=50});
}

bool Simple::LED::enumerate(const EnumFunc& func) {
    if (func(profile_, strID(PROFILE_STR), PROFILE_STR)) return true;
    if (func(powerPin_, strID(POWERPIN_STR), POWERPIN_STR)) return true;
    if (func(resistance_, strID(RESISTANCE_STR), RESISTANCE_STR)) return true;
    return false;
}

data::Model *Simple::LED::find(uint64 id) {
    if (id == strID(PROFILE_STR)) return &profile_;
    if (id == strID(POWERPIN_STR)) return &powerPin_;
    if (id == strID(RESISTANCE_STR)) return &resistance_;
    return nullptr;
}

