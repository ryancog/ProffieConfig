#include "vector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/vector.cpp
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
#include <mutex>

using namespace data::base;

bool Vector::append(std::unique_ptr<base::Model>&& obj) {
    std::lock_guard scopeLock(*this);
    return insert(mChildren.size(), std::move(obj));
}

bool Vector::moveUp(size idx) {
    return swap(idx - 1);
}

bool Vector::moveDown(size idx) {
    return swap(idx);
}

bool Vector::setupInsert(size idx, const std::unique_ptr<Model>& obj) {
    assert(idx <= mChildren.size());
    assert(obj);
    return true;
}

void Vector::doInsert(bool undoRemove, size idx, std::unique_ptr<Model>&& obj) {
    if (undoRemove)
        responderHook(&RecvTable::onRemove_, idx);

    mChildren.insert(
        std::next(mChildren.begin(), static_cast<ssize>(idx)),
        std::move(obj)
    );

    sendToObservers(&RecvTable::onInsert_, idx);

    if (not undoRemove)
        responderHook(&RecvTable::onInsert_, idx);
    else
        responderHook(&RecvTable::preRemove_, idx);
}

bool Vector::setupRemove(size idx) {
    assert(idx < mChildren.size());
    return true;
}

std::unique_ptr<Model> Vector::doRemove(bool undoInsert, size idx) {
    if (undoInsert)
        responderHook(&RecvTable::onInsert_, idx);

    sendToObservers(&RecvTable::preRemove_, idx);

    if (not undoInsert)
        responderHook(&RecvTable::preRemove_, idx);

    auto iter{std::next(mChildren.begin(), static_cast<ssize>(idx))};
    auto ret{std::move(*iter)};
    mChildren.erase(iter);

    sendToObservers(&RecvTable::onRemove_, idx);

    if (not undoInsert)
        responderHook(&RecvTable::onRemove_, idx);

    return ret;
}

bool Vector::setupSwap(size idx) {
    assert(idx < mChildren.size() - 1);
    return true;
}

void Vector::doSwap(bool undo, size idx) {
    if (undo)
        responderHook(&RecvTable::onSwap_, idx);

    auto tmp{std::move(mChildren[idx + 1])};
    mChildren[idx + 1] = std::move(mChildren[idx]);
    mChildren[idx] = std::move(tmp);

    sendToObservers(&RecvTable::onSwap_, idx);

    if (not undo)
        responderHook(&RecvTable::onSwap_, idx);
}

Vector::ROContext::ROContext(const Vector& vec) : Model::ROContext(vec) {}

std::optional<size> Vector::ROContext::find(const Model& model) const {
    for (size idx{0}; idx < children().size(); ++idx) {
        if (children()[idx].get() != &model) continue;

        return idx;
    }

    return std::nullopt;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool Vector::ROContext::canMoveUp(size idx) const {
    return idx > 0;
}

bool Vector::ROContext::canMoveDown(size idx) const {
    return idx + 1 != model().mChildren.size();
}

auto Vector::ROContext::children(
) const -> std::span<const std::unique_ptr<Model>> {
    return model().mChildren;
}

Vector::Context::Context(Vector& vec) :
    Model::Context(vec), ROContext(vec), Model::ROContext(vec) {}

void Vector::Context::insert(
    size idx, std::unique_ptr<Model>&& obj
) const {
    model().insert(idx, std::move(obj));
}

void Vector::Context::append(std::unique_ptr<base::Model>&& obj) const {
    model().append(std::move(obj));
}

void Vector::Context::remove(size idx) const {
    model().remove(idx);
}

bool Vector::Context::remove(Model& model) const {
    if (auto pos{find(model)}) {
        remove(*pos);
        return true;
    }

    return false;
}

void Vector::Context::clear() const {
    model().clear();
}

void Vector::Context::moveUp(size idx) const {
    model().swap(idx - 1);
}

void Vector::Context::moveDown(size idx) const {
    model().swap(idx);
}

