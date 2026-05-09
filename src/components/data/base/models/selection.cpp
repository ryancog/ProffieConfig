#include "selection.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/selection.cpp
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

Selection::Selection(const Selection& other) = default;

void Selection::setAddFilter(AddFilter filter) {
    std::lock_guard scopeLock(*this);
    mAddFilter = filter;
}

void Selection::setPruner(Pruner pruner) {
    std::lock_guard scopeLock(*this);
    mPruner = pruner;
}

bool Selection::setupSelect(uint32 idx, bool& val) {
    assert(idx < mItems.size());
    return mSelected[idx] != val;
}

void Selection::doSelect(uint32 idx, bool val) {
    mSelected[idx] = val;
    sendToReceivers(&RecvTable::onSelection_, idx);
}

bool Selection::setupSetItems(std::vector<std::string>& items) {
    for (auto iter{items.begin()}; iter != items.end();) {
        if (mAddFilter) mAddFilter(*this, *iter);

        if (iter->empty()) iter = items.erase(iter);
        else ++iter;
    }

    if (mItems.size() != items.size()) return true;

    for (size idx{0}; idx < items.size(); ++idx) {
        if (mItems[idx] != items[idx]) return true;
    }

    return false;
}

std::pair<std::vector<std::string>, std::vector<bool>>
Selection::doSetItems(
    std::vector<std::string>&& items, std::vector<bool> selected
) {
    auto lastItems{std::move(mItems)};
    auto lastSel{std::move(mSelected)};

    mItems = std::move(items);
    if (selected.empty()) {
        mSelected.clear();
        mSelected.resize(mItems.size());
    } else {
        assert(selected.size() == mItems.size());
        mSelected = std::move(selected);
    }

    sendToReceivers(&RecvTable::onItems_);

    return {lastItems, lastSel};
}

int32 Selection::findString(const std::string& str) const {
    for (uint32 idx{0}; idx < mItems.size(); ++idx) {
        auto& item{mItems[idx]};

        if (item == str) return static_cast<int32>(idx);
    }

    return -1;
}

bool Selection::isSelected(uint32 idx) {
    assert(idx < mItems.size());
    return mSelected[idx];
}

bool Selection::setupInsert(uint32 idx, std::string& str) {
    assert(idx <= mItems.size());

    if (mAddFilter) mAddFilter(*this, str);

    return not str.empty();
}

void Selection::doInsert(uint32 idx, std::string&& str, bool sel) {
    mItems.insert(
        std::next(mItems.begin(), idx),
        std::move(str)
    );

    mSelected.insert(
        std::next(mSelected.begin(), idx),
        sel
    );

    sendToReceivers(&RecvTable::onInsert_, idx);
    if (sel)
        sendToReceivers(&RecvTable::onSelection_, idx);
}

bool Selection::setupRemove(uint32 idx) {
    assert(idx < mItems.size());
    return true;
}

std::pair<std::string, bool> Selection::doRemove(uint32 idx) {
    auto itemIter{std::next(mItems.begin(), idx)};
    auto selIter{std::next(mSelected.begin(), idx)};

    auto lastStr{std::move(*itemIter)};
    auto lastSel{*selIter};

    mItems.erase(itemIter);
    mSelected.erase(selIter);

    sendToReceivers(&RecvTable::onRemove_, idx);

    return {lastStr, lastSel};
}

Selection::ROContext::ROContext(const Selection& sel) :
    Model::ROContext(sel) {}

const std::vector<bool>& Selection::ROContext::selected() const {
    return model().mSelected;
}

const std::vector<std::string>& Selection::ROContext::items() const {
    return model().mItems;
}

Selection::Context::Context(Selection& sel) :
    Model::Context(sel), ROContext(sel), Model::ROContext(sel) {}

void Selection::Context::select(uint32 idx, bool select) const {
    model().select(idx, select);
}

void Selection::Context::select(std::string&& str) const {
    model().select(std::move(str));
}

void Selection::Context::setItems(std::vector<std::string>&& items) const {
    model().setItems(std::move(items));
}

void Selection::Context::remove(uint32 idx) const {
    model().remove(idx);
}

