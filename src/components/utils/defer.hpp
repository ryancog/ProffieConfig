#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/utils/defer.hpp
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

// See https://stackoverflow.com/a/42060129
namespace defer_priv {

struct Dummy {};

template <typename F>
struct Handler {
    ~Handler() { f_(); }
    F f_;
};

} // namespace defer_priv

template <typename F>
defer_priv::Handler<F> operator*(defer_priv::Dummy, F f) { return {f}; }

#define DEFER_DECL_2(LINE) zzDefer##LINE
#define DEFER_DECL(LINE) DEFER_DECL_2(LINE)
// NOLINTNEXTLINE(readability-identifier-naming)
#define defer auto DEFER_DECL(__LINE__) = defer_priv::Dummy{} * [&]()

