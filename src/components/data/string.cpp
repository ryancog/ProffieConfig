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

data::String::String(Node *parent) : Model(parent) {
    mRsp = std::make_unique<Responder>();
    mRsp->attach(*this);
}

data::String::String(const String& other, Node *parent) :
    Model(other, parent) {
    mValue = other.mValue;
    mPos = other.mPos;
    mRsp = std::make_unique<Responder>(*other.mRsp);
    mRsp->attach(*this);
}

data::String::~String() {
    mRsp->detach();
}

auto data::String::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<String>(*this, parent);
}

void data::String::setFilter(Filter filter) {
    std::lock_guard scopeLock{pLock};
    mFilter = std::move(filter);
}

data::String::Context::Context(String& str) : Model::Context(str) {}

data::String::Context::~Context() = default;

void data::String::Context::change(std::string&& str, size pos) const {
    model().processAction(std::make_unique<ChangeAction>(
        std::move(str), pos
    ));
}

void data::String::Context::clear() const {
    model().processAction(std::make_unique<ChangeAction>(
        std::string{}, 0
    ));
}

void data::String::Context::move(size pos) const {
    model().processAction(std::make_unique<MoveAction>(
        pos
    ));
}

void data::String::Context::moveStart() const {
    move(0);
}

void data::String::Context::moveEnd() const {
    move(model<String>().mValue.size());
}

const std::string& data::String::Context::val() const {
    return model<String>().mValue;
}

size data::String::Context::pos() const {
    return model<String>().mPos;
}

data::String::ChangeAction::ChangeAction(std::string&& str, size pos) :
    mStr{std::move(str)}, mPos{pos} {}

bool data::String::ChangeAction::shouldPerform(Model& model) {
    auto& str{static_cast<String&>(model)};

    assert(mPos <= mStr.length());
    if (str.mFilter) str.mFilter(mStr, mPos);
    assert(mPos <= mStr.length());

    return mStr != str.mValue or mPos != str.mPos;
}

void data::String::ChangeAction::perform(Model& model) {
    auto& str{static_cast<String&>(model)};

    { std::string tmp{std::move(str.mValue)};
        str.mValue = std::move(mStr);
        mStr = std::move(tmp);
    }

    { auto tmp{str.mPos};
        str.mPos = mPos;
        mPos = tmp;
    }

    str.sendToReceivers(&Receiver::onChange);
    str.sendToReceivers(&Receiver::onMove);
}

void data::String::ChangeAction::retract(Model& model) {
    // These are identical
    perform(model);
}

data::String::MoveAction::MoveAction(size pos) : mPos{pos} {}

bool data::String::MoveAction::shouldPerform(Model& model) {
    auto& str{static_cast<String&>(model)};

    assert(mPos <= str.mValue.size());
    return mPos != str.mPos;
}

void data::String::MoveAction::perform(Model& model) {
    auto& str{static_cast<String&>(model)};

    auto tmp{str.mPos};
    str.mPos = mPos;
    mPos = tmp;
    
    str.sendToReceivers(&Receiver::onMove);
}

void data::String::MoveAction::retract(Model& model) {
    // These are identical
    perform(model);
}

