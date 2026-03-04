#include "vector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/vector.cpp
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
#include <memory>

data::Vector::Vector(Node *parent) : Node(parent) {
    mRsp = std::make_unique<Responder>();
    mRsp->attach(*this);
}

data::Vector::Vector(const Vector& other, Node *parent) :
    Node(other, parent) {
    mChildren.reserve(other.mChildren.size());
    for (const auto& child : other.mChildren) {
        mChildren.push_back(child->clone(this));
    }
    mRsp = std::make_unique<Responder>(*other.mRsp);
    mRsp->attach(*this);
}

auto data::Vector::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Vector>(*this, parent);
}

auto data::Vector::responder() const -> Responder& { return *mRsp; }

bool data::Vector::enumerate(const EnumFunc& func) {
    std::scoped_lock scopeLock{pLock};

    for (size idx{0}; idx < mChildren.size(); ++idx) {
        if (func(*mChildren[idx], idx, {})) return true;
    }

    return false;
}

data::Model *data::Vector::find(uint64 id) {
    if (id >= mChildren.size()) return nullptr;
    return mChildren[id].get();
}

data::Vector::Context::Context(Vector& vec) : Model::Context(vec) {}

data::Vector::Context::~Context() = default;

void data::Vector::Context::insert(
    size idx, std::unique_ptr<Model>&& obj
) const {
    assert(obj->parent() == &model());
    model().processAction(std::make_unique<InsertAction>(
        idx, std::move(obj)
    ));
}

void data::Vector::Context::add(std::unique_ptr<Model>&& obj) const {
    insert(model<Vector>().mChildren.size(), std::move(obj));
}

void data::Vector::Context::remove(size idx) const {
    model().processAction(std::make_unique<RemoveAction>(
        idx
    ));
}

void data::Vector::Context::moveUp(size idx) const {
    model().processAction(std::make_unique<SwapAction>(
        idx - 1
    ));
}

void data::Vector::Context::moveDown(size idx) const {
    model().processAction(std::make_unique<SwapAction>(
        idx
    ));
}

void data::Vector::Context::duplicate(size idx, DuplicationMode mode) const {
    auto& vec{model<Vector>()};

    assert(idx < vec.mChildren.size());

    size insertIdx{};
    switch (mode) {
        using enum DuplicationMode;
        case Append:
            insertIdx = vec.mChildren.size();
            break;
        case Insert:
            insertIdx = idx + 1;
            break;
    }

    vec.processAction(std::make_unique<InsertAction>(
        insertIdx, vec.mChildren[idx]->clone(&vec)
    ));
}

auto data::Vector::Context::children(
) const -> const std::vector<std::unique_ptr<Model>>& {
    return model<Vector>().mChildren;
}

data::Vector::InsertAction::InsertAction(
    size pos, std::unique_ptr<Model>&& model
) : mPos{pos}, mModel{std::move(model)} {}

bool data::Vector::InsertAction::shouldPerform(Model& model) {
    auto& vec{static_cast<Vector&>(model)};
    assert(mPos <= vec.mChildren.size());
    return true;
}

void data::Vector::InsertAction::perform(Model& model) {
    auto& vec{static_cast<Vector&>(model)};

    vec.mChildren.insert(
        std::next(vec.mChildren.begin(), static_cast<ssize>(mPos)),
        std::move(mModel)
    );

    vec.sendToReceivers(&Receiver::onInsert, mPos);
}

void data::Vector::InsertAction::retract(Model& model) {
    auto& vec{static_cast<Vector&>(model)};

    mModel = std::move(vec.mChildren[mPos]);
    vec.mChildren.erase(std::next(
        vec.mChildren.begin(), static_cast<ssize>(mPos)
    ));

    vec.sendToReceivers(&Receiver::onInsert, mPos);
}

data::Vector::RemoveAction::RemoveAction(size pos) : mPos{pos} {}

bool data::Vector::RemoveAction::shouldPerform(Model& model) {
    auto& vec{static_cast<Vector&>(model)};
    assert(mPos < vec.mChildren.size());
    return true;
}

void data::Vector::RemoveAction::perform(Model& model) {
    auto& vec{static_cast<Vector&>(model)};

    vec.sendToReceivers(&Receiver::preRemove, mPos);

    mModel = std::move(vec.mChildren[mPos]);
    vec.mChildren.erase(std::next(
        vec.mChildren.begin(), static_cast<ssize>(mPos)
    ));

    vec.sendToReceivers(&Receiver::onRemove, mPos);
}

void data::Vector::RemoveAction::retract(Model& model) {
    auto& vec{static_cast<Vector&>(model)};

    vec.mChildren.insert(
        std::next(vec.mChildren.begin(), static_cast<ssize>(mPos)),
        std::move(mModel)
    );

    vec.sendToReceivers(&Receiver::onInsert, mPos);
}

data::Vector::SwapAction::SwapAction(size pos) : mPos{pos} {}

bool data::Vector::SwapAction::shouldPerform(Model& model) {
    auto& vec{static_cast<Vector&>(model)};
    assert(mPos > 0 and mPos < vec.mChildren.size());
    return true;
}

void data::Vector::SwapAction::perform(Model& model) {
    auto& vec{static_cast<Vector&>(model)};

    auto tmp{std::move(vec.mChildren[mPos + 1])};
    vec.mChildren[mPos + 1] = std::move(vec.mChildren[mPos]);
    vec.mChildren[mPos] = std::move(tmp);

    vec.sendToReceivers(&Receiver::onSwap, mPos);
}

void data::Vector::SwapAction::retract(Model& model) {
    // These are actually identical...
    perform(model);
}

