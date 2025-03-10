#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/trace.h
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

#include <utils/types.h>

#include "private/export.h"

namespace Trace {

struct UTILS_EXPORT FilePos {
    string fullLine;
    int32 line{0};
    int32 pos{0};

    [[nodiscard]] operator string() const;
};

[[nodiscard]] UTILS_EXPORT FilePos getFilePos(const string& fileBuffer, uint64 index);

} // namespace Trace

