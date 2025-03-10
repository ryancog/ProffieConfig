#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/clone.h
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

#include <utility>

template <class BASE>
struct RequireClone {
    virtual ~RequireClone() = default;
    virtual BASE *clone() const = 0;
};

template <class BASE, class DERIVED>
struct Cloneable : BASE {
    Cloneable() = default;
    template <typename ...ARGS>
    Cloneable(ARGS&&... args) : BASE(std::forward<ARGS>(args)...){};
    BASE *clone() const override { 
        return new DERIVED(static_cast<const DERIVED&>(*this));
    }
};

