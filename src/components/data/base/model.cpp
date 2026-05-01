#include "model.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/model.cpp
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
#include <mutex>

#include "data/receiver.hpp"

using namespace data::base;

Model::Model() = default;

Model::Model(const Model& other) : mEnabled{other.mEnabled} {}

Model::~Model() {
    // Things must be detached by now. If something were to be called, the
    // derived has been destructed, it would be UB.
    assert(mReceivers.empty());
}

void Model::focus() {
    std::lock_guard scopeLock(*this);
    // If a UI element is attached that would respond to this, it'll focus, if
    // not, this does nothing.
    sendToReceivers(&RecvTable::onFocus_);
}

bool Model::setupEnable(bool& en) {
    return mEnabled != en;
}

void Model::doEnable(bool en) {
    mEnabled = en;
    sendToReceivers(&RecvTable::onEnable_);
}

void Model::sendToReceivers(
    const std::function<void(Receiver *, const data::RecvTable *)>& tryTable
) const {
    for (auto *receiver : mReceivers) {
        std::lock_guard scopeLock(receiver->pMutex);

        // If the receiver is in `mReceivers` then it's guaranteed the model is
        // in the map.
        tryTable(receiver, receiver->mRecvMap[this]);
    }
}

Model::ROContext::ROContext(const Model& model) : mModel{model} {
    mModel.lock();
}

Model::ROContext::~ROContext() {
    mModel.unlock();
}

bool Model::ROContext::enabled() const {
    return mModel.mEnabled;
}

Model::Context::Context(Model& base) : ROContext(base) {}

Model::Context::~Context() = default;

void Model::Context::enable(bool en) const {
    model().enable(en);
}

void Model::Context::focus() const {
    model().focus();
}

