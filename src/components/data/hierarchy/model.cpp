#include "model.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/hierarchy/model.cpp
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
#include <thread>

#include "data/hierarchy/action.hpp"
#include "data/hierarchy/node.hpp"
#include "data/hierarchy/root.hpp"

data::Model::Model(Node *parent, Root *root) :
    mParent{parent},
    mRoot{parent ? parent->root() : root} {}

data::Model::~Model() {
    // Things must be detached by now for similar reasons as receiver.
    assert(mReceivers.empty());
}

std::unique_ptr<data::Model> data::Model::clone(Node *) const {
    assert(0);
    __builtin_unreachable();
}

uint64 data::Model::strID(const std::string& str) {
    return std::hash<std::string>{}(str);
}

data::Model::Model(const Model& other, Node *parent, Root *root) :
    Model(parent, root) {
    mEnabled = other.mEnabled;
}

void data::Model::attachReceiver(Receiver& receiver) {
    std::scoped_lock scopeLock{pLock, receiver.mLock};

    assert(receiver.mModel == nullptr);

    mReceivers.insert(&receiver);
    receiver.mModel = this;
    receiver.onAttach();
}

void data::Model::detachReceiver(Receiver& receiver) {
    std::scoped_lock scopeLock{pLock, receiver.mLock};

    assert(receiver.mModel == this);
    receiver.onDetach();
    receiver.mModel = nullptr;

    auto erased{mReceivers.erase(&receiver)};
    assert(erased);
}

void data::Model::processAction(std::unique_ptr<Action>&& action) {
    while (not processAction(std::move(action), false)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool data::Model::processUIAction(std::unique_ptr<Action>&& action) {
    return processAction(std::move(action), true);
}

void data::Model::sendToReceivers(const std::function<void(Receiver *)>& func) {
    std::scoped_lock scopeLock{pLock};

    for (auto *receiver : mReceivers) {
        std::scoped_lock scopeLock{receiver->mLock};

        func(receiver);
    }
}

bool data::Model::processAction(
    std::unique_ptr<Action>&& action, bool fromUI
) {
    std::scoped_lock scopeLock{pLock};

    // If UI sent an action, shouldPerform returning false probably means that
    // filtering or other modifications resulted in the values converging. In
    // this case, the UI still needs to reload, so that it has the modified
    // value, and not whatever value that may have happened to have been
    // modified to be equivalent.
    if (not action->shouldPerform(*this)) return not fromUI;

    if (mRoot and not mRoot->capturePerformance(fromUI)) return false;

    action->perform(*this);

    if (mRoot) {
        action->mTrace.clear();
        mParent->sendUpAction(*this, std::move(action));
    }

    return true;
}

data::Model::ROContext::ROContext(const Model& base) : mModel{base} {
    const_cast<Model&>(mModel).pLock.lock();
} 

data::Model::ROContext::~ROContext() {
    const_cast<Model&>(mModel).pLock.unlock();
}

bool data::Model::ROContext::enabled() const {
    return model().mEnabled;
}

data::Model::Context::Context(Model& base) : ROContext(base) {}

data::Model::Context::~Context() = default;

void data::Model::Context::enable(bool en) const {
    model().processAction(std::make_unique<EnableAction>(en));
}

void data::Model::Context::focus() const {
    model().sendToReceivers(&Receiver::onFocus);
}

data::Model::Receiver::Receiver() = default;

/**
 * No-op copy constructor to allow derived data copy.
 */
data::Model::Receiver::Receiver(const Receiver&) {}

data::Model::Receiver::~Receiver() {
    // Although at first glance it seems like we could simply detach the
    // receiver here if it's attached to a model, this situation is potentially
    // unsafe. It means the receiver has begun destruction already while still
    // attached.
    //
    // If this were to be allowed, events could come in with a partially
    // destroyed receiver and this could result in Bad Things:tm:
    assert(not mModel);
}

bool data::Model::Receiver::processAction(std::unique_ptr<Action>&& action) {
    std::scoped_lock scopeLock{mLock};
    if (not mModel) return false;
    return mModel->processAction(std::move(action), true);
}

data::Model::EnableAction::EnableAction(bool en) : mEnable{en} {}

bool data::Model::EnableAction::shouldPerform(Model& model) {
    return model.mEnabled != mEnable;
}

void data::Model::EnableAction::perform(Model& model) {
    enable(model, mEnable);
}

void data::Model::EnableAction::retract(Model& model) {
    enable(model, not mEnable);
}

void data::Model::EnableAction::enable(Model& model, bool en) {
    model.mEnabled = en;
    model.sendToReceivers(&Receiver::onEnabled);
}

