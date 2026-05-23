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
#include "data/hierarchic/model.hpp"

using namespace data;

void Receiver::maybeActivate(base::Model *model) {
    if (auto *rcvr{dynamic_cast<Receiver *>(model)})
        rcvr->activate();
    else if (auto *hier{dynamic_cast<hier::Model *>(model)})
        activateHierarchic(hier);
}

void Receiver::maybeDeactivate(base::Model *model) {
    if (auto *rcvr{dynamic_cast<Receiver *>(model)})
        rcvr->deactivate();
    else if (auto *hier{dynamic_cast<hier::Model *>(model)})
        deactivateHierarchic(hier);
}

void Receiver::activateHierarchic(hier::Model *model) {
    for (auto *child : model->children()) {
        if (auto *childRcvr{dynamic_cast<Receiver *>(child)}) {
            std::lock_guard scopeLock{childRcvr->pMutex};

            // When recursively activating things, some stuff might acceptably
            // already have been activated, so skip those.
            if (not childRcvr->mAttached)
                childRcvr->activate();
        } else {
            activateHierarchic(child);
        }
    }
}

void Receiver::deactivateHierarchic(hier::Model *model) {
    for (auto *child : model->children()) {
        if (auto *childRcvr{dynamic_cast<Receiver *>(child)})
            childRcvr->deactivate();
        else
            deactivateHierarchic(child);
    }
}

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
    std::lock_guard scopeLock(pMutex);

    // Can't double-activate.
    assert(not mAttached);

    // First, lock everything we plan to attach to to ensure consistent state.
    // Attach can also happen here, sequentially, since nothing else will be
    // allowed to occur until unlock.
    for (auto [model, map] : mRecvMap) {
        model->lock();
        model->mReceivers.insert(this);
    }

    // Set this before the callback in case it tries something silly.
    mAttached = true;

    // Now that everything's locked (consistent state), the callback is free
    // to do whatever (re)setup it needs.
    onActivate();

    // And activate any children
    if (auto *ptr{dynamic_cast<hier::Model *>(this)})
        activateHierarchic(ptr);

    for (auto [model, map] : mRecvMap) {
        model->unlock();
    }
}

void Receiver::deactivate() {
    std::lock_guard scopeLock(pMutex);

    // Do allow double-deactivate so that deactivate can be called before
    // when it might be called "normally."
    //
    // This is mainly for UI stuff where a usual destroy flow might deactivate,
    // but exceptional cases require early deactivation.
    if (not mAttached) return;

    preDeactivate();

    // The remaining flow mirrors activate()
    for (auto [model, map] : mRecvMap) {
        model->lock();
        model->mReceivers.erase(this);
    }

    mAttached = false;

    if (auto *ptr{dynamic_cast<hier::Model *>(this)})
        deactivateHierarchic(ptr);

    onDeactivate();

    for (auto [model, map] : mRecvMap) {
        model->unlock();
    }
}

void Receiver::amend(const base::Model& model, const RecvTable& table) {
    std::lock_guard scopeLock(pMutex);

    // Don't leave a hanging reference in a prior-listed model.
    // I think it's best to require explicit repeal(), to make the flow clear.
    assert(not mapped(model));

    mRecvMap[&model] = &table;

    // If things are already attached, then this needs special treatment.
    // This won't call onActivate again, if needed another hook can be added,
    // but I don't think it's needed currently.
    if (mAttached) {
        std::lock_guard scopeLock(model);
        model.mReceivers.insert(this);
    }
}

void Receiver::repeal(const base::Model& model) {
    std::lock_guard scopeLock(pMutex);

    mRecvMap.erase(&model);

    if (mAttached) {
        std::lock_guard scopeLock(model);
        model.mReceivers.erase(this);
    }
}

void Receiver::repealAllWithTable(const RecvTable& test) {
    std::lock_guard scopeLock(pMutex);

    // Don't modify the map while iterating over it...
    std::vector<const base::Model *> toRepeal;

    for (auto [model, table] : mRecvMap) {
        if (table == &test)
            toRepeal.push_back(model);
    }

    for (auto *model : toRepeal)
        repeal(*model);
}

bool Receiver::mapped(const base::Model& model) const {
    std::lock_guard scopeLock(pMutex);

    return mRecvMap.contains(&model);
}

