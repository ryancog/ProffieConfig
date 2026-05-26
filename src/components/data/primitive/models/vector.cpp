#include "vector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/vector.cpp
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

bool Vector::insert(size pos, std::unique_ptr<base::Model>&& obj) {
    std::lock_guard scopeLock(*this);

    if (not setupInsert(pos, obj)) return false;

    doInsert(pos, std::move(obj));
    return true;
}

bool Vector::remove(size pos) {
    std::lock_guard scopeLock(*this);

    if (not setupRemove(pos)) return false;

    doRemove(pos);
    return true;
}

bool Vector::clear() {
    std::lock_guard scopeLock(*this);

    ROContext ctxt(*this);

    if (ctxt.children().empty())
        return false;

    while (not ctxt.children().empty())
        doRemove(0);

    return true;
}

bool Vector::swap(size pos) {
    std::lock_guard scopeLock(*this);

    if (not setupSwap(pos)) return false;

    doSwap(pos);
    return true;
}

