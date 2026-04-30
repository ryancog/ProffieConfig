#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/number.hpp
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

#include "data/base/model.hpp"
#include "data/recvtable.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data::base {

namespace detail {

template <typename T>
struct DATA_EXPORT Number : virtual Model {
    struct ROContext;
    struct Context;
    struct RecvTable;

    using Filter = void (*)(const ROContext&, T&);

    Number(const Number&);

    static constexpr T DEFAULT_MIN{0};
    static constexpr T DEFAULT_MAX{std::is_integral_v<T>
        ? static_cast<T>(10)
        : static_cast<T>(1)};
    static constexpr T DEFAULT_INC{std::is_integral_v<T>
        ? static_cast<T>(1)
        : static_cast<T>(0.1)};
    static constexpr T DEFAULT_OFF{0};

    struct Params {
        T min_{DEFAULT_MIN};
        T max_{DEFAULT_MAX};
        T inc_{DEFAULT_INC};
        T off_{DEFAULT_OFF};

        auto operator<=>(const Params&) const = default;
    };

    void setFilter(Filter);

    /**
     * Change value
     */
    virtual bool set(T) = 0;

    /**
     * Change the parameters for the Number and, if necessary, update the value
     * to respect them.
     */
    virtual bool update(Params) = 0;

protected:
    bool setupSet(T&);
    T doSet(T);

    bool setupUpdate(Params&);
    std::pair<Params, T> doUpdate(Params, std::optional<T> = {});

private:
    void clamp(T&) const;

    Filter mFilter{nullptr};
    T mValue{0};
    Params mParams;
};

template <typename T>
struct DATA_EXPORT Number<T>::ROContext : virtual Model::ROContext {
    ROContext(const Number&);

    template <typename M = Number>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] T val() const;
    [[nodiscard]] Params params() const;
};

template <typename T>
struct DATA_EXPORT Number<T>::Context : Model::Context, ROContext {
    Context(Number&);

    template <typename M = Number>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void set(T) const;
    void update(Params) const;
};

template <typename T>
struct DATA_EXPORT Number<T>::RecvTable : Model::RecvTable {
    Mapping<> onSet_;
    Mapping<> onUpdate_;
};

} // namespace detail

using Integer = detail::Number<int32>;
using Decimal = detail::Number<float64>;

} // namespace data::base

