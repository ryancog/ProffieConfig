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

#include "config/blades/bladeconfig.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"

using namespace config::blades;

Simple::Simple(Blade& blade) :
    Model(blade.root()),
    led1_(*this),
    led2_(*this),
    led3_(*this),
    led4_(*this),
    parent_(blade) {}

auto Simple::children() const -> std::vector<const Model *> {
    return {
        &led1_,
        &led2_,
        &led3_,
        &led4_,
    };
}

Simple::LED::LED(Simple& simple) :
    Model(simple.root()),
    profile_(root()),
    powerPin_(root()),
    resistance_(root()) {
    CreationScope createScope(this);

    static const auto profileTable{[] {
        data::hier::Choice::RecvTable table;
        table.onChoice_ = data::map<&LED::onProfile>();
        return table;
    }()};
    respondWith(profile_, profileTable);

    profile_.update(eLED_Max);
    profile_.choose(eLED_None);

    resistance_.update({.min_=0, .max_=10000, .inc_=50});
}

void Simple::LED::onActivate() {
    onProfile();
}

auto Simple::LED::children() const -> std::vector<const Model *> {
    return {
        &profile_,
        &powerPin_,
        &resistance_,
    };
}

void Simple::LED::onProfile() {
    auto profile{data::context(profile_)};

    powerPin_.enable(profile.idx() != eLED_None);

    resistance_.enable(
        profile.idx() >= eLED_Use_Resistance_Start and
        profile.idx() <= eLED_Use_Resistance_End
    );
}

