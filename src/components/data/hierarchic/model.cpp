#include "model.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/hierarchic/model.cpp
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

#include "data/hierarchic/root.hpp"
#include "data/receiver.hpp"
#include "utils/hash.hpp"

using namespace data::hier;

Model::Model(Root& root) : mRoot{root} {}

Model::Model(const Model& other, Root& root) :
    base::Model(other), mRoot{root} {}

bool Model::enable(bool en) {
    return processAction(std::make_unique<EnableAction>(en));
}

void Model::lock() const {
    mRoot.mMutex.lock();
}

bool Model::tryLock() const {
    return mRoot.mMutex.try_lock();
}

void Model::unlock() const {
    mRoot.mMutex.unlock();
}

std::vector<Model *> Model::children() {
    auto tmp{const_cast<const Model *>(this)->children()};

    std::vector<Model *> ret;
    ret.reserve(tmp.size());

    for (const auto *ptr : tmp)
        ret.push_back(const_cast<Model *>(ptr));

    return ret;
}

std::vector<const Model *> Model::children() const {
    return {};
}

uint64 Model::hash(uint64 seed) const {
    std::lock_guard scopeLock(*this);

    auto ret{utils::hash::combine(seed, hashThis())};

    for (const auto *child : childrenToHash())
        ret = child->hash(ret);

    return ret;
}

uint64 Model::hashThis() const {
    return 0;
}

std::vector<const Model *> Model::childrenToHash() const {
    return children();
}

bool Model::processAction(std::unique_ptr<Action>&& action) {
    std::lock_guard scopeLock(*this);

    action->mSource = this;

    if (not mRoot.capturePerformance())
        return false;

    if (not action->setup()) {
        mRoot.abortCapture();
        return false;
    }

    // About to maybe invalidate the `action` object, store the underlying
    // pointer.
    auto *store{action.get()};

    // This needs to happen *before* `Action::perform()` as otherwise any
    // nested actions it causes will end up being recorded before it, and
    // things will end up in a partially-forward, partially-reverse order
    // that's impossible to replay.
    //
    // E.g.:
    // `subAct1Sub1`
    // `subAct1`
    // `subAct2Sub1`
    // `subAct2`
    // `this`
    //
    // Where the correct order would be this, subAct1, subAct1Sub1, subAct2,
    // subAct2Sub1.
    if (mRoot.isActuallyCapturing())
        mRoot.recordAction(std::move(action));

    store->perform();

    if (mRoot.isActuallyCapturing())
        mRoot.finishCapture();

    return true;
}

void Model::sendToObservers(const RecvTableBinding& binding) const {
    mRoot.mStates.push_back(Root::State::In_Observer);

    data::base::Model::sendToObservers(binding);

    mRoot.mStates.pop_back();
}

void Model::responderHook(const RecvTableBinding& binding) const {
    auto state{mRoot.mStates.back()};

    const auto performance{[&] {
        // There won't be frames while suppressed.
        if (state == Root::State::Performance) {
            auto& curFrame{mRoot.mActionFrames.back()};

            assert(curFrame.responderId_ == 0);
            curFrame.responderId_ = binding.id_;
        }

        for (auto *receiver : receivers()) {
            std::lock_guard scopeLock(receiver->pMutex);

            auto iter{receiver->mRespondMap.find(this)};
            if (iter != receiver->mRespondMap.end())
                binding.tryTable(*receiver, *iter->second);
        }

        if (state == Root::State::Performance) {
            // The action frame vec could've been modified such that a
            // reference would be invalidated, re-fetch.
            auto& curFrame{mRoot.mActionFrames.back()};
            curFrame.responderId_ = 0;
        }
    }};

    const auto undo{[&] {
        auto& frame{mRoot.mActionFrames.back()};
        auto& children{frame.action_->mChildren};

        const auto processForId{[&](uint64 toProcess) {
            for (; frame.replayIdx_ != -1UZ; --frame.replayIdx_) {
                const auto& [id, action]{children[frame.replayIdx_]};
                if (id != toProcess)
                    break;

                auto& newFrame{mRoot.mActionFrames.emplace_back()};
                newFrame.replayIdx_ = action->mChildren.size() - 1;
                newFrame.action_ = action.get();

                action->retract();

                mRoot.mActionFrames.pop_back();
            }
        }};

        // First, process any non-responder actions that happened before.
        processForId(0);

        // Then any actions for this binding
        processForId(binding.id_);
    }};

    const auto redo{[&] {
        auto& frame{mRoot.mActionFrames.back()};
        auto& children{frame.action_->mChildren};

        const auto processForId{[&](uint64 toProcess) {
            for (; frame.replayIdx_ != children.size(); ++frame.replayIdx_) {
                const auto& [id, action]{children[frame.replayIdx_]};
                if (id != toProcess)
                    break;

                auto& newFrame{mRoot.mActionFrames.emplace_back()};
                newFrame.replayIdx_ = 0;
                newFrame.action_ = action.get();

                action->perform();

                mRoot.mActionFrames.pop_back();
            }
        }};

        // First, process any non-responder actions that happened before.
        processForId(0);

        // Then any actions for this binding
        processForId(binding.id_);
    }};

    switch (state) {
        case Root::State::Suppressed:
            // The hook should still be processed.
            [[fallthrough]];
        case Root::State::Performance:
            performance();
            break;
        case Root::State::Replay_Undo:
            undo();
            break;
        case Root::State::Replay_Redo:
            redo();
            break;
        case Root::State::Normal:
            // responderHook() was called from outside an action?
            [[fallthrough]];
        case Root::State::In_Observer:
            // If this is hit, it means an observer tried to illegally perform
            // an action. It needs to be a responder to do that.
            abort();
    }
}

Model::EnableAction::EnableAction(bool en) : mEnable{en} {}

bool Model::EnableAction::setup() {
    return source<Model>().setupEnable(mEnable);
}

void Model::EnableAction::perform() {
    source<Model>().doEnable(mEnable);
}

void Model::EnableAction::retract() {
    source<Model>().doEnable(not mEnable);
}

Model::CreationScope::CreationScope(Model *model) : mModel{*model} {
    assert(model);
    mModel.root().suppressActions();
}

Model::CreationScope::~CreationScope() {
    mModel.root().unsuppressActions(false);
}

