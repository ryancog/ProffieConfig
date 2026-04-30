#include "bool.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/bool.cpp
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

#include <mutex>

using namespace data::base;

Bool::Bool(const Bool& other) = default;

void Bool::setFilter(Filter filter) {
    std::lock_guard scopeLock(*this);
    mFilter = filter;
}

bool Bool::setupSet(bool& val) {
    if (mFilter) mFilter(*this, val);
    return mValue != val;
}

void Bool::doSet(bool val) {
    mValue = val;
    sendToReceivers(&RecvTable::onSet_);
}

Bool::ROContext::ROContext(const Bool& bl) : Model::ROContext(bl) {}

bool Bool::ROContext::val() const {
    return model().mValue;
}

Bool::Context::Context(Bool& bl) : 
    Model::Context(bl), ROContext(bl), Model::ROContext(bl) {}

void Bool::Context::set(bool val) const {
    model().set(val);
}

void Bool::Context::operator|=(bool val) const {
    model().set(val or model().mValue);
}

