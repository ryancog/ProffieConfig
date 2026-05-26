#include "injection.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/misc/injection.cpp
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

using namespace config;

Injection::Injection(Config& config, std::string&& str) :
    Model(config), filename_{std::move(str)} {}

uint64 Injection::hashThis() const {
    std::error_code ec;

    // Need to make sure the file hasn't been modified by the user, this seems
    // a reasonable way to do it (rather than hashing the whole file).
    auto lastTime{fs::last_write_time(filename_, ec)};

    // The error code is ignored here, because the value returned in lastTime
    // is unique for errors in its own right.

    return utils::hash::combine(
        utils::hash::single(filename_),
        // This will drop the high 64 bits. Practically, this should be perfectly
        // fine for uniqueness.
        static_cast<uint64>(lastTime.time_since_epoch().count())
    );
}

