#include "range.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * based on code from ProffieOS, copyright Fredrik Hubinette et al.
 *
 * proffieconstructs/range.cpp
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

Range::Range() {}
Range::Range(uint32_t start, uint32_t end) : start(start), end(end) {}

uint32_t Range::size() const {
    if (start >= end) return 0;
    return end - start;
}

Range Range::operator&(const Range& other) const {
    return Range(
            std::max(start, other.start),
            std::min(end, other.end)
            );
}

