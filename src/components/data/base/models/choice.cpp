#include "choice.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/choice.cpp
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

Choice::Choice(const Choice& other) = default;

void Choice::setFilter(Filter filter) {
    std::lock_guard scopeLock(*this);
    mFilter = filter;
}

bool Choice::setupChoose(int32& idx) {
    assert(idx >= -1);
    assert(idx < static_cast<int32>(mNum));

    if (mFilter) mFilter(*this, idx);

    assert(idx >= -1);
    assert(idx < static_cast<int32>(mNum));

    return mIdx != idx;
}

int32 Choice::doChoose(int32 idx) {
    auto ret{mIdx};
    mIdx = idx;

    sendToReceivers(&RecvTable::onChoice_);

    return ret;
}

bool Choice::setupUpdate(uint32 num, int32& idx) {
    if (mNum == num) return false;

    assert(idx >= -1 and idx < static_cast<int32>(num));
    if (mFilter) {
        // Make it so that the filter sees the proposed number of choices
        // rather than the current number.
        auto tmp{mNum};
        mNum = num;

        mFilter(*this, idx);

        mNum = tmp;
    }
    assert(idx >= -1 and idx < static_cast<int32>(num));

    return true;
}

std::pair<uint32, int32> Choice::doUpdate(uint32 num, int32 idx) {
    std::pair<uint32, int32> ret{mNum, mIdx};

    UpdateInfo info{
        .choicePreserved_=mIdx == idx
    };

    mNum = num;;
    mIdx = idx;

    sendToReceivers(&RecvTable::onUpdate_, info);
    if (not info.choicePreserved_) {
        sendToReceivers(&RecvTable::onChoice_);
    }

    return ret;
}

Choice::ROContext::ROContext(const Choice& bl) : Model::ROContext(bl) {}

int32 Choice::ROContext::idx() const {
    return model().mIdx;
}

uint32 Choice::ROContext::num() const {
    return model().mNum;
}

Choice::Context::Context(Choice& bl) : 
    Model::Context(bl), ROContext(bl), Model::ROContext(bl) {}

void Choice::Context::choose(int32 val) const {
    model().choose(val);
}

void Choice::Context::update(uint32 num, int32 idx) const {
    model().update(num, idx);
}

