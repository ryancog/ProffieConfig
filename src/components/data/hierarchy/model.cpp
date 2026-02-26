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
    pParent{parent},
    pRoot{parent ? parent->pRoot : root} {}

data::Model::~Model() {
    // Things must be detached by now for similar reasons as receiver.
    assert(mReceivers.empty());
}

uint64 data::Model::strID(const string& str) {
    return std::hash<string>{}(str);
}

data::Model::Model(const Model& other, Node *parent, Root *root) :
    Model(parent, root) {
    mEnabled = other.mEnabled;
    mShown = other.mShown;
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

void data::Model::sendToReceivers(const function<void(Receiver *)>& func) {
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

    if (not action->shouldPerform(*this)) return true;

    if (pRoot and not pRoot->capturePerformance(fromUI)) return false;

    action->perform(*this);

    if (pParent) {
        action->mTrace.clear();
        pParent->sendUpAction(*this, std::move(action));
    }

    return true;
}

data::Model::Context::Context(Model& base) : pModel{base} {
    pModel.pLock.lock();
}

data::Model::Context::~Context() {
    pModel.pLock.unlock();
}

void data::Model::Context::enable(bool en) {
    pModel.processAction(std::make_unique<EnableAction>(en));
}

void data::Model::Context::show(bool show) {
    pModel.processAction(std::make_unique<ShowAction>(show));
}

bool data::Model::Context::enabled() const {
    return pModel.mShown and pModel.mEnabled;
}

bool data::Model::Context::shown() const {
    return pModel.mShown;
}

void data::Model::Context::focus() {
    pModel.sendToReceivers(&Receiver::onFocus);
}

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
    if (model.mShown) model.sendToReceivers(&Receiver::onEnabled, en);
}

data::Model::ShowAction::ShowAction(bool show) : mShow{show} {}

bool data::Model::ShowAction::shouldPerform(Model& model) {
    return model.mShown != mShow;
}

void data::Model::ShowAction::perform(Model& model) {
    show(model, mShow);
}

void data::Model::ShowAction::retract(Model& model) {
    show(model, not mShow);
}

void data::Model::ShowAction::show(Model& model, bool show) {
    model.mShown = show;
    model.sendToReceivers(&Receiver::onShown, show);
}

