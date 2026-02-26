#include "bool.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/bool.cpp
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

data::Bool::Bool(Node *parent) : Model(parent) {}

data::Bool::Bool(const Bool& other, Node *parent) :
    Model(other, parent) {
    mValue = other.mValue;
}

auto data::Bool::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Bool>(*this, parent);
}

data::Bool::Context::Context(Bool& bl) : Model::Context(bl) {}

data::Bool::Context::~Context() = default;

void data::Bool::Context::set(bool val) {
    pModel.processAction(std::make_unique<SetAction>(
        val
    ));
}

void data::Bool::Context::operator|=(bool val) {
    auto& bl{static_cast<Bool&>(pModel)};
    pModel.processAction(std::make_unique<SetAction>(
        val or bl.mValue
    ));
}

bool data::Bool::Context::val() const {
    auto& bl{static_cast<Bool&>(pModel)};
    return bl.mValue;
}

data::Bool::SetAction::SetAction(bool val) : mValue{val} {}

bool data::Bool::SetAction::shouldPerform(Model& model) {
    auto& bl{static_cast<Bool&>(model)};
    return bl.mValue != mValue;
}

void data::Bool::SetAction::perform(Model& model) {
    auto& bl{static_cast<Bool&>(model)};

    bl.mValue = mValue;

    bl.sendToReceivers(&Receiver::onSet, bl.mValue);
}

void data::Bool::SetAction::retract(Model& model) {
    auto& bl{static_cast<Bool&>(model)};

    bl.mValue = not mValue;

    bl.sendToReceivers(&Receiver::onSet, bl.mValue);
}

