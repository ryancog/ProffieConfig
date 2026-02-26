#include "string.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/string.cpp
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

data::String::String(Node *parent) : Model(parent) {}

data::String::String(const String& other, Node *parent) :
    Model(other, parent) {
    pValue = other.pValue;
    pPos = other.pPos;
}

auto data::String::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<String>(*this, parent);
}

data::String::Context::Context(String& text) : Model::Context(text) {}

data::String::Context::~Context() = default;

void data::String::Context::insert(string str) {
    pModel.processAction(std::make_unique<InsertAction>(
        std::move(str)
    ));
}

void data::String::Context::remove(size num) {
    pModel.processAction(std::make_unique<RemoveAction>(
        num
    ));
}

void data::String::Context::clear() {
    auto& text{static_cast<String&>(pModel)};
    moveEnd();
    pModel.processAction(std::make_unique<RemoveAction>(
        text.pValue.size()
    ));
}

void data::String::Context::move(size pos) {
    pModel.processAction(std::make_unique<MoveAction>(
        pos
    ));
}

void data::String::Context::moveStart() {
    move(0);
}

void data::String::Context::moveEnd() {
    auto& text{static_cast<String&>(pModel)};
    move(text.pValue.size());
}

const string& data::String::Context::val() const {
    auto& text{static_cast<String&>(pModel)};
    return text.pValue;
}

size data::String::Context::pos() const {
    auto& text{static_cast<String&>(pModel)};
    return text.pPos;
}

data::String::InsertAction::InsertAction(string&& str) : mStr{std::move(str)} {}

bool data::String::InsertAction::shouldPerform(Model&) {
    return true;
}

void data::String::InsertAction::perform(Model& model) {
    auto& text{static_cast<String&>(model)};

    text.pValue.insert(text.pPos, mStr);
    text.pPos += mStr.size();

    text.sendToReceivers(&Receiver::onChange, text.pValue);
    text.sendToReceivers(&Receiver::onMove, text.pPos);
}

void data::String::InsertAction::retract(Model& model) {
    auto& text{static_cast<String&>(model)};

    text.pPos -= mStr.size();
    text.pValue.erase(text.pPos, mStr.size());

    text.sendToReceivers(&Receiver::onChange, text.pValue);
    text.sendToReceivers(&Receiver::onMove, text.pPos);
}

data::String::RemoveAction::RemoveAction(size num) : mNum{num} {}

bool data::String::RemoveAction::shouldPerform(Model& model) {
    auto& text{static_cast<String&>(model)};
    assert(text.pPos >= mNum);
    return true;
}

void data::String::RemoveAction::perform(Model& model) {
    auto& text{static_cast<String&>(model)};

    text.pPos -= mNum;
    mRemoved = text.pValue.substr(text.pPos, mNum);
    text.pValue.erase(text.pPos, mNum);

    text.sendToReceivers(&Receiver::onChange, text.pValue);
    text.sendToReceivers(&Receiver::onMove, text.pPos);
}

void data::String::RemoveAction::retract(Model& model) {
    auto& text{static_cast<String&>(model)};

    text.pValue.insert(text.pPos, mRemoved);
    text.pPos += mNum;

    text.sendToReceivers(&Receiver::onChange, text.pValue);
    text.sendToReceivers(&Receiver::onMove, text.pPos);
}

data::String::MoveAction::MoveAction(size pos) : mPos{pos} {}

bool data::String::MoveAction::shouldPerform(Model& model) {
    auto& text{static_cast<String&>(model)};

    assert(mPos <= text.pValue.size());
    return mPos != text.pPos;
}

void data::String::MoveAction::perform(Model& model) {
    auto& text{static_cast<String&>(model)};

    mLast = text.pPos;
    text.pPos = mPos;
    
    text.sendToReceivers(&Receiver::onMove, text.pPos);
}

void data::String::MoveAction::retract(Model& model) {
    auto& text{static_cast<String&>(model)};

    text.pPos = mLast;

    text.sendToReceivers(&Receiver::onMove, text.pPos);
}

