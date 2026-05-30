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

        sendToObservers(&RecvTable::onActionClear_, lastIdx);
        sendToObservers(&RecvTable::onAction_);

        // If could, can't anymore
        if (couldUndo)
            sendToObservers(&RecvTable::onCanUndo_);
        if (couldRedo)
            sendToObservers(&RecvTable::onCanRedo_);
    }

    mStates.pop_back();

    mMutex.unlock();
}

bool Root::capturePerformance() {
    switch (mStates.back()) {
        case State::Normal:
            mStates.push_back(State::Performance);
            break;
        case State::Suppressed:
            // Don't care about setting anything up for recording.
            return true;
        case State::Replay_Undo:
        case State::Replay_Redo:
            // Nothing is allowed besides explicit perform calls.
            return false;
        case State::Performance:
            break;
        case State::In_Observer:
            // If this is hit, it means an observer tried to illegally perform
            // an action. It needs to be a responder to do that.
            abort();
    }

    ++mPerformanceNesting;

    if (mPerformanceNesting == 1) {
        // Increment index to the next action.
        ++mActionIdx;
        
        // Insert rather than clear and append so that an abort can preserve
        // things, and finish knows when to call the observers (we don't want
        // to call the observers here.)
        mActions.emplace(std::next(
            mActions.begin(),
            static_cast<ssize>(mActionIdx)
        ));
    }

    return true;
}

bool Root::isActuallyCapturing() {
    return mStates.back() == State::Performance;
}

void Root::abortCapture() {
    if (not isActuallyCapturing())
        return;

    --mPerformanceNesting;
    // Action frame hasn't been pushed yet.

    // Still more to do before finalizing.
    if (mPerformanceNesting != 0)
        return;

    // This was the "root" action. Remove the empty/null entry.
    mActions.erase(std::next(
        mActions.begin(),
        static_cast<ssize>(mActionIdx)
    ));
    // And pull back the idx.
    --mActionIdx;

    mStates.pop_back();
}

void Root::beginReplay(bool undo, Action& action) {
    if (undo) {
        mStates.push_back(State::Replay_Undo);

        // Setup starting frame.
        auto& frame{mActionFrames.emplace_back()};
        frame.action_ = &action;
        frame.rIter_ = action.mChildren.rbegin();
    } else {
        mStates.push_back(State::Replay_Redo);

        // Setup starting frame.
        auto& frame{mActionFrames.emplace_back()};
        frame.action_ = &action;
        frame.iter_ = action.mChildren.begin();
    }
}

void Root::endReplay() {
    assert(mActionFrames.size() == 1);

    auto& frame{mActionFrames.back()};
    auto& children{frame.action_->mChildren};

    if (mStates.back() == State::Replay_Undo) {
        for (; frame.rIter_ != children.rend(); ++frame.rIter_) {
            const auto& child{*frame.rIter_};

            // These should all be 0 responder id. Any others should've been
            // caught during normal processing.
            assert(child.first == 0);

            child.second->retract();
        }
    } else if (mStates.back() == State::Replay_Redo) {
        for (; frame.iter_ != children.end(); ++frame.iter_) {
            const auto& child{*frame.iter_};

            // These should all be 0 responder id. Any others should've been
            // caught during normal processing.
            assert(child.first == 0);

            child.second->perform();
        }
    } else assert(0);

    mActionFrames.pop_back();
    mStates.pop_back();
}

void Root::recordAction(std::unique_ptr<Action>&& action) {
    assert(mStates.back() == State::Performance);

    auto *raw{action.get()};

    if (mActionFrames.empty()) {
        mActions[mActionIdx] = std::move(action);
    } else {
        auto& curFrame{mActionFrames.back()};
        curFrame.action_->mChildren.emplace_back(
            curFrame.responderId_,
            std::move(action)
        );
    }

    auto& frame{mActionFrames.emplace_back()};
    frame.action_ = raw;
}

void Root::finishCapture() {
    assert(mStates.back() == State::Performance);

    --mPerformanceNesting;
    mActionFrames.pop_back();

    // Still more to do before finalizing.
    if (mPerformanceNesting != 0)
        return;

    // Try coalesce before anything else
    // if (
    //         // There's a prior action.
    //         mActionIdx > 0 and
    //         // For now, only try to coalesce individual actions. Other
    //         // situations are more complicated than I care to deal with for
    //         // now.
    //         mActions[mActionIdx - 1].size() == 1 and
    //         mActions[mActionIdx].size() == 1
    //    ) {
    //     auto& lastAct{*mActions[mActionIdx - 1][0]};
    //     auto& newAct{*mActions[mActionIdx][0]};
    //     if (lastAct.maybeCoalesce(newAct)) {
    //         mActions.pop_back();
    //         --mActionIdx;
    //     }
    // }

    // If there are actions on the redo side, they need to be cleared.
    if (mActions.size() > mActionIdx + 1) {
        // Truncate the actions, removing any available redo.
        mActions.resize(mActionIdx + 1);

        // Cleared; can't anymore
        sendToObservers(&RecvTable::onCanRedo_);
    }

    // Call after all processing is complete.
    sendToObservers(&RecvTable::onAction_);

    // This is the first action, undo is available now, and it was not
    // prior.
    if (mActions.size() == 1)
        sendToObservers(&RecvTable::onCanUndo_);

    mStates.pop_back();
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

    if (model().mStates.back() != State::Normal)
        return;

    auto couldRedo{canRedo()};

    auto& action{model().mActions[model().mActionIdx]};
    model().beginReplay(true, *action);
    // This will cause a cascade due to the replay state.
    action->retract();
    model().endReplay();

    action->source<Model>().focus();

    --model().mActionIdx;

    model().sendToObservers(&RecvTable::onAction_);

    // If we couldn't before, now we can.
    if (not couldRedo)
        model().sendToObservers(&RecvTable::onCanRedo_);

    // We could on entry to this function
    if (not canUndo())
        model().sendToObservers(&RecvTable::onCanUndo_);
}

void Root::Context::redo() const {
    if (not canRedo()) return;

    if (model().mStates.back() != State::Normal)
        return;

    auto couldUndo{canUndo()};

    ++model().mActionIdx;

    auto& action{model().mActions[model().mActionIdx]};
    model().beginReplay(false, *action);
    // This will cause a cascade due to the replay state.
    action->perform();
    model().endReplay();

    action->source<Model>().focus();

    model().sendToObservers(&RecvTable::onAction_);

    // If we couldn't before, now we can.
    if (not couldUndo)
        model().sendToObservers(&RecvTable::onCanUndo_);

    // We could on entry to this function
    if (not canRedo())
        model().sendToObservers(&RecvTable::onCanRedo_);
}

