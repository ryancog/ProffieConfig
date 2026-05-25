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
        mActions.clear();
        const auto lastIdx{mActionIdx};
        mActionIdx = eAct_Idx_First;

        sendToReceivers(&RecvTable::onActionClear_, lastIdx);
        sendToReceivers(&RecvTable::onAction_);
        sendToReceivers(&RecvTable::onCanUndo_, false);
        sendToReceivers(&RecvTable::onCanRedo_, false);
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
        //
        // This is alright to increment before the potential call to onCanRedo_
        // because if it is called, there is in fact actions past the current
        // one, and it may even be useful to know what action is being
        // replaced.
        ++mActionIdx;
        
        // If there are actions on the redo side, they need to be cleared.
        if (mActions.size() > mActionIdx) {
            sendToReceivers(&RecvTable::onCanRedo_, false);

            // mActionIdx right now is the same as the size of the vector with
            // *only* current actions (that is, one less than there will be
            // once this performance is complete).
            //
            // Resized so that the following `emplace_back()` will be accurate
            // for both w/ redo and w/o redo cases. Effectively just truncating
            // the actions, removing any available redo.
            mActions.resize(mActionIdx);
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
            sendToReceivers(&RecvTable::onCanUndo_, true);

        mStates.pop_back();
    }
}

Root::ROContext::ROContext(const Root& root) : Model::ROContext(root) {}

size Root::ROContext::actionIndex() const {
    return model().mActionIdx;
}

bool Root::ROContext::canUndo() const {
    return model().mActionIdx != eAct_Idx_First;
}

bool Root::ROContext::canRedo() const {
    return model().mActionIdx + 1 != model().mActions.size();
}

Root::Context::Context(Root& root) : 
    Model::Context(root), ROContext(root), Model::ROContext(root) {}

void Root::Context::undo() const {
    if (not canUndo()) return;

    if (not model().beginReplay()) return;

    // Actions are in reverse order, with the last first, so can just forward
    // iterate.
    auto& aList{model().mActions[model().mActionIdx]};
    for (auto iter{aList.begin()}; iter != aList.end(); ++iter) {
        (*iter)->retract();
    }

    --model().mActionIdx;

    model().endReplay();

    model().sendToReceivers(&RecvTable::onAction_);
    model().sendToReceivers(&RecvTable::onCanRedo_, true);
    model().sendToReceivers(&RecvTable::onCanUndo_, canUndo());
}

void Root::Context::redo() const {
    if (not canRedo()) return;

    if (not model().beginReplay()) return;

    ++model().mActionIdx;

    // Actions are in reverse order, so reverse iterate to perform.
    auto& aList{model().mActions[model().mActionIdx]};
    for (auto iter{aList.rbegin()}; iter != aList.rend(); ++iter) {
        (*iter)->perform();
    }

    model().endReplay();

    model().sendToReceivers(&RecvTable::onAction_);
    model().sendToReceivers(&RecvTable::onCanUndo_, true);
    model().sendToReceivers(&RecvTable::onCanRedo_, canRedo());
}

