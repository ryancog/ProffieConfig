#include "bladeconfig.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/bladeconfig.cpp
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

#include <algorithm>

#include "utils/string.h"

Config::Blade::Blade() {
    type.setChoices(Utils::createEntries({
        "WS281X",
        _("Simple"),
    }));

    // Yeah, we're going to be blissfully ignorant of this handler...
}

Config::BladeConfig::BladeConfig() {
    name.setUpdateHandler([this]() {
        auto nameValue{static_cast<string>(name)};
        Utils::trimUnsafe(nameValue);
        std::transform(
            nameValue.begin(),
            nameValue.end(),
            nameValue.begin(),
            [](unsigned char chr){ return std::tolower(chr); }
        );
        if (static_cast<string>(name) == nameValue) return;
        name = std::move(nameValue);
    });

    // presetArray;
    // id;
    // noBladeID;
}



