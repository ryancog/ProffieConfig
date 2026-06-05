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

namespace base {

struct Model;

} // namespace base

struct Receiver;

template <auto MP>
consteval auto map();

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
//
// TODO: This same RecvTable is used for observers and responders, but there
// are certain things which don't have a responderHook, for one reason or
// another, and so it seems there should be some way to delineate.
//
// It's not an overt issue, but it's certainly misleading/confusing.
struct DATA_EXPORT RecvTable {
    virtual ~RecvTable() = default;

    template <typename ...Args>
    struct Mapping {
        using Func = void (*)(const base::Model&, Receiver&, Args...);

        constexpr Mapping() = default;
        constexpr Mapping(Func func) : func_{func} {}

        Func func_{nullptr};
    };
};

namespace priv {

// Avoid ambiguous resolution via concept constrain
template <typename T = void, typename ...Args>
concept ModelMapping = std::is_same_v<const base::Model&, T>;

template <typename T = void, typename ...Args>
concept PlainMapping = not ModelMapping<T, Args...>;

template <auto MP>
struct Mapper;

template <
    typename Derived,
    PlainMapping ...Args,
    void (Derived::*MEM_PTR)(Args...)
>
struct Mapper<MEM_PTR> {
    static void mapped(
        const base::Model&, Receiver& rcvr, Args... args
    ) {
        (dynamic_cast<Derived&>(rcvr).*MEM_PTR)(args...);
    }
};

template <
    typename Derived,
    ModelMapping ...Args,
    void (Derived::*MEM_PTR)(const base::Model&, Args...)
>
struct Mapper<MEM_PTR> {
    static void mapped(
        const base::Model& model, Receiver& rcvr, Args... args
    ) {
        (dynamic_cast<Derived&>(rcvr).*MEM_PTR)(model, args...);
    }
};

} // namespace priv

template <auto MP>
consteval auto map() {
    return priv::Mapper<MP>::mapped;
}

} // namespace data

