#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * config/button.h
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

#include <string>
#include <optional>
#include <stdint.h>

namespace Config {

struct Button {
    enum class Type : uint32_t {
        BUTTON,
        LATCHING_BUTTON,
        INVERTEDLATCHINGBUTTON,
        TOUCH_BUTTON
    };
    enum class Function : uint32_t {
        POWER,
        AUX,
        AUX2,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        SELECT
    };

    std::string name;
    std::string pin{-1};
    Type type{Type::BUTTON};
    Function function{Function::POWER};
    int32_t touchValue{-1};
};

std::optional<std::string> buttonTypeToStr(Button::Type);
std::optional<Button::Type> strToButtonType(const std::string&);

}
