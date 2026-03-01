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
data::priv::Number<T>::Number(Node *parent) : Model(parent) {
    mRsp = std::make_unique<Responder>();
    mRsp->attach(*this);
}

template <typename T>
data::priv::Number<T>::Number(const Number& other, Node *parent) :
    Model(other, parent) {
    mValue = other.mValue;
    mParams = other.mParams;

    mRsp = std::make_unique<Responder>(*other.mRsp);
    mRsp->attach(*this);
}

template <typename T>
data::priv::Number<T>::~Number() {
    mRsp->detach();
}

template <typename T>
auto data::priv::Number<T>::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Number>(*this, parent);
}

template <typename T>
void data::priv::Number<T>::setFilter(Filter filter) {
    std::lock_guard scopeLock{pLock};
    mFilter = std::move(filter);
}

template <typename T>
T data::priv::Number<T>::Number::clamp(T val) const {
    return std::clamp<T>(
        (((val - mParams.off_) / mParams.inc_) * mParams.inc_) + mParams.off_,
        mParams.min_,
        mParams.max_
    );
}

template <typename T>
data::priv::Number<T>::Context::Context(Number& num)
    : Model::Context{num} {}

template <typename T>
data::priv::Number<T>::Context::~Context() = default;

template <typename T>
void data::priv::Number<T>::Context::set(T val) const {
    model().processAction(std::make_unique<SetAction>(
        val
    ));
}

template <typename T>
void data::priv::Number<T>::Context::update(Params params) const {
    model().processAction(std::make_unique<UpdateAction>(
        params
    ));
}

template <typename T>
T data::priv::Number<T>::Context::val() const {
    return model<Number>().mValue;
}

template <typename T>
auto data::priv::Number<T>::Context::params() const -> Params {
    return model<Number>().mParams;
}

template <typename T>
data::priv::Number<T>::SetAction::SetAction(T val) :
    mValue{val} {}

template <typename T>
bool data::priv::Number<T>::SetAction::shouldPerform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    num.mFilter(mValue);
    mValue = num.clamp(mValue);

    return mValue != num.mValue;
}

template <typename T>
void data::priv::Number<T>::SetAction::perform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    mLast = num.mValue;
    num.mValue = num.clamp(mValue);

    num.sendToReceivers(&Receiver::onSet);
}

template <typename T>
void data::priv::Number<T>::SetAction::retract(Model& model) {
    auto& num{static_cast<Number&>(model)};

    num.mValue = mLast;

    num.sendToReceivers(&Receiver::onSet);
}

template <typename T>
data::priv::Number<T>::UpdateAction::UpdateAction(Params params) :
    mParams{params} {}

template <typename T>
bool data::priv::Number<T>::UpdateAction::shouldPerform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    assert(mParams.min_ <= mParams.max_);
    assert(mParams.off_ < mParams.inc_);

    return mParams != num.mParams;
}

template <typename T>
void data::priv::Number<T>::UpdateAction::perform(Model& model) {
    auto& num{static_cast<Number&>(model)};

    mLast = num.mParams;
    num.mParams = mParams;
    mLastValue = num.mValue;

    auto tmp{num.mValue};
    num.mFilter(tmp);
    num.mValue = num.clamp(tmp);

    num.sendToReceivers(&Receiver::onUpdate);
    num.sendToReceivers(&Receiver::onSet);
}

template <typename T>
void data::priv::Number<T>::UpdateAction::retract(Model& model) {
    auto& num{static_cast<Number&>(model)};

    num.mParams = mLast;
    num.mValue = mLastValue;

    num.sendToReceivers(&Receiver::onUpdate);
    num.sendToReceivers(&Receiver::onSet);
}

template struct data::priv::Number<int32>;
template struct data::priv::Number<float64>;

