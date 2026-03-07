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

using namespace data::logic;

detail::Base::~Base() = default;

bool detail::Base::activate(ChangeFunc func) {
    return activate(std::move(func), pLock);
}

bool detail::Base::activate(
    ChangeFunc changeFunc,
    std::recursive_mutex *mutex
) {
    pLock = mutex;
    return doActivate(std::move(changeFunc));
}

Manager::Manager(Element&& child) :
    mChild{std::move(child)} {
    assert(mChild);

    std::lock_guard scopeLock{mLock};

    const auto changeFunc{[this](bool val) {
        std::lock_guard scopeLock{mLock};
        mVal = val;

        for (auto *rcvr : mReceivers) {
            rcvr->onChange(val);
        }
    }};
    mChild->activate(changeFunc, &mLock);
}

Holder::Holder(Element&& child) :
    shared_ptr(new Manager(std::move(child))) {}

Receiver::~Receiver() = default;

void Receiver::attach(Manager& man) {
    std::scoped_lock scopeLock{mLock, man.mLock};

    detach();

    mMan = &man;
    man.mReceivers.insert(this);
    onChange(man.mVal);
}

void Receiver::detach() {
    std::lock_guard scopeLock{mLock};

    if (mMan != nullptr) {
        std::lock_guard scopeLock{mMan->mLock};

        mMan->mReceivers.erase(this);
        mMan = nullptr;
    }
}

