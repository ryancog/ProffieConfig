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

#include "data/context.hpp"
#include "data/receiver.hpp"

using namespace data::hier;

Vector::Vector(Root& root) : Model(root) {}

// No copy ctor. Don't know how to copy children (or if they even can be).

std::vector<const Model *> Vector::children() const {
    std::vector<const Model *> ret;

    auto ctxt{context(*this)};

    ret.reserve(ctxt.children().size());
    for (const auto& child : ctxt.children()) {
        ret.push_back(dynamic_cast<data::hier::Model *>(child.get()));
    }

    return ret;
}

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
    auto *raw{mModel.get()};

    source<Vector>().doInsert(mPos, std::move(mModel));

    Receiver::maybeActivate(raw);
}

void Vector::InsertAction::retract() {
    Receiver::maybeDeactivate(source<Vector>().children()[mPos]);

    mModel = source<Vector>().doRemove(mPos);
}

Vector::RemoveAction::RemoveAction(size pos) : mPos{pos} {}

bool Vector::RemoveAction::setup() {
    return source<Vector>().setupRemove(mPos);
}

void Vector::RemoveAction::perform() {
    Receiver::maybeDeactivate(source<Vector>().children()[mPos]);

    mModel = source<Vector>().doRemove(mPos);
}

void Vector::RemoveAction::retract() {
    auto *raw{mModel.get()};

    source<Vector>().doInsert(mPos, std::move(mModel));

    Receiver::maybeActivate(raw);
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

