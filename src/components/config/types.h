#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/config.h
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

#include <memory>
#include <unordered_map>

#include <utils/types.h>

namespace Config {

using UID = uint64;
constexpr UID NULL_ID{static_cast<UID>(~0)};

template<typename T>
using Map = std::unordered_map<UID, std::shared_ptr<T>>;

template<typename T>
using Pair = std::pair<UID, std::shared_ptr<T>>;

// The object is tracked as or part of a config.
struct Tracked {
    operator UID() const { return mUID; }
    [[nodiscard]] UID getUID() const { return mUID; }

protected:
    Tracked(UID id) : mUID(id) {}

private:
    UID mUID{NULL_ID};
};

} // namespace Config

