#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/selection.hpp
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

#include <vector>
#include <string>

#include "data/hierarchy/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

// TODO: This would probably like to be a more generic structure.
//       It is currently configured the way it is because it is used solely for power pins, whose elements are intrinsically non-localized, unique
//       strings.
struct DATA_EXPORT Selection : Model {
    struct Context;
    struct Receiver;

    struct SelectAction;
    struct ClearAction;
    struct SetItemsAction;
    struct AddAction;
    struct RemoveAction;

    Selection(Node * = nullptr);
    Selection(const Selection&, Node * = nullptr);

    std::unique_ptr<Model> clone(Node *) const override;

private:
    std::vector<std::string> mItems;
    std::vector<bool> mSelected;
};

struct DATA_EXPORT Selection::Context : Model::Context {
    Context(Selection&);
    ~Context();

    /**
     * Select item at idx. Does nothing if no item
     */
    void select(uint32 idx, bool select = true);
    void deselect(uint32 idx) { select(idx, false); }

    /**
     * Select first item with string. Create new item
     * and select it if none exists.
     */
    void select(std::string&&);

    /**
     * Remove all selections.
     */
    void clear();

    /**
     * Assign a new set of items to the selection. Clears previous selection.
     */
    void setItems(std::vector<std::string>&&);

    /**
     * Add a new item w/ string.
     */
    void add(std::string&&);

    /**
     * Remove item at idx.
     */
    void remove(uint32);

    [[nodiscard]] const std::vector<bool>& selected() [[clang::lifetimebound]];
    [[nodiscard]] const std::vector<std::string>&
        items() [[clang::lifetimebound]];
};

struct DATA_EXPORT Selection::Receiver : Model::Receiver {
protected:
    friend Selection;

    /**
     * Items is (de)selected.
     */
    virtual void onSelection(uint32 idx, bool) {}

    /**
     * All selection is cleared.
     */
    virtual void onClear() {}

    /**
     * Items are completely changed.
     */
    virtual void onItems(const std::vector<std::string>&) {}

    /**
     * Item Added
     */
    virtual void onAdd(uint32, const std::string&) {}

    /**
     * Item Removed
     */
    virtual void onRemove(uint32) {}
};

struct DATA_EXPORT Selection::SelectAction : Action {
    SelectAction(uint32, bool);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const uint32 mIdx;
    const bool mSelect;
};

struct DATA_EXPORT Selection::ClearAction : Action {
    ClearAction();

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    std::vector<bool> mLast;
};

struct DATA_EXPORT Selection::SetItemsAction : Action {
    SetItemsAction(std::vector<std::string>&&);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const std::vector<std::string> mItems;
    std::vector<std::string> mLast;
    std::vector<bool> mLastSelected;
};

struct DATA_EXPORT Selection::AddAction : Action {
    AddAction(std::string&&);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const std::string mItem;
};

struct DATA_EXPORT Selection::RemoveAction : Action {
    RemoveAction(uint32);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const uint32 mIdx;
    std::string mItem;
    bool mLastSelected;
};

} // namespace data

