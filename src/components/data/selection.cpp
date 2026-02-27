#include "selection.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/selection.cpp
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

data::Selection::Selection(Node *parent) :
    Model{parent} {}

data::Selection::Selection(const Selection& other, Node *parent) :
    Model(other, parent) {
    mItems = other.mItems;
    mSelected = other.mSelected;
}

auto data::Selection::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Selection>(*this, parent);
}

data::Selection::Context::Context(Selection& sel) :
    Model::Context{sel} {}

data::Selection::Context::~Context() = default;

void data::Selection::Context::select(uint32 idx, bool select) {
    pModel.processAction(std::make_unique<SelectAction>(
        idx, select
    ));
}

void data::Selection::Context::select(std::string&& str) {
    auto& sel{static_cast<Selection&>(pModel)};

    uint32 idx{0};
    for (; idx < sel.mItems.size(); ++idx) {
        if (sel.mItems[idx] == str) break;
    }

    if (idx == sel.mItems.size()) {
        pModel.processAction(std::make_unique<AddAction>(
            std::move(str)
        ));
    }

    select(idx);
}

void data::Selection::Context::clear() { 
    pModel.processAction(std::make_unique<ClearAction>());
}

void data::Selection::Context::setItems(std::vector<std::string>&& items) {
    pModel.processAction(std::make_unique<SetItemsAction>(
        std::move(items)
    ));
}

void data::Selection::Context::add(std::string&& str) {
    pModel.processAction(std::make_unique<AddAction>(
        std::move(str)
    ));
}

void data::Selection::Context::remove(uint32 idx) {
    pModel.processAction(std::make_unique<RemoveAction>(
        idx
    ));
}

const std::vector<bool>& data::Selection::Context::selected() {
    auto& sel{static_cast<Selection&>(pModel)};
    return sel.mSelected;
}

const std::vector<std::string>& data::Selection::Context::items() {
    auto& sel{static_cast<Selection&>(pModel)};
    return sel.mItems;
}

data::Selection::SelectAction::SelectAction(uint32 idx, bool select)
    : mIdx{idx}, mSelect{select} {}

bool data::Selection::SelectAction::shouldPerform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    assert(mIdx < sel.mItems.size());
    return sel.mSelected[mIdx] != mSelect;
}

void data::Selection::SelectAction::perform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mSelected[mIdx] = mSelect;

    sel.sendToReceivers(&Receiver::onSelection, mIdx, mSelect);
}

void data::Selection::SelectAction::retract(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mSelected[mIdx] = not mSelect;

    sel.sendToReceivers(&Receiver::onSelection, mIdx, not mSelect);
}

data::Selection::ClearAction::ClearAction() = default;

bool data::Selection::ClearAction::shouldPerform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    return not sel.mSelected.empty();
}

void data::Selection::ClearAction::perform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    mLast = std::move(sel.mSelected);
    sel.mSelected.clear();
    sel.mSelected.resize(sel.mItems.size());

    for (size idx{0}; idx < sel.mSelected.size(); ++idx) {
        sel.sendToReceivers(&Receiver::onSelection, idx, false);
    }
}

void data::Selection::ClearAction::retract(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mSelected = std::move(mLast);

    for (size idx{0}; idx < sel.mSelected.size(); ++idx) {
        sel.sendToReceivers(&Receiver::onSelection, idx, sel.mSelected[idx]);
    }
}

data::Selection::SetItemsAction::SetItemsAction(
    std::vector<std::string>&& items
) : mItems{std::move(items)} {}

bool data::Selection::SetItemsAction::shouldPerform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    return sel.mItems != mItems;
}

void data::Selection::SetItemsAction::perform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    mLastSelected = std::move(sel.mSelected);
    sel.mSelected.clear();
    sel.mSelected.resize(sel.mItems.size());
    mLast = std::move(sel.mItems);
    sel.mItems = mItems;

    sel.sendToReceivers(&Receiver::onItems, sel.mItems);
}

void data::Selection::SetItemsAction::retract(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mItems = std::move(mLast);
    sel.mSelected = std::move(mLastSelected);

    sel.sendToReceivers(&Receiver::onItems, sel.mItems);
    for (size idx{0}; idx < sel.mSelected.size(); ++idx) {
        sel.sendToReceivers(&Receiver::onSelection, idx, sel.mSelected[idx]);
    }
}

data::Selection::AddAction::AddAction(std::string&& item) :
    mItem{std::move(item)} {}

bool data::Selection::AddAction::shouldPerform(Model&) {
    return true;
}

void data::Selection::AddAction::perform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mItems.push_back(mItem);
    sel.mSelected.emplace_back();

    // size - 1 is the previous end; the receiver's current end.
    sel.sendToReceivers(
        &Receiver::onAdd,
        sel.mItems.size() - 1,
        sel.mItems.back()
    );
}

void data::Selection::AddAction::retract(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mItems.pop_back();
    sel.mSelected.pop_back();

    sel.sendToReceivers(&Receiver::onRemove, sel.mItems.size());
}

data::Selection::RemoveAction::RemoveAction(uint32 idx) : mIdx{idx} {}

bool data::Selection::RemoveAction::shouldPerform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    assert(mIdx < sel.mItems.size());
    return true;
}

void data::Selection::RemoveAction::perform(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    mItem = std::move(sel.mItems[mIdx]);
    mLastSelected = sel.mSelected[mIdx];
    sel.mItems.erase(std::next(sel.mItems.begin(), mIdx));
    sel.mSelected.erase(std::next(sel.mSelected.begin(), mIdx));

    sel.sendToReceivers(&Receiver::onRemove, mIdx);
}

void data::Selection::RemoveAction::retract(Model& model) {
    auto& sel{static_cast<Selection&>(model)};

    sel.mItems.insert(
        std::next(sel.mItems.begin(), mIdx),
        std::move(mItem)
    );
    sel.mSelected.insert(
        std::next(sel.mSelected.begin(), mIdx),
        mLastSelected
    );

    sel.sendToReceivers(&Receiver::onAdd, mIdx, sel.mItems[mIdx]);
}

