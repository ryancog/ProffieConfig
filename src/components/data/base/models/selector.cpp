#include "selector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/selector.cpp
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

#include "data/context.hpp"
#include "data/recvtable.hpp"
#include "utils/defer.hpp"

using namespace data::base;

const Vector::RecvTable Selector::smVecTable{[] {
    Vector::RecvTable table;
    table.onInsert_ = map(&Selector::onInsert);
    table.preRemove_ = map(&Selector::preRemove);
    table.onRemove_ = map(&Selector::onRemove);
    table.onSwap_ = map(&Selector::onSwap);
    return table;
}()};

const Choice::RecvTable Selector::smChoiceTable{[] {
    Choice::RecvTable table;
    table.onChoice_ = map(&Selector::onChoice);
    return table;
}()};

void Selector::init() {
    amend(choice(), smChoiceTable);
}

// No copy ctor because this state can't be preserved. `other` would need to
// be locked and this would need to be activated immediately.

bool Selector::setupBind(const Vector *vec) {
    return mVec != vec;
}

const Vector *Selector::doBind(const Vector *vec) {
    std::lock_guard scopeLock(pMutex);
    auto choiceCtxt{context(choice())};

    sendToReceivers(&RecvTable::preRebound_);

    if (mVec)
        repeal(*mVec);

    auto ret{mVec};
    mVec = vec;

    // So, I've gone back and forth with this, but I think it's best to update
    // the choice first. This way, both the receiver calls for the choice and
    // this are made with valid state.
    //
    // If onRebound is called first, the choice state isn't valid yet.
    if (mVec) {
        auto vecCtxt{context(*mVec)};

        amend(*mVec, smVecTable);

        // For now, if both vecs have same length the choice will persist the
        // selection automatically since no update will actually occur.
        choiceCtxt.update(vecCtxt.children().size());
    } else {
        choiceCtxt.update(0);
    }

    sendToReceivers(&RecvTable::onRebound_);

    return ret;
}

bool Selector::canMoveUp(const Choice::ROContext& ctxt) {
    return ctxt.idx() > 0;
}

bool Selector::canMoveDown(const Choice::ROContext& ctxt) {
    return ctxt.idx() + 1 != ctxt.num();
}

void Selector::onChoice() {
    auto ctxt{context(choice())};

    if (canMoveUp(ctxt) != mLastCanMoveUp) {
        sendToReceivers(&RecvTable::onCanMoveUp_);
        mLastCanMoveUp = canMoveUp(ctxt);
    }

    if (canMoveDown(ctxt) != mLastCanMoveDown) {
        sendToReceivers(&RecvTable::onCanMoveDown_);
        mLastCanMoveDown = canMoveDown(ctxt);
    }
}

void Selector::onInsert(size pos) {
    auto choiceCtxt{context(choice())};
    auto vecCtxt{context(*mVec)};

    auto idx{choiceCtxt.idx()};
    if (idx >= static_cast<int32>(pos)) {
        ++idx;
    }

    choiceCtxt.update(vecCtxt.children().size(), idx);
}

void Selector::preRemove(size pos) {
    // Prevent anything from being tampered with across the removal.
    lock();
    choice().lock();

    // Clear the choice if it's about to be removed.
    auto ctxt{context(choice())};
    if (ctxt.idx() == pos) {
        ctxt.unchoose();
    }
}

void Selector::onRemove(size pos) {
    defer {
        choice().unlock();
        unlock();
    };

    auto choiceCtxt{context(choice())};
    auto vecCtxt{context(*mVec)};

    auto idx{choiceCtxt.idx()};
    if (idx == static_cast<int32>(pos)) {
        idx = -1;
    } else if (idx > static_cast<int32>(pos)) {
        --idx;
    }

    choiceCtxt.update(vecCtxt.children().size(), idx);
}

void Selector::onSwap(size pos) {
    auto ctxt{context(choice())};

    if (ctxt.idx() == pos) {
        ctxt.choose(static_cast<int32>(pos) + 1);
    } else if (ctxt.idx() == pos + 1) {
        ctxt.choose(static_cast<int32>(pos));
    }
}

Selector::ROContext::ROContext(const Selector& sel) :
    Model::ROContext(sel) {
    // Also make sure the choice does not change.
    model().choice().lock();
}

Selector::ROContext::~ROContext() {
    model().choice().unlock();
}

const Vector *Selector::ROContext::bound() const {
    return model().mVec;
}

int32 Selector::ROContext::choiceIdx() const {
    return context(model().choice()).idx();
}

bool Selector::ROContext::canMoveUp() const {
    return Selector::canMoveUp(context(model().choice()));
}

bool Selector::ROContext::canMoveDown() const {
    return Selector::canMoveDown(context(model().choice()));
}

Model *Selector::ROContext::selectedImpl() const {
    if (not bound()) return nullptr;

    const auto idx{choiceIdx()};
    if (idx == -1) return nullptr;

    auto vec{context(*bound())};
    // In this selector context, the lifetimebound state should be fine.
    // NOLINTNEXTLINE
    return vec.children()[idx].get();
}

Selector::Context::Context(Selector& sel) :
    Model::Context(sel), ROContext(sel), Model::ROContext(sel) {}

void Selector::Context::bind(const Vector *vec) const {
    model().bind(vec);
}

