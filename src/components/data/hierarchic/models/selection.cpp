#include "selection.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/selection.cpp
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

using namespace data::hier;

bool Selection::select(uint32 idx, bool select) {
    return processAction(std::make_unique<SelectAction>(idx, select));
}

bool Selection::select(std::string&& str) {
    auto ctxt{context(*this)};

    auto idx{findString(str)};
    if (idx == -1) {
        return processAction(std::make_unique<InsertAction>(
            ctxt.items().size(), std::move(str)
        ));
    }

    return processAction(std::make_unique<SelectAction>(idx, true));
}

bool Selection::setItems(std::vector<std::string>&& items) {
    return processAction(std::make_unique<SetItemsAction>(std::move(items)));
}

bool Selection::add(std::string&& str) {
    auto ctxt{context(*this)};
    return processAction(std::make_unique<InsertAction>(
        ctxt.items().size(), std::move(str)
    ));
}

bool Selection::remove(uint32 idx) {
    return processAction(std::make_unique<RemoveAction>(idx));
}

Selection::SelectAction::SelectAction(uint32 idx, bool select) :
    mIdx{idx}, mSelect{select} {}

bool Selection::SelectAction::setup() {
    return source<Selection>().setupSelect(mIdx, mSelect);
}

void Selection::SelectAction::perform() {
    source<Selection>().doSelect(mIdx, mSelect);
}

void Selection::SelectAction::retract() {
    source<Selection>().doSelect(mIdx, not mSelect);
}

Selection::SetItemsAction::SetItemsAction(std::vector<std::string>&& items) :
    mItems{std::move(items)} {}

bool Selection::SetItemsAction::setup() {
    return source<Selection>().setupSetItems(mItems);
}

void Selection::SetItemsAction::perform() {
    auto last{source<Selection>().doSetItems(std::move(mItems))};
    mItems = std::move(last.first);
    mSelected = std::move(last.second);
}

void Selection::SetItemsAction::retract() {
    auto orig{source<Selection>().doSetItems(
        std::move(mItems), std::move(mSelected)
    )};
    mItems = std::move(orig.first);
}

Selection::InsertAction::InsertAction(uint32 idx, std::string&& item) :
    mIdx{idx}, mItem{std::move(item)} {}

bool Selection::InsertAction::setup() {
    return source<Selection>().setupInsert(mIdx, mItem);
}

void Selection::InsertAction::perform() {
    source<Selection>().doInsert(mIdx, std::move(mItem));
}

void Selection::InsertAction::retract() {
    mItem = std::move(source<Selection>().doRemove(mIdx).first);
}

Selection::RemoveAction::RemoveAction(uint32 idx) : mIdx{idx} {}

bool Selection::RemoveAction::setup() {
    return source<Selection>().setupRemove(mIdx);
}

void Selection::RemoveAction::perform() {
    auto removed{source<Selection>().doRemove(mIdx)};
    mItem = std::move(removed.first);
    mSelected = removed.second;
}

void Selection::RemoveAction::retract() {
    source<Selection>().doInsert(mIdx, std::move(mItem), mSelected);
}

