#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/number.hpp
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

#include "data/hierarchy/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

namespace detail {

template <typename T>
struct DATA_EXPORT Number : Model {
    struct ROContext;
    struct Context;
    struct Receiver;
    struct Responder;

    struct SetAction;
    struct UpdateAction;

    using Filter = void (*)(const Context&, T&);

    Number(Node * = nullptr);
    Number(const Number&, Node * = nullptr);
    ~Number() override;

    std::unique_ptr<Model> clone(Node *) const override;

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

    [[nodiscard]] Responder& responder() const;

private:
    [[nodiscard]] T clamp(T) const;

    std::unique_ptr<Responder> mRsp;

    Filter mFilter;
    T mValue{0};
    Params mParams;
};

template <typename T>
struct DATA_EXPORT Number<T>::ROContext : virtual Model::ROContext {
    ROContext(const Number&);
    ~ROContext();

    [[nodiscard]] T val() const;
    [[nodiscard]] Params params() const;
};

template <typename T>
struct DATA_EXPORT Number<T>::Context : Model::Context, ROContext {
    Context(Number&);
    ~Context();

    void set(T val) const;
    void update(Params) const;
};

template <typename T>
struct DATA_EXPORT Number<T>::Receiver : Model::Receiver {
protected:
    friend Number;

    /**
     * Value changed.
     */
    virtual void onSet() {}
    
    /**
     * Params changed.
     */
    virtual void onUpdate() {}
};

template <typename T>
struct DATA_EXPORT Number<T>::Responder : Model::Responder<Number> {
    Model::Responder<Number>::template Function<> onSet_;
    Model::Responder<Number>::template Function<> onUpdate_;

private:
    void onSet() override { 
        if (onSet_) onSet_(
            Model::Responder<Number>::template context<Number>()
        );
    }

    void onUpdate() override {
        if (onUpdate_) onUpdate_(
            Model::Responder<Number>::template context<Number>()
        );
    }
};

template <typename T>
struct DATA_EXPORT Number<T>::SetAction : Action {
    SetAction(T val);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    T mValue;
    T mLast;
};

template <typename T>
struct DATA_EXPORT Number<T>::UpdateAction : Action {
    UpdateAction(Params params);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const Params mParams;
    Params mLast;
    T mLastValue;
};

} // namespace detail

using Integer = detail::Number<int32>;
using Decimal = detail::Number<float64>;

} // namespace data

