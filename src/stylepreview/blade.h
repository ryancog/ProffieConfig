#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * stylepreview/blade.h
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

#include "styles/elements/effects.h"
#include <chrono>
#include <cstdint>
#include <vector>

namespace BladeStyles::StylePreview {

struct BladeEffect {
    Effect type;

    uint64_t startMicros{}; // In micros
    int32_t location;
    // float soundLength; // In what time unit?
    // uint16_t wavnum; 
};

class Blade {
public:

    // uint32_t getAngle1() const;

    // Effects are cleared out 7s (7000000us) after occurring
    // Maximum of 10 effects in "stack" (Make this a deque?)
    const std::vector<BladeEffect>& getEffects() const;

    bool isOn() const;

    void doEffect(Effect, int32_t location, int32_t wavNum);

    int32_t batteryLevel{100};
    int32_t numLeds{144};
};

}

