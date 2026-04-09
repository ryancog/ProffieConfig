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

constexpr std::string_view STAR1_STR{"Star1"};
constexpr std::string_view STAR2_STR{"Star2"};
constexpr std::string_view STAR3_STR{"Star3"};
constexpr std::string_view STAR4_STR{"Star4"};

constexpr std::string_view LED_STR{"LED"};
constexpr std::string_view POWERPIN_STR{"PowerPin"};
constexpr std::string_view RESISTANCE_STR{"Resistance"};

} // namespace

Simple::Simple(data::Node *parent) :
    data::Node(parent),
    star1_(*this),
    star2_(*this),
    star3_(*this),
    star4_(*this) {}

Simple::~Simple() = default;

bool Simple::enumerate(const EnumFunc& func) {
    if (func(star1_, strID(STAR1_STR), STAR1_STR)) return true;
    if (func(star2_, strID(STAR2_STR), STAR2_STR)) return true;
    if (func(star3_, strID(STAR3_STR), STAR3_STR)) return true;
    if (func(star4_, strID(STAR4_STR), STAR4_STR)) return true;
    return false;
}

data::Model *Simple::find(uint64 id) {
    if (id == strID(STAR1_STR)) return &star1_;
    if (id == strID(STAR2_STR)) return &star2_;
    if (id == strID(STAR3_STR)) return &star3_;
    if (id == strID(STAR4_STR)) return &star4_;
    return nullptr;
}

Simple::Star::Star(Simple& simple) :
    data::Node(&simple),
    led_(this),
    powerPin_(this),
    resistance_(this) {
    CreationScope createScope(*this);

    led_.responder().onChoice_ = [](const data::Choice::ROContext& ctxt) {
        auto& star{*ctxt.model().parent<Star>()};
        data::String::Context{star.powerPin_}.enable(
            ctxt.idx() != eLED_None
        );

        data::Integer::Context{star.resistance_}.enable(
            ctxt.idx() >= eLED_Use_Resistance_Start and
            ctxt.idx() <= eLED_Use_Resistance_End
        );
    };

    data::Choice::Context led{led_};
    led.update(eLED_Max);
    led.choose(eLED_None);

    data::Integer::Context resistance{resistance_};
    resistance.update({.min_=0, .max_=10000, .inc_=50});
}

bool Simple::Star::enumerate(const EnumFunc& func) {
    if (func(led_, strID(LED_STR), LED_STR)) return true;
    if (func(powerPin_, strID(POWERPIN_STR), POWERPIN_STR)) return true;
    if (func(resistance_, strID(RESISTANCE_STR), RESISTANCE_STR)) return true;
    return false;
}

data::Model *Simple::Star::find(uint64 id) {
    if (id == strID(LED_STR)) return &led_;
    if (id == strID(POWERPIN_STR)) return &powerPin_;
    if (id == strID(RESISTANCE_STR)) return &resistance_;
    return nullptr;
}

