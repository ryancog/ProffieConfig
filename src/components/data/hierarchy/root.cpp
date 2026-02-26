#include "root.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchy/root.cpp
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

data::Root::Root() : Node{nullptr, this} {}

data::Root::Root(const Root&) : Root() {
    // Intentionally, nothing from other is copied here.
    // All the important data will be in derived, and the rest of the state
    // aren't things that should be copied.
}

data::Root::~Root() = default;

void data::Root::attachReciever(Receiver& receiver) {
    std::scoped_lock scopeLock{pLock};

    assert(not mReceiver);

    mReceiver = &receiver;
    mReceiver->pRoot = this;
}

void data::Root::detachReceiver(Receiver& receiver) {
    std::scoped_lock scopeLock{pLock};

    assert(&receiver == mReceiver);

    mReceiver->onDetach();
    mReceiver->pRoot = nullptr;
    mReceiver = nullptr;
}

void data::Root::suppressActions() {
    std::scoped_lock scopeLock{pLock};

    while (mState != State::Normal) {
        pLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        pLock.lock();
    }

    mState = State::Suppressed;
}

void data::Root::unsuppressActions() {
    std::scoped_lock scopeLock{pLock};

    assert(mState == State::Suppressed);

    mActions.clear();
    mActionIdx = ACT_IDX_FIRST;
    if (mReceiver) {
        mReceiver->onCanUndo(false);
        mReceiver->onCanRedo(false);
    }

    mState = State::Normal;
}

bool data::Root::capturePerformance(bool fromUI) {
    std::scoped_lock scopeLock{pLock};

    switch (mState) {
        case State::Normal:
            mState = State::Performance;
            break;
        case State::Suppressed:
            // Don't care about setting anything up for recording.
            return not fromUI;
        case State::Replay:
            // Nothing is allowed besides explicit perform calls from compound.
            return false;
        case State::Performance:
            if (fromUI) return false;
            break;
    }

    ++mPerformanceNesting;

    if (mPerformanceNesting == 1) {
        // Increment index to the next action list.
        ++mActionIdx;
        
        // If there are actions on the redo side, they need to be cleared.
        if (mActions.size() > mActionIdx) {
            if (mReceiver) mReceiver->onCanRedo(false);

            // mActionIdx, thanks to zero-indexing, is the same as the size of
            // the vector with *only* current actions.
            //
            // Resized smaller so that the following push will be accurate
            // for both w/ redo and w/o redo cases.
            mActions.resize(mActionIdx);
        }

        mActions.emplace_back();
    }

    return true;
}

void data::Root::abortCapture() {
    std::scoped_lock scopeLock{pLock};

    mActions.pop_back();
    mState = State::Normal;
}

// The begin and end replay here aren't currently necessary (nor are the checks
// for replay state elsewhere), since undo/redo occurs inside locked Context.
bool data::Root::beginReplay() {
    if (mState != State::Normal) return false;

    mState = State::Replay;
    return true;
}

void data::Root::endReplay() {
    assert(mState == State::Replay);
    mState = State::Normal;
}

void data::Root::recordAction(std::unique_ptr<Action>&& action) {
    std::scoped_lock scopeLock{pLock};

    assert(mState == State::Performance);

    mActions[mActionIdx].push_back(std::move(action));

    --mPerformanceNesting;
    if (mPerformanceNesting == 0) {
        // This is the first action, undo is available now, and it was not prior.
        if (mActions.size() == 1) mReceiver->onCanUndo(true);

        if (
                // There's a prior action.
                mActionIdx > 0 and
                // For now, only try to coalesce individual actions. Other
                // situations are more complicated that I care to deal with for
                // now.
                mActions[mActionIdx - 1].size() == 1 and
                mActions[mActionIdx].size() == 1
           ) {
            auto& lastAct{*mActions[mActionIdx - 1][0]};
            auto& newAct{*mActions[mActionIdx][0]};
            if (lastAct.maybeCoalesce(newAct)) {
                mActions.pop_back();
                --mActionIdx;
            }
        }

        mState = State::Normal;
    }
}

auto data::Root::clone(Node *parent) const -> std::unique_ptr<Model> {
    assert(parent == nullptr);
    return clone();
}

data::Root::Context::Context(Root& root) : mRoot{root} {
    mRoot.pLock.lock();
}

data::Root::Context::~Context() {
    mRoot.pLock.unlock();
}

void data::Root::Context::undo() {
    if (not canUndo()) return;

    if (not mRoot.beginReplay()) return;

    // Actions are in reverse order, with the last first, so can just forward
    // iterate.
    auto& aList{mRoot.mActions[mRoot.mActionIdx]};
    for (auto iter{aList.begin()}; iter != aList.end(); ++iter) {
        mRoot.sendDownAction(**iter, ActionMode::Rewind);
    }

    --mRoot.mActionIdx;

    mRoot.endReplay();

    if (mRoot.mReceiver) {
        mRoot.mReceiver->onCanRedo(true);
        mRoot.mReceiver->onCanUndo(canUndo());
    }
}

void data::Root::Context::redo() {
    if (not canRedo()) return;

    if (not mRoot.beginReplay()) return;

    ++mRoot.mActionIdx;

    // Actions are in reverse order, so reverse iterate to perform.
    auto& aList{mRoot.mActions[mRoot.mActionIdx]};
    for (auto iter{aList.rbegin()}; iter != aList.rend(); ++iter) {
        mRoot.sendDownAction(**iter, ActionMode::Perform);
    }

    mRoot.endReplay();

    if (mRoot.mReceiver) {
        mRoot.mReceiver->onCanUndo(true);
        mRoot.mReceiver->onCanRedo(canRedo());
    }
}

bool data::Root::Context::canUndo() const {
    return mRoot.mActionIdx != ACT_IDX_FIRST;
}

bool data::Root::Context::canRedo() const {
    return mRoot.mActionIdx + 1 != mRoot.mActions.size();
}

data::Root::Receiver::~Receiver() = default;

