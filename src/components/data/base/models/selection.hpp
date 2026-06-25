#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/selection.hpp
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

#include "data/base/model.hpp"
#include "data/recvtable.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data::base {

// TODO: This would probably like to be a more generic structure.
//       It is currently configured the way it is because it is used solely
//       for power pins, whose elements are intrinsically non-localized,
//       unique strings.
struct DATA_EXPORT Selection : virtual Model {
    struct DATA_EXPORT ROContext;
    struct DATA_EXPORT Context;
    struct DATA_EXPORT RecvTable;

    using AddFilter = void (*)(const ROContext&, std::string&);

    /**
     * @return true if the unselected value should be pruned.
     */
    using Pruner = bool (*)(const ROContext&, uint32);

    Selection() = default;
    Selection(const Selection&);

    void setAddFilter(AddFilter);
    void setPruner(Pruner);

    /**
     * Select item at idx.
     */
    virtual bool select(uint32 idx, bool select = true) = 0;

    /**
     * Select first item with string. Create new item and select it if none
     * exists.
     */
    virtual bool select(std::string&&) = 0;

    /**
     * Assign a new set of items to the selection. Clears previous selection.
     */
    virtual bool setItems(std::vector<std::string>&&) = 0;

    /**
     * Add a new item w/ string.
     */
    virtual bool add(std::string&&) = 0;

    /**
     * Remove item at idx.
     */
    virtual bool remove(uint32) = 0;

protected:
    bool setupSelect(uint32, bool&);
    void doSelect(bool undo, uint32, bool);

    bool setupSetItems(std::vector<std::string>&);
    std::pair<std::vector<std::string>, std::vector<bool>>
    doSetItems(bool undo, std::vector<std::string>&&, std::vector<bool> = {});

    int32 findString(const std::string&) const;
    bool isSelected(uint32);

    bool setupInsert(uint32, std::string&);
    void doInsert(bool removeUndo, uint32, std::string&&, bool sel = true);

    bool setupRemove(uint32);
    std::pair<std::string, bool> doRemove(bool insertUndo, uint32);

private:
    AddFilter mAddFilter{nullptr};
    Pruner mPruner{nullptr};
    std::vector<std::string> mItems;
    std::vector<bool> mSelected;
};

struct DATA_EXPORT Selection::ROContext : virtual Model::ROContext {
    ROContext(const Selection&);

    template <typename M = Selection>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] const std::vector<bool>& selected() const LIFETIMEBOUND;
    [[nodiscard]] const std::vector<std::string>& items() const LIFETIMEBOUND;
};

struct DATA_EXPORT Selection::Context : Model::Context, ROContext {
    Context(Selection&);

    template <typename M = Selection>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void select(uint32 idx, bool select = true) const;
    void deselect(uint32 idx) const { select(idx, false); }

    void select(std::string&&) const;

    void setItems(std::vector<std::string>&&) const;
    void remove(uint32) const;
};

struct DATA_EXPORT Selection::RecvTable : Model::RecvTable {
    /**
     * Item is (de)selected.
     */
    Mapping<uint32> onSelection_;

    /**
     * Items are completely changed.
     */
    Mapping<> onItems_;

    /**
     * Item Inserted
     */
    Mapping<uint32> onInsert_;

    /**
     * Item Removed
     */
    Mapping<uint32> onRemove_;
};

} // namespace data::base

