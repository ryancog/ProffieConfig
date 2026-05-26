#include "root.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/root.cpp
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

using namespace data::hier;

Root::Root() : Model(*this) {}

Root::Root(const Root&) : Root() {
    // Intentionally, nothing from other is copied here.
    // All the important data will be in derived, and the rest of the state
    // aren't things that should be copied.
}

Root::~Root() = default;

void Root::suppressActions() {
    mMutex.lock();

    mStates.push_back(State::Suppressed);
}

void Root::unsuppressActions(bool clearHistory) {
    assert(mStates.back() == State::Suppressed);

    if (clearHistory) {
        auto couldUndo{canUndo()};
        auto couldRedo{canRedo()};

        mActions.clear();
        const auto lastIdx{mActionIdx};
        mActionIdx = eAct_Idx_First;

        sendToReceivers(&RecvTable::onActionClear_, lastIdx);
        sendToReceivers(&RecvTable::onAction_);

        // If could, can't anymore
        if (couldUndo)
            sendToReceivers(&RecvTable::onCanUndo_);
        if (couldRedo)
            sendToReceivers(&RecvTable::onCanRedo_);
    }

    mStates.pop_back();

    mMutex.unlock();
}

bool Root::capturePerformance() {
    std::lock_guard scopeLock(mMutex);

    switch (mStates.back()) {
        case State::Normal:
            mStates.push_back(State::Performance);
            break;
        case State::Suppressed:
            // Don't care about setting anything up for recording.
            return true;
        case State::Replay:
            // Nothing is allowed besides explicit perform calls from compound.
            return false;
        case State::Performance:
            break;
    }

    ++mPerformanceNesting;

    if (mPerformanceNesting == 1) {
        // Increment index to the next action list.
        ++mActionIdx;
        
        // If there are actions on the redo side, they need to be cleared.
        if (mActions.size() > mActionIdx) {
            // mActionIdx right now is the same as the size of the vector with
            // *only* current actions (that is, one less than there will be
            // once this performance is complete).
            //
            // Resized so that the following `emplace_back()` will be accurate
            // for both w/ redo and w/o redo cases. Effectively just truncating
            // the actions, removing any available redo.
            mActions.resize(mActionIdx);

            // Cleared; can't anymore
            sendToReceivers(&RecvTable::onCanRedo_);
        }

        mActions.emplace_back();
    }

    return true;
}

bool Root::isActuallyCapturing() {
    std::lock_guard scopeLock(mMutex);

    return mStates.back() == State::Performance;
}

void Root::abortCapture() {
    std::lock_guard scopeLock(mMutex);

    mActions.pop_back();
    mStates.pop_back();
}

// The begin and end replay here aren't currently necessary (nor are the checks
// for replay state elsewhere), since undo/redo occurs inside locked Context.
bool Root::beginReplay() {
    if (mStates.back() != State::Normal) return false;

    mStates.push_back(State::Replay);
    return true;
}

void Root::endReplay() {
    assert(mStates.back() == State::Replay);

    mStates.pop_back();
}

void Root::recordAction(std::unique_ptr<Action>&& action) {
    std::lock_guard scopeLock(mMutex);

    assert(mStates.back() == State::Performance);

    mActions[mActionIdx].push_back(std::move(action));

    --mPerformanceNesting;
    if (mPerformanceNesting == 0) {
        // Try coalesce before anything else
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

        // Call after all processing is complete.
        sendToReceivers(&RecvTable::onAction_);

        // This is the first action, undo is available now, and it was not
        // prior.
        if (mActions.size() == 1)
            sendToReceivers(&RecvTable::onCanUndo_);

        mStates.pop_back();
    }
}

bool Root::canUndo() const {
    return mActionIdx != eAct_Idx_First;
}

bool Root::canRedo() const {
    return mActionIdx + 1 != mActions.size();
}

Root::ROContext::ROContext(const Root& root) : Model::ROContext(root) {}

size Root::ROContext::actionIndex() const {
    return model().mActionIdx;
}

bool Root::ROContext::canUndo() const {
    return model().canUndo();
}

bool Root::ROContext::canRedo() const {
    return model().canRedo();
}

Root::Context::Context(Root& root) : 
    Model::Context(root), ROContext(root), Model::ROContext(root) {}

void Root::Context::undo() const {
    if (not canUndo()) return;

    if (not model().beginReplay()) return;

    auto couldRedo{canRedo()};

    // Actions are in reverse order
    // Originally I thought that actions should be replayed in reverse (so
    // forward iterated), but it makes more sense to retract in forward order
    // because otherwise children who depend on the parent's state upon an
    // action would be broken.
    auto& aList{model().mActions[model().mActionIdx]};
    for (auto iter{aList.rbegin()}; iter != aList.rend(); ++iter) {
        (*iter)->retract();
    }

    --model().mActionIdx;

    model().endReplay();

    model().sendToReceivers(&RecvTable::onAction_);

    // If we couldn't before, now we can.
    if (not couldRedo)
        model().sendToReceivers(&RecvTable::onCanRedo_);

    // We could on entry to this function
    if (not canUndo())
        model().sendToReceivers(&RecvTable::onCanUndo_);
}

void Root::Context::redo() const {
    if (not canRedo()) return;

    if (not model().beginReplay()) return;

    auto couldUndo{canUndo()};

    ++model().mActionIdx;

    // Actions are in reverse order, so reverse iterate to perform.
    auto& aList{model().mActions[model().mActionIdx]};
    for (auto iter{aList.rbegin()}; iter != aList.rend(); ++iter) {
        (*iter)->perform();
    }

    model().endReplay();

    model().sendToReceivers(&RecvTable::onAction_);

    // If we couldn't before, now we can.
    if (not couldUndo)
        model().sendToReceivers(&RecvTable::onCanUndo_);

    // We could on entry to this function
    if (not canRedo())
        model().sendToReceivers(&RecvTable::onCanRedo_);
}

