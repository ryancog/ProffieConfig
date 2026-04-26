#include "selector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/selector.cpp
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

#include "utils/defer.hpp"

namespace {

constexpr cstring CHOICE_STRID{"Choice"};

} // namespace

data::Selector::Selector(Node *parent) :
    Node(parent),
    choice_(this) {
    choice_.attachReceiver(static_cast<Choice::Receiver&>(*this));

    data::Bool::Context{mMoveUp}.set(false);
    data::Bool::Context{mMoveDown}.set(false);
    data::Bool::Context{mHasSel}.set(false);
}

// These are specialized "copy constructors"
// NOLINTNEXTLINE(bugprone-copy-constructor-init)
data::Selector::Selector(const Selector& other, Node *parent) :
    Node(other, parent),
    choice_(other.choice_, this),
    mMoveUp(other.mMoveUp),
    mMoveDown(other.mMoveDown),
    mHasSel(other.mHasSel) {}

data::Selector::~Selector() {
    choice_.detachReceiver(static_cast<Choice::Receiver&>(*this));
    if (mVec) mVec->detachReceiver(static_cast<Vector::Receiver&>(*this));
}

auto data::Selector::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Selector>(*this, parent);
}

const data::Bool& data::Selector::canMoveUp() {
    return mMoveUp;
}

const data::Bool& data::Selector::canMoveDown() {
    return mMoveDown;
}

const data::Bool& data::Selector::hasSelection() {
    return mHasSel;
}

bool data::Selector::enumerate(const EnumFunc& func) {
    return func(choice_, strID(CHOICE_STRID), CHOICE_STRID);
}

data::Model *data::Selector::find(uint64 id) {
    if (id == strID(CHOICE_STRID)) return &choice_;
    return nullptr;
}

void data::Selector::onChoice() {
    auto choice{Choice::Receiver::context<Choice>()};

    Bool::Context moveUp{mMoveUp};
    moveUp.set(choice.idx() > 0);

    Bool::Context moveDown{mMoveDown};
    moveDown.set(choice.idx() + 1 != choice.numChoices());

    Bool::Context hasSel{mHasSel};
    hasSel.set(choice.idx() != -1);
}

void data::Selector::onInsert(size pos) {
    Choice::Context choice{choice_};
    Vector::ROContext vec{*mVec};

    auto idx{choice.idx()};
    if (idx >= static_cast<int32>(pos)) {
        ++idx;
    }

    choice.update(vec.children().size(), idx);
}

void data::Selector::preRemove(size pos) {
    // Prevent anything from being tampered with across the removal.
    lock();
    choice_.lock();

    // Clear the choice if it's about to be removed.
    Choice::Context choice{choice_};
    if (choice.idx() == pos) {
        choice.unchoose();
    }
}

void data::Selector::onRemove(size pos) {
    defer {
        choice_.unlock();
        unlock();
    };

    Choice::Context choice{choice_};
    Vector::ROContext vec{*mVec};

    auto idx{choice.idx()};
    if (idx == static_cast<int32>(pos)) {
        idx = -1;
    } else if (idx > static_cast<int32>(pos)) {
        --idx;
    }

    choice.update(vec.children().size(), idx);
}

void data::Selector::onSwap(size pos) {
    Choice::Context choice{choice_};

    if (choice.idx() == pos) {
        choice.choose(static_cast<int32>(pos) + 1);
    } else if (choice.idx() == pos + 1) {
        choice.choose(static_cast<int32>(pos));
    }
}

data::Selector::ROContext::ROContext(const Selector& sel) :
    Model::ROContext(sel) {
    // Also make sure the choice does not change.
    sel.choice_.lock();
}

data::Selector::ROContext::~ROContext() {
    model<data::Selector>().choice_.unlock();
}

const data::Vector *data::Selector::ROContext::bound() const {
    return model<Selector>().mVec;
}

int32 data::Selector::ROContext::choiceIdx() const {
    return data::Choice::ROContext{model<data::Selector>().choice_}.idx();
}

data::Model *data::Selector::ROContext::selectedImpl() const {
    if (not bound()) return nullptr;

    const auto idx{choiceIdx()};
    if (idx == -1) return nullptr;

    data::Vector::ROContext vec{*bound()};
    // In this selector context, the lifetimebound state should be fine.
    // NOLINTNEXTLINE
    return vec.children()[idx].get();
}

data::Selector::Context::Context(Selector& sel) :
    Model::Context(sel), ROContext(sel), Model::ROContext(sel) {}

data::Selector::Context::~Context() = default;

void data::Selector::Context::bind(const Vector *vec) const {
    model().processAction(std::make_unique<BindAction>(
        vec
    ));
}

data::Selector::BindAction::BindAction(const Vector *vec) : mVec{vec} {}

bool data::Selector::BindAction::setup(Model& model) {
    auto& sel{static_cast<Selector&>(model)};
    return sel.mVec != mVec;
}

void data::Selector::BindAction::perform(Model& model) {
    auto& sel{static_cast<Selector&>(model)};
    Choice::Context choice{sel.choice_};

    mLast = sel.mVec;
    mLastSel = choice.idx();

    if (mLast) {
        mLast->detachReceiver(static_cast<Vector::Receiver&>(sel));
    }

    sel.mVec = mVec;

    // So, I've gone back and forth with this, but I think it's best to update
    // the choice first. This way, both the receiver calls for the choice and
    // this are made with valid state.
    //
    // If onRebound is called first, the choice state isn't valid yet.
    uint32 numChoices{0};
    if (sel.mVec) {
        sel.mVec->attachReceiver(static_cast<Vector::Receiver&>(sel));

        Vector::ROContext vec{*sel.mVec};
        numChoices = vec.children().size();
    }

    // For now, if both vecs have same length the choice will persist the
    // selection automatically since no update will actually occur.
    choice.update(numChoices);

    sel.sendToReceivers(&Receiver::onRebound);
}

void data::Selector::BindAction::retract(Model& model) {
    auto& sel{static_cast<Selector&>(model)};
    Choice::Context choice{sel.choice_};

    if (sel.mVec) {
        sel.mVec->detachReceiver(static_cast<Vector::Receiver&>(sel));
    }

    sel.mVec = mLast;

    sel.sendToReceivers(&Receiver::onRebound);

    if (sel.mVec) {
        sel.mVec->attachReceiver(static_cast<Vector::Receiver&>(sel));

        Vector::ROContext vec{*sel.mVec};
        choice.update(vec.children().size(), mLastSel);
    } else {
        choice.update(0);
    }
}

