#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/bladeconfig.h
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

#include "ui/controls/numeric.h"
#include "ui/controls/text.h"
#include "utils/types.h"

#include "ws281x.h"
#include "simple.h"

namespace Config {

constexpr uint32 NO_BLADE{1000000000};

struct BladeConfig {
    BladeConfig();

    void addBlade();
    void removeBlade(uint32 idx);
    void addSubBlade();
    void removeSubBlade(uint32 idx);

    PCUI::TextData name;
    PCUI::ChoiceData presetArray;
    PCUI::NumericData id;
    PCUI::ToggleData noBladeID;

    vector<variant<WS281XBlade, SimpleBlade>> blades;
};

} // namespace Config
