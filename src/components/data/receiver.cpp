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

    // onActivate() could potentially modify the mapping, which would cause a
    // mismatch with the model locking.
    std::vector<const base::Model *> models;
    models.reserve(mObserveMap.size() + mRespondMap.size());

    // First, lock everything we plan to attach to to ensure consistent state.
    // Attach can also happen here, sequentially, since nothing else will be
    // allowed to occur until unlock.
    const auto processMap{[this, &models](const RecvMap& map) {
        for (auto [model, table] : map) {
            model->lock();
            model->mReceivers.insert(this);

            models.push_back(model);
        }
    }};
    processMap(mObserveMap);
    processMap(mRespondMap);

    // Set this before the callback in case it tries something silly.
    mAttached = true;

    // And activate any children
    // Do this beforehand, since it's more reasonable that the parent may do
    // things which affect the children/children want to respond to rather than
    // children affecting the parent.
    //
    // Without something more clever, it has to be one way or the other...
    if (auto *ptr{dynamic_cast<hier::Model *>(this)})
        activateHierarchic(ptr);

    // Now that everything's locked (consistent state), the callback is free
    // to do whatever (re)setup it needs.
    onActivate();

    for (const auto *model : models)
        model->unlock();
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

    std::vector<const base::Model *> models;
    models.reserve(mObserveMap.size() + mRespondMap.size());

    const auto processMap{[this, &models](const RecvMap& map) {
        for (auto [model, table] : map) {
            model->lock();
            model->mReceivers.erase(this);

            models.push_back(model);
        }
    }};
    processMap(mObserveMap);
    processMap(mRespondMap);

    mAttached = false;

    if (auto *ptr{dynamic_cast<hier::Model *>(this)})
        deactivateHierarchic(ptr);

    onDeactivate();

    for (const auto *model : models)
        model->unlock();
}

void Receiver::observeWith(const base::Model& model, const RecvTable& table) {
    amendFor(mObserveMap, model, table);
}

void Receiver::respondWith(const hier::Model& model, const RecvTable& table) {
    // The idea of a responder is that it's going to cause actions.
    // If this Receiver isn't a hierarchic model, there's not (currently) a
    // case where it can cause an action, and this is probably a usage error.
    //
    // Also, make sure they're in the same hierarchy.
    auto *self{dynamic_cast<hier::Model *>(this)};
    assert(self and &self->root() == &model.root());

    amendFor(mRespondMap, model, table);
}

void Receiver::amendFor(
    RecvMap& map, const base::Model& model, const RecvTable& table
) {
    std::lock_guard scopeLock(pMutex);

    // Don't leave a hanging reference in a prior-listed model.
    // I think it's best to require explicit repeal(), to make the flow clear.
    //
    // Also don't allow a table to both take on both the observe and respond
    // roles.
    assert(not mapped(model));

    map[&model] = &table;

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

    // It's in one or the other, try each.
    if (not mObserveMap.erase(&model))
        mRespondMap.erase(&model);

    if (mAttached) {
        std::lock_guard scopeLock(model);
        model.mReceivers.erase(this);
    }
}

void Receiver::repealAllWithTable(const RecvTable& test) {
    std::lock_guard scopeLock(pMutex);

    // Don't modify the maps while iterating...
    std::vector<const base::Model *> toRepeal;

    for (auto [model, table] : mObserveMap) {
        if (table == &test)
            toRepeal.push_back(model);
    }

    for (auto [model, table] : mRespondMap) {
        if (table == &test)
            toRepeal.push_back(model);
    }

    for (const auto *model : toRepeal)
        repeal(*model);
}

bool Receiver::mapped(const base::Model& model) const {
    std::lock_guard scopeLock(pMutex);

    return mObserveMap.contains(&model) or mRespondMap.contains(&model);
}

