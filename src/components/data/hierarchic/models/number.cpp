#include "number.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/number.cpp
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

using namespace data::hier;

template <typename T>
detail::Number<T>::Number(Root& root) : Model(root) {}

template <typename T>
detail::Number<T>::Number(const Number& other, Root& root) :
    base::detail::Number<T>(other), Model(other, root) {}

template <typename T>
bool detail::Number<T>::set(T val) {
    return processAction(std::make_unique<SetAction>(val));
}

template <typename T>
bool detail::Number<T>::update(Number<T>::Params params) {
    return processAction(std::make_unique<UpdateAction>(params));
}

template <typename T>
detail::Number<T>::SetAction::SetAction(T val) : mValue{val} {}

template <typename T>
bool detail::Number<T>::SetAction::setup() {
    return source<Number<T>>().setupSet(mValue);
}

template <typename T>
void detail::Number<T>::SetAction::perform() {
    mValue = source<Number<T>>().doSet(mValue);
}

template <typename T>
void detail::Number<T>::SetAction::retract() {
    mValue = source<Number<T>>().doSet(mValue);
}

template <typename T>
detail::Number<T>::UpdateAction::UpdateAction(Number<T>::Params params) :
    mParams{params} {}

template <typename T>
bool detail::Number<T>::UpdateAction::setup() {
    return source<Number<T>>().setupUpdate(mParams);
}

template <typename T>
void detail::Number<T>::UpdateAction::perform() {
    auto last{source<Number<T>>().doUpdate(mParams)};
    mParams = last.first;
    mValue = last.second;
}

template <typename T>
void detail::Number<T>::UpdateAction::retract() {
    auto orig{source<Number<T>>().doUpdate(mParams, mValue)};
    mParams = orig.first;
}

template struct detail::Number<int32>;
template struct detail::Number<float64>;

