#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/selection.hpp
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

#include "data/base/models/selection.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

struct DATA_EXPORT Selection : base::Selection, Model {
    struct SelectAction;
    struct SetItemsAction;
    struct InsertAction;
    struct RemoveAction;

    Selection(Root&);
    Selection(const Selection&, Root&);

    bool select(uint32 idx, bool select = true) override;
    bool select(std::string&&) override;
    bool setItems(std::vector<std::string>&&) override;
    bool add(std::string&&) override;
    bool remove(uint32) override;

protected:
    uint64 hashThis() const override;
};

struct DATA_EXPORT Selection::SelectAction : Action {
    SelectAction(uint32, bool);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const uint32 mIdx;
    bool mSelect;
};

struct DATA_EXPORT Selection::SetItemsAction : Action {
    SetItemsAction(std::vector<std::string>&&);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    std::vector<std::string> mItems;
    std::vector<bool> mSelected;
};

struct DATA_EXPORT Selection::InsertAction : Action {
    InsertAction(uint32, std::string&&);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const uint32 mIdx;
    std::string mItem;
};

struct DATA_EXPORT Selection::RemoveAction : Action {
    RemoveAction(uint32);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const uint32 mIdx;
    std::string mItem;
    bool mSelected;
};

} // namespace data::hier

