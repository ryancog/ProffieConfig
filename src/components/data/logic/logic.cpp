#include "logic.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/logic/logic.cpp
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

#include <cassert>
#include <utility>

using namespace data::logic;

detail::Base::~Base() = default;

bool detail::Base::activate(detail::Base& base, ChangeFunc func) {
    return base.activate(std::move(func), pLock);
}

void detail::Base::onChange(bool v) {
    assert(mChangeFunc);
    mChangeFunc(v);
}

bool detail::Base::activate(
    ChangeFunc changeFunc,
    std::recursive_mutex *mutex
) {
    pLock = mutex;
    mChangeFunc = std::move(changeFunc);
    return doActivate();
}

Manager::Manager(Element&& child) :
    mChild{std::move(child)} {
    assert(mChild);

    std::lock_guard scopeLock{mLock};

    const auto changeFunc{[this](bool val) {
        std::lock_guard scopeLock{mLock};
        mVal = val;

        for (auto *rcvr : mReceivers) {
            rcvr->onChange();
        }
    }};
    mVal = mChild->activate(changeFunc, &mLock);
}

void Manager::lock() {
    mLock.lock();
    mChild->lock();
}

void Manager::unlock() {
    mChild->unlock();
    mLock.lock();
}

bool Manager::val() const {
    return mVal;
}

Holder::Holder(Element&& child) :
    shared_ptr(new Manager(std::move(child))) {}

Receiver::~Receiver() = default;

void Receiver::attach(Manager& man) {
    std::scoped_lock scopeLock{mLock, man.mLock};

    detach();

    mMan = &man;
    man.mReceivers.insert(this);
    onChange();
}

void Receiver::detach() {
    std::lock_guard scopeLock{mLock};

    if (mMan != nullptr) {
        std::lock_guard scopeLock{mMan->mLock};

        mMan->mReceivers.erase(this);
        mMan = nullptr;
    }
}

