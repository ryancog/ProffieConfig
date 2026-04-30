#include "receiver.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/receiver.cpp
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

#include "data/base/model.hpp"

using namespace data;

Receiver::Receiver() = default;

Receiver::~Receiver() {
    // Although at first glance it seems like we could simply detach the
    // receiver here, this situation is unsafe. It means the actual receiver
    // has been destructed while still attached.
    //
    // Calling member functions from the table would result in Bad Things:tm:
    assert(not mAttached);
}

void Receiver::activate() {
    // Can't double-activate.
    assert(not mAttached);

    // First, lock everything we plan to attach to to ensure consistent state.
    // Attach can also happen here, sequentially, since nothing else will be
    // allowed to occur until unlock.
    for (auto [model, map] : pRecvMap) {
        model->lock();
        model->mReceivers.insert(this);
    }

    // Set this before the callback in case it tries something silly.
    mAttached = true;

    // Now that everything's locked (consistent state), the callback is free
    // to do whatever (re)setup it needs.
    onActivate();

    for (auto [model, map] : pRecvMap) {
        model->unlock();
    }
}

void Receiver::deactivate() {
    // Do allow double-deactivate so that deactivate can be called before
    // when it might be called "normally."
    //
    // This is mainly for UI stuff where a usual destroy flow might deactivate,
    // but exceptional cases require early deactivation.
    if (not mAttached) return;

    // The remaining flow mirrors activate()
    for (auto [model, map] : pRecvMap) {
        model->lock();
        model->mReceivers.erase(this);
    }

    mAttached = false;

    onDeactivate();

    for (auto [model, map] : pRecvMap) {
        model->unlock();
    }
}

