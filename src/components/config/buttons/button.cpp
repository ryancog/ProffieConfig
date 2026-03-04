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

using namespace config::buttons;

Button::Button(data::Node *parent) : data::Node(parent) {
    /*
    type.setUpdateHandler([this](uint32 id) {
        if (id != pcui::ChoiceData::eID_Selection) return;

        touch.show(type == TOUCH_BUTTON);
    });
    pin.setUpdateHandler([this](uint32 id) {
        if (id != pcui::ComboBoxData::eID_Value) return;

        auto rawValue{static_cast<string>(pin)};
        uint32 numTrimmed{};
        auto insertionPoint{pin.getInsertionPoint()};
        Utils::trimCppName(
            rawValue,
            true,
            &numTrimmed,
            insertionPoint
        );

        if (rawValue == static_cast<string>(pin)) return;

        pin = std::move(rawValue);
        pin.setInsertionPoint(insertionPoint - numTrimmed);
    });
    name.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto rawValue{static_cast<string>(name)};
        uint32 numTrimmed{};
        auto insertionPoint{name.getInsertionPoint()};
        Utils::trim(
            rawValue,
            {.allowAlpha=true, .allowNum=true},
            &numTrimmed,
            insertionPoint
        );

        if (rawValue == static_cast<string>(name)) return;

        name = std::move(rawValue);
        name.setInsertionPoint(insertionPoint - numTrimmed);
    });

    touch.setRange(0, 50000, false);
    touch.setIncrement(10, false);
    touch.setValue(1700);
    */
}

Button::~Button() = default;

bool Button::enumerate(const EnumFunc&) {
    assert(0); // TODO
}

data::Model *Button::find(uint64) {
    assert(0); // TODO
}

