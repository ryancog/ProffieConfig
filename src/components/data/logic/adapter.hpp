#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/logic/adapter.hpp
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

#include "data/base/model.hpp"
#include "data/base/models/bool.hpp"
#include "data/base/models/choice.hpp"
#include "data/base/models/number.hpp"
#include "data/base/models/string.hpp"
#include "data/base/models/vector.hpp"
#include "data/logic/logic.hpp"

#include "data_export.h"

namespace data::logic {

struct IsEnabled {};

DATA_EXPORT Element operator|(const base::Model&, IsEnabled);

struct IsSet {};

DATA_EXPORT Element operator|(const base::Bool&, IsSet);

/**
 * If values are provided, checks if the data has a selection in the set.
 * If no values are provided, checks that the data has a valid selection. E.g.
 * not -1 for a choice
 */
struct DATA_EXPORT HasSelection : std::set<int32> {};

DATA_EXPORT Element operator|(const base::Choice&, HasSelection);

struct DATA_EXPORT IsEmpty {};

DATA_EXPORT Element operator|(const base::String&, IsEmpty);

DATA_EXPORT Element operator|(const base::Vector&, IsEmpty);

struct DATA_EXPORT BitAnd {
    int32 val_;
};

DATA_EXPORT Element operator|(const base::Integer&, BitAnd);

struct DATA_EXPORT Equals {
    int32 val_;
};

DATA_EXPORT Element operator|(const base::Integer&, Equals);

} // namespace data::logic

