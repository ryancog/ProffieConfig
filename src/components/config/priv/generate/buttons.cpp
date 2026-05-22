#include "buttons.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/generate/buttons.cpp
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

#include "config/buttons/button.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"

using namespace config;
using namespace config::priv;

void gen::buttons(std::ostream& out, const Config& config) {
    out << "#ifdef CONFIG_BUTTONS\n";

    auto buttons{data::context(config.buttons_)};

    auto idx{1};
    for (const auto& model : buttons.children()) {
        auto& button{dynamic_cast<buttons::Button&>(*model)};

        auto type{data::context(button.type_)};
        auto event{data::context(button.event_)};
        auto pin{data::context(button.pin_)};

        out << BUTTON_TYPE_STRS[type.idx()] << ' ';
        out << "Button" << idx << '{';
        out << BUTTON_EVENT_STRS[event.idx()] << ", ";
        out << pin.val() << ", ";

        if (type.idx() == eBtn_Type_Touch) {
            auto touch{data::context(button.touch_)};
            out << touch.val() << ", ";
        }

        auto name{data::context(button.name_)};
        out << '"' << name.val() << '"';
        out << "};\n";
        ++idx;
    }

    out << "#endif\n";
}

