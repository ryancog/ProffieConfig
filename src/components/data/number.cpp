#include "number.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/number.cpp
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

template <typename T>
data::detail::Number<T>::Number(Node *parent) : Model(parent) {
    mRsp = std::make_unique<Responder>();
    mRsp->attach(*this);
}

template <typename T>
data::detail::Number<T>::Number(const Number& other, Node *parent) :
    Model(other, parent) {
    mValue = other.mValue;
    mParams = other.mParams;

    mRsp = std::make_unique<Responder>(*other.mRsp);
    mRsp->attach(*this);
}

template <typename T>
data::detail::Number<T>::~Number() {
    mRsp->detach();
}

template <typename T>
auto data::detail::Number<T>::clone(
    Node *parent
) const -> std::unique_ptr<Model> {
    return std::make_unique<Number>(*this, parent);
}

template <typename T>
void data::detail::Number<T>::setFilter(Filter filter) {
    std::lock_guard scopeLock{pLock};
    mFilter = filter;
}

template <typename T>
auto data::detail::Number<T>::responder() const -> Responder& { return *mRsp; }

template <typename T>
T data::detail::Number<T>::Number::clamp(T val) const {
    return std::clamp<T>(
        (((val - mParams.off_) / mParams.inc_) * mParams.inc_) + mParams.off_,
        mParams.min_,
        mParams.max_
    );
}

template <typename T>
data::detail::Number<T>::ROContext::ROContext(const Number& num)
    : Model::ROContext(num) {}

template <typename T>
data::detail::Number<T>::ROContext::~ROContext() = default;

template <typename T>
T data::detail::Number<T>::ROContext::val() const {
    return model<Number>().mValue;
}

template <typename T>
auto data::detail::Number<T>::ROContext::params() const -> Params {
    return model<Number>().mParams;
}

template <typename T>
data::detail::Number<T>::Context::Context(Number& num)
    : Model::Context(num), Number<T>::ROContext(num), Model::ROContext(num) {}

template <typename T>
data::detail::Number<T>::Context::~Context() = default;

template <typename T>
void data::detail::Number<T>::Context::set(T val) const {
    model().processAction(std::make_unique<SetAction>(
        val
    ));
}

template <typename T>
void data::detail::Number<T>::Context::update(Params params) const {
    model().processAction(std::make_unique<UpdateAction>(
        params
    ));
}

template <typename T>
data::detail::Number<T>::SetAction::SetAction(T val) :
    mValue{val} {}

template <typename T>
bool data::detail::Number<T>::SetAction::shouldPerform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    num.mFilter(num, mValue);
    mValue = num.clamp(mValue);

    return mValue != num.mValue;
}

template <typename T>
void data::detail::Number<T>::SetAction::perform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    mLast = num.mValue;
    num.mValue = num.clamp(mValue);

    num.sendToReceivers(&Receiver::onSet);
}

template <typename T>
void data::detail::Number<T>::SetAction::retract(Model& model) {
    auto& num{static_cast<Number&>(model)};

    num.mValue = mLast;

    num.sendToReceivers(&Receiver::onSet);
}

template <typename T>
data::detail::Number<T>::UpdateAction::UpdateAction(Params params) :
    mParams{params} {}

template <typename T>
bool data::detail::Number<T>::UpdateAction::shouldPerform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    assert(mParams.min_ <= mParams.max_);
    assert(mParams.off_ < mParams.inc_);

    return mParams != num.mParams;
}

template <typename T>
void data::detail::Number<T>::UpdateAction::perform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    mLast = num.mParams;
    num.mParams = mParams;
    mLastValue = num.mValue;

    auto tmp{num.mValue};
    num.mFilter(num, tmp);
    num.mValue = num.clamp(tmp);

    num.sendToReceivers(&Receiver::onUpdate);
    num.sendToReceivers(&Receiver::onSet);
}

template <typename T>
void data::detail::Number<T>::UpdateAction::retract(Model& model) {
    auto& num{static_cast<Number&>(model)};

    num.mParams = mLast;
    num.mValue = mLastValue;

    num.sendToReceivers(&Receiver::onUpdate);
    num.sendToReceivers(&Receiver::onSet);
}

template struct data::detail::Number<int32>;
template struct data::detail::Number<float64>;

