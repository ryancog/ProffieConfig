#include "button.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/buttons/button.cpp
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

#include "config/config.hpp"
#include "config/strings.hpp"
#include "utils/string.hpp"

using namespace config::buttons;

Button::Button(Config& config) :
    Model(config),
    type_(root()),
    event_(root()),
    pin_(root()),
    name_(root()),
    touch_(root()) {
    CreationScope createScope(this);

    const auto pinFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            true,
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;
    }};
    pin_.setFilter(pinFilter);

    const auto nameFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trim(
            str,
            {.allowAlpha=true, .allowNum=true},
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;
    }};
    name_.setFilter(nameFilter);

    type_.update(config::eBtn_Type_Max);
    event_.update(config::eBtn_Evt_Max);

    touch_.update({.min_=0, .max_=50000, .inc_=10});
    touch_.set(1700);
}

Button::~Button() = default;

auto Button::children() -> std::vector<Model *> {
    return {
        &type_,
        &event_,
        &pin_,
        &name_,
        &touch_,
    };
}

