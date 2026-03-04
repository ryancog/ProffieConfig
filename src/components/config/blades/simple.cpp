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

#include "config/priv/strings.hpp"
#include "data/number.hpp"

using namespace config::blades;

Simple::Simple(data::Node *parent) :
    data::Node(parent),
    star1_(*this),
    star2_(*this),
    star3_(*this),
    star4_(*this) {}

Simple::~Simple() = default;

bool Simple::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *Simple::find(uint64) {
    assert(0); // TODO
}

Simple::Star::Star(Simple& simple) : data::Node(&simple) {
    using namespace priv;

    led_.responder().onChoice_ = [](const data::Choice::Context& ctxt) {
        auto& star{*ctxt.model().parent<Star>()};
        data::String::Context{star.powerPin_}.enable(
            ctxt.choice() != eLED_None
        );

        data::Integer::Context{star.resistance_}.enable(
            ctxt.choice() >= eLED_Use_Resistance_Start and
            ctxt.choice() <= eLED_Use_Resistance_End
        );
    };

    data::Choice::Context led{led_};
    led.update(eLED_Max);
    led.choose(eLED_None);

    data::Integer::Context resistance{resistance_};
    resistance.update({.min_=0, .max_=10000, .inc_=50});
}

bool Simple::Star::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *Simple::Star::find(uint64) {
    assert(0); // TODO
}

