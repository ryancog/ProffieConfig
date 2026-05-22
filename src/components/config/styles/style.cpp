#include "style.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/styles/style.cpp
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
#include "utils/string.hpp"

using namespace config::styles;

Style::Style(Config& config) :
    Model(config), name_(config), style_(config) {

    const auto nameFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;
    }};
    name_.setFilter(nameFilter);
}

Style::Style(const Style& other, Config& config) :
    Model(config), name_(other.name_, config), style_(other.style_, config) {}

auto Style::children() -> std::vector<Model *> {
    return {
        &name_,
        &style_
    };
}

