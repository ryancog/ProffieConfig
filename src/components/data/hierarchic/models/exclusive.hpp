#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/exclusive.hpp
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

#include "data/base/models/exclusive.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

struct DATA_EXPORT Exclusive : base::Exclusive, Model {
    struct DATA_EXPORT SelectAction;

    Exclusive(Root&, size);

    using Model::children;
    std::vector<const Model *> children() const override;

    bool select(size) override;

protected:
    /**
     * For derived, which will call init() itself.
     */
    Exclusive(Root&);

    uint64 hashThis() const override;

private:
    std::unique_ptr<base::Bool> create(size) override;
};

struct DATA_EXPORT Exclusive::SelectAction : Action {
    SelectAction(size);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    size mIdx;
};

} // namespace data::hier

