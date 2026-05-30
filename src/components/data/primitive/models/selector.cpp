#include "selector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/selector.cpp
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

Selector::Selector() {
    init();
    activate();
}

Selector::~Selector() {
    deactivate();
}

data::base::Choice& Selector::choice() const {
    return mChoice;
}

bool Selector::bind(const base::Vector *vec) {
    std::lock_guard scopeLock(*this);

    if (not setupBind(vec)) return false;

    doBind(false, vec);
    return true;
}

