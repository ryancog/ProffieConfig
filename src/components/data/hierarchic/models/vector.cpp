#include "vector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/vector.cpp
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

using namespace data::hier;

Vector::Vector(Root& root) : Model(root) {}

// No copy ctor. Don't know how to copy children (or if they even can be).

bool Vector::insert(size idx, std::unique_ptr<base::Model>&& obj) {
    return processAction(std::make_unique<InsertAction>(idx, std::move(obj)));
}

bool Vector::remove(size idx) {
    return processAction(std::make_unique<RemoveAction>(idx));
}

bool Vector::swap(size idx) {
    return processAction(std::make_unique<SwapAction>(idx));
}

Vector::InsertAction::InsertAction(
    size pos, std::unique_ptr<base::Model>&& model
) : mPos{pos}, mModel{std::move(model)} {}

bool Vector::InsertAction::setup() {
    return source<Vector>().setupInsert(mPos, mModel);
}

void Vector::InsertAction::perform() {
    return source<Vector>().doInsert(mPos, std::move(mModel));
}

void Vector::InsertAction::retract() {
    mModel = source<Vector>().doRemove(mPos);
}

Vector::RemoveAction::RemoveAction(size pos) : mPos{pos} {}

bool Vector::RemoveAction::setup() {
    return source<Vector>().setupRemove(mPos);
}

void Vector::RemoveAction::perform() {
    mModel = source<Vector>().doRemove(mPos);
}

void Vector::RemoveAction::retract() {
    source<Vector>().doInsert(mPos, std::move(mModel));
}

Vector::SwapAction::SwapAction(size pos) : mPos{pos} {}

bool Vector::SwapAction::setup() {
    return source<Vector>().setupSwap(mPos);
}

void Vector::SwapAction::perform() {
    source<Vector>().doSwap(mPos);
}

void Vector::SwapAction::retract() {
    source<Vector>().doSwap(mPos);
}

