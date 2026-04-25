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

#include "config/strings.hpp"
#include "utils/string.hpp"

using namespace config::buttons;

namespace {

constexpr std::string_view TYPE_STR{"Type"};
constexpr std::string_view EVENT_STR{"Event"};
constexpr std::string_view PIN_STR{"Pin"};
constexpr std::string_view NAME_STR{"Name"};
constexpr std::string_view TOUCH_STR{"TouchThreshold"};

} // namespace

Button::Button(data::Node *parent) :
    data::Node(parent),
    type_(this),
    event_(this),
    pin_(this),
    name_(this),
    touch_(this) {
    CreationScope createScope(*this);

    const auto pinFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
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
        const data::String::ROContext&, std::string& str, size& pos
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

    data::Choice::Context{type_}.update(config::eBtn_Type_Max);
    data::Choice::Context{event_}.update(config::eBtn_Evt_Max);

    data::Integer::Context touch{touch_};
    touch.update({.min_=0, .max_=50000, .inc_=10});
    touch.set(1700);
}

Button::~Button() = default;

bool Button::enumerate(const EnumFunc& func) {
    if (func(type_, strID(TYPE_STR), TYPE_STR)) return true;
    if (func(event_, strID(EVENT_STR), EVENT_STR)) return true;
    if (func(pin_, strID(PIN_STR), PIN_STR)) return true;
    if (func(name_, strID(NAME_STR), NAME_STR)) return true;
    if (func(touch_, strID(TOUCH_STR), TOUCH_STR)) return true;
    return false;
}

data::Model *Button::find(uint64 id) {
	if (id == strID(TYPE_STR)) return &type_;
	if (id == strID(EVENT_STR)) return &event_;
	if (id == strID(PIN_STR)) return &pin_;
	if (id == strID(NAME_STR)) return &name_;
	if (id == strID(TOUCH_STR)) return &touch_;
    return nullptr;
}

