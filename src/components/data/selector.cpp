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

namespace {

constexpr cstring CHOICE_STRID{"CHOICE"};
constexpr cstring MOVEUP_STRID{"MOVEDN"};
constexpr cstring MOVEDN_STRID{"MOVEDN"};
constexpr cstring DUP_STRID{"DUP"};

} // namespace

data::Selector::Selector(Node *parent) :
    Node(parent),
    choice_(this),
    moveUp_(this),
    moveDown_(this),
    duplicate_(this) {
    choice_.attachReceiver(static_cast<Choice::Receiver&>(*this));
}

// These are specialized "copy constructors"
// NOLINTNEXTLINE(bugprone-copy-constructor-init)
data::Selector::Selector(const Selector& other, Node *parent) :
    Node(other, parent),
    choice_(other.choice_, this),
    moveUp_(other.moveUp_, this),
    moveDown_(other.moveDown_, this),
    duplicate_(other.duplicate_, this) {}

data::Selector::~Selector() {
    choice_.detachReceiver(static_cast<Choice::Receiver&>(*this));
    if (mVec) mVec->detachReceiver(static_cast<Vector::Receiver&>(*this));
}

auto data::Selector::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Selector>(*this, parent);
}

bool data::Selector::enumerate(const EnumFunc& func) {
    if (func(choice_, strID(CHOICE_STRID), CHOICE_STRID)) return true;
    if (func(moveUp_, strID(MOVEUP_STRID), MOVEUP_STRID)) return true;
    if (func(moveDown_, strID(MOVEDN_STRID), MOVEDN_STRID)) return true;
    if (func(duplicate_, strID(DUP_STRID), DUP_STRID)) return true;
    return false;
}

data::Model *data::Selector::find(uint64 id) {
    if (id == strID(CHOICE_STRID)) return &choice_;
    if (id == strID(MOVEUP_STRID)) return &moveUp_;
    if (id == strID(MOVEDN_STRID)) return &moveDown_;
    if (id == strID(DUP_STRID)) return &duplicate_;
    return nullptr;
}

void data::Selector::onChoice() {
    auto choice{Choice::Receiver::context<Choice>()};

    Generic::Context moveUp{moveUp_};
    moveUp.enable(choice.choice() > 0);

    Generic::Context moveDown{moveDown_};
    moveDown.enable(choice.choice() + 1 != choice.numChoices());

    Generic::Context dup{duplicate_};
    dup.enable(choice.choice() != -1);
}

void data::Selector::onInsert(size pos) {
    Choice::Context choice{choice_};
    Vector::Context vec{*mVec};

    auto lastChoice{choice.choice()};

    choice.update(vec.children().size());

    if (choice.choice() >= pos) ++lastChoice;
    choice.choose(lastChoice);
}

void data::Selector::onRemove(size pos) {
    Choice::Context choice{choice_};
    Vector::Context vec{*mVec};

    auto lastChoice{choice.choice()};

    choice.update(vec.children().size());

    if (choice.choice() == pos) {
        choice.unchoose();
        return;
    }

    if (choice.choice() > pos) --lastChoice;
    choice.choose(lastChoice);
}

void data::Selector::onSwap(size pos) {
    Choice::Context choice{choice_};

    if (choice.choice() == pos) {
        choice.choose(static_cast<int32>(pos) + 1);
    } else if (choice.choice() == pos + 1) {
        choice.choose(static_cast<int32>(pos));
    }
}

data::Selector::Context::Context(Selector& sel) : Node::Context(sel) {}

data::Selector::Context::~Context() = default;

void data::Selector::Context::bind(Vector *vec) const {
    model().processAction(std::make_unique<BindAction>(
        vec
    ));
}

data::Vector *data::Selector::Context::bound() const {
    return model<Selector>().mVec;
}

data::Selector::BindAction::BindAction(Vector *vec) : mVec{vec} {}

bool data::Selector::BindAction::shouldPerform(Model& model) {
    auto& sel{static_cast<Selector&>(model)};
    return sel.mVec != mVec;
}

void data::Selector::BindAction::perform(Model& model) {
    auto& sel{static_cast<Selector&>(model)};
    Choice::Context choice{sel.choice_};

    mLast = sel.mVec;
    mLastSel = choice.choice();

    if (mLast) {
        mLast->detachReceiver(static_cast<Vector::Receiver&>(sel));
    }

    sel.mVec = mVec;
    
    if (sel.mVec) {
        sel.mVec->attachReceiver(static_cast<Vector::Receiver&>(sel));

        Vector::Context vec{*sel.mVec};
        choice.update(vec.children().size());
        // TODO: Persistence here.
    } else {
        choice.update(0);
    }
}

void data::Selector::BindAction::retract(Model& model) {
    auto& sel{static_cast<Selector&>(model)};
    Choice::Context choice{sel.choice_};

    if (sel.mVec) {
        sel.mVec->detachReceiver(static_cast<Vector::Receiver&>(sel));
    }

    sel.mVec = mLast;

    if (sel.mVec) {
        sel.mVec->attachReceiver(static_cast<Vector::Receiver&>(sel));

        Vector::Context vec{*sel.mVec};
        choice.update(vec.children().size());
        choice.choose(mLastSel);
    } else {
        choice.update(0);
    }
}

