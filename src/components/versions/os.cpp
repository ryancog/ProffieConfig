#include "os.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/os.cpp
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

#include "versions/priv/data.hpp"

using namespace versions::os;

Context::Context() { priv::lock.lock(); }
Context::~Context() { priv::lock.unlock(); }

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::span<const std::unique_ptr<OS>> Context::available() {
    std::lock_guard scopeLock(priv::lock);
    return priv::availableOS;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::span<const std::unique_ptr<OS>> Context::list() {
    std::lock_guard scopeLock(priv::lock);
    return priv::os;
}

