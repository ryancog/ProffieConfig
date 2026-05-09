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
#include <variant>

#include "data_export.h"

namespace data {

namespace base {

struct Model;

} // namespace base

struct Receiver;

// TODO: Can I come up with a more brief way to use this?
//
// Right now my usual solution is something like:
// static const auto table{[] {
//     RecvTable table;
//     ... mappings ...
//     return table;
// }()};
//
// Which is a lot of clutter...
struct DATA_EXPORT RecvTable {
    virtual ~RecvTable() = default;

    template <typename ...Args>
    using Mapping = std::variant<
        std::monostate,
        void (Receiver::*)(Args...),
        void (Receiver::*)(const base::Model&, Args...)
    >;
};

template <typename DerivedRcvr, typename ...Args>
requires std::is_base_of_v<Receiver, DerivedRcvr>
auto map(void (DerivedRcvr::*func)(Args...)) {
    return reinterpret_cast<void (Receiver::*)(Args...)>(func);
}

} // namespace data

