#include "selection.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/selection.cpp
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

#include "data/context.hpp"

using namespace data::prim;

bool Selection::select(uint32 idx, bool select) {
    std::lock_guard scopeLock(*this);

    if (not setupSelect(idx, select)) return false;

    doSelect(idx, select);
    return true;
}

bool Selection::select(std::string&& str) {
    std::lock_guard scopeLock(*this);
    auto ctxt{context(*this)};

    auto idx{findString(str)};

    if (idx == -1) {
        if (not setupInsert(ctxt.items().size(), str)) return false;

        doInsert(ctxt.items().size(), std::move(str));
        return true;
    }

    bool select{true};
    if (not setupSelect(idx, select)) return false;

    doSelect(idx, select);
    return true;
}

bool Selection::setItems(std::vector<std::string>&& items) {
    std::lock_guard scopeLock(*this);

    if (not setupSetItems(items)) return false;

    doSetItems(std::move(items));
    return true;
}

bool Selection::add(std::string&& str) {
    std::lock_guard scopeLock(*this);
    auto ctxt{context(*this)};

    if (not setupInsert(ctxt.items().size(), str)) return false;

    doInsert(ctxt.items().size(), std::move(str));
    return true;
}

bool Selection::remove(uint32 idx) {
    std::lock_guard scopeLock(*this);

    if (not setupRemove(idx)) return false;

    doRemove(idx);
    return true;
}

