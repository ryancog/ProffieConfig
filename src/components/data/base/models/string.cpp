#include "string.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/string.cpp
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
#include <mutex>

using namespace data::base;

String::String(const String& other) = default;

void String::setFilter(Filter filter) {
    std::lock_guard scopeLock(*this);
    mFilter = filter;
}

bool String::clear() {
    return change({});
}

bool String::change(std::string&& str) {
    return change(std::move(str), str.length());
}

bool String::setupChange(std::string& str, size& pos) {
    assert(pos <= str.length());

    if (mFilter) mFilter(*this, str, pos);
    assert(pos <= str.length());

    return str != mValue;
}

std::pair<std::string, size> String::doChange(
    bool undo, std::string&& str, size pos
) {
    auto moved{mPos != pos};

    if (undo) {
        if (moved)
            responderHook(&RecvTable::onMove_);

        responderHook(&RecvTable::onChange_);
    }

    auto lastStr{std::move(mValue)};
    mValue = std::move(str);

    auto lastPos{mPos};
    mPos = pos;

    sendToObservers(&RecvTable::onChange_);

    if (moved)
        sendToObservers(&RecvTable::onMove_);

    if (not undo) {
        responderHook(&RecvTable::onChange_);

        if (moved)
            responderHook(&RecvTable::onMove_);
    }

    return {lastStr, mPos};
}

// NOLINTNEXTLINE(readability-make-member-function-const)
bool String::setupMove(size pos) {
    return pos != mPos;
}

size String::doMove(bool undo, size pos) {
    if (undo)
        responderHook(&RecvTable::onMove_);

    auto ret{mPos};
    mPos = pos;

    sendToObservers(&RecvTable::onMove_);

    if (not undo)
        responderHook(&RecvTable::onMove_);

    return ret;
}

String::ROContext::ROContext(const String& str) : Model::ROContext(str) {}

const std::string& String::ROContext::val() const {
    return model().mValue;
}

size String::ROContext::pos() const {
    return model().mPos;
}

String::Context::Context(String& str) :
    Model::Context(str), ROContext(str), Model::ROContext(str) {}

void String::Context::change(std::string&& str, size pos) const {
    model().change(std::move(str), pos);
}

void String::Context::change(std::string&& str) const {
    change(std::move(str), str.length());
}

void String::Context::append(char c) const {
    append(std::string_view{&c, 1});
}

void String::Context::append(std::string_view view) const {
    auto value{model<String>().mValue};
    value += view;
    change(std::move(value));
}

void String::Context::clear() const {
    model().clear();
}

void String::Context::move(size pos) const {
    model().move(pos);
}

void String::Context::moveStart() const {
    move(0);
}

void String::Context::moveEnd() const {
    move(model<String>().mValue.size());
}

