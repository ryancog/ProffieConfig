#include "number.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/number.cpp
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

#include <algorithm>
#include <cassert>
#include <mutex>

using namespace data::base;

template <typename T>
detail::Number<T>::Number(const Number& other) = default;

template <typename T>
void detail::Number<T>::setFilter(Filter filter) {
    std::lock_guard scopeLock(*this);
    mFilter = filter;
}

template <typename T>
void detail::Number<T>::clamp(T& val) const {
    val = std::clamp<T>(
        (((val - mParams.off_) / mParams.inc_) * mParams.inc_) + mParams.off_,
        mParams.min_,
        mParams.max_
    );
}

template <typename T>
bool detail::Number<T>::setupSet(T& val) {
    if (mFilter) mFilter(*this, val);
    clamp(val);

    return mValue != val;
}

template <typename T>
T detail::Number<T>::doSet(bool undo, T val) {
    if (undo)
        responderHook(&RecvTable::onSet_);

    auto ret{mValue};
    mValue = val;

    sendToObservers(&RecvTable::onSet_);

    if (not undo)
        responderHook(&RecvTable::onSet_);

    return ret;
}

template <typename T>
bool detail::Number<T>::setupUpdate(Params& params) {
    assert(params.min_ <= params.max_);
    assert(params.off_ < params.inc_);

    return mParams != params;
}

template <typename T>
auto detail::Number<T>::doUpdate(
    bool undo, Params params, std::optional<T> val
) -> std::pair<Params, T> {
    const auto lastParams{mParams};
    const auto lastVal{mValue};

    if (undo) {
        if (mValue != val)
            responderHook(&RecvTable::onSet_);

        responderHook(&RecvTable::onUpdate_);
    }

    mParams = params;
    if (val) {
        mValue = *val;
    } else {
        if (mFilter) mFilter(*this, mValue);
        clamp(mValue);
    }

    sendToObservers(&RecvTable::onUpdate_);

    if (lastVal != mValue)
        sendToObservers(&RecvTable::onSet_);

    if (not undo) {
        responderHook(&RecvTable::onUpdate_);

        if (lastVal != mValue)
            responderHook(&RecvTable::onSet_);
    }

    return {lastParams, lastVal};
}

template <typename T>
detail::Number<T>::ROContext::ROContext(const Number& num)
    : Model::ROContext(num) {}

template <typename T>
T detail::Number<T>::ROContext::val() const {
    return model().mValue;
}

template <typename T>
auto detail::Number<T>::ROContext::params() const -> Params {
    return model().mParams;
}

template <typename T>
detail::Number<T>::Context::Context(Number& num)
    : Model::Context(num), Number<T>::ROContext(num), Model::ROContext(num) {}

template <typename T>
void detail::Number<T>::Context::set(T val) const {
    model().set(val);
}

template <typename T>
void detail::Number<T>::Context::update(Params params) const {
    model().update(params);
}

template struct detail::Number<int32>;
template struct detail::Number<float64>;

