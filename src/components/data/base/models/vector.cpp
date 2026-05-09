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

void Vector::doInsert(size idx, std::unique_ptr<Model>&& obj) {
    mChildren.insert(
        std::next(mChildren.begin(), static_cast<ssize>(idx)),
        std::move(obj)
    );

    sendToReceivers(&RecvTable::onInsert_, idx);
}

bool Vector::setupRemove(size idx) {
    assert(idx < mChildren.size());
    return true;
}

std::unique_ptr<Model> Vector::doRemove(size idx) {
    sendToReceivers(&RecvTable::preRemove_, idx);

    auto iter{std::next(mChildren.begin(), static_cast<ssize>(idx))};
    auto ret{std::move(*iter)};
    mChildren.erase(iter);

    sendToReceivers(&RecvTable::onRemove_, idx);

    return ret;
}

bool Vector::setupSwap(size idx) {
    assert(idx < mChildren.size() - 1);
    return true;
}

void Vector::doSwap(size idx) {
    auto tmp{std::move(mChildren[idx + 1])};
    mChildren[idx + 1] = std::move(mChildren[idx]);
    mChildren[idx] = std::move(tmp);

    sendToReceivers(&RecvTable::onSwap_, idx);
}

Vector::ROContext::ROContext(const Vector& vec) : Model::ROContext(vec) {}

auto Vector::ROContext::children(
) const -> const std::vector<std::unique_ptr<Model>>& {
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
    for (size idx{0}; idx < children().size(); ++idx) {
        if (children()[idx].get() != &model) continue;

        remove(idx);
        return true;
    }

    return false;
}

void Vector::Context::moveUp(size idx) const {
    model().swap(idx - 1);
}

void Vector::Context::moveDown(size idx) const {
    model().swap(idx);
}

