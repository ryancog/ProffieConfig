#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/recvtable.hpp
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

#include <type_traits>

#include "data_export.h"

namespace data {

struct Receiver;

struct DATA_EXPORT RecvTable {
    virtual ~RecvTable() = default;

    template <typename ...Args>
    using Mapping = void (Receiver::*)(Args...);
};

template <typename Receiver, typename ...Args>
requires std::is_base_of_v<data::Receiver, Receiver>
auto map(void (Receiver::*func)(Args...)) {
    return reinterpret_cast<RecvTable::Mapping<Args...>>(func);
}

} // namespace data

