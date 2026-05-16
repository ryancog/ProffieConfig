#include "string.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/string.cpp
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

#include <mutex>

using namespace data::prim;

String::String(std::string&& str) {
    change(std::move(str));
}

bool String::change(std::string&& str, size pos) {
    std::lock_guard scopeLock(*this);

    if (not setupChange(str, pos)) return false;

    doChange(std::move(str), pos);
    return true;
}

bool String::move(size pos) {
    std::lock_guard scopeLock(*this);

    if (not setupMove(pos)) return false;

    doMove(pos);
    return true;
}

