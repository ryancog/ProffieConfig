#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/selector.hpp
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

#include "data/base/models/selector.hpp"
#include "data/hierarchic/models/choice.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

struct DATA_EXPORT Selector : base::Selector, Model {
    struct DATA_EXPORT BindAction;

    Selector(Root&);
    Selector(const Selector&, Root&);

    hier::Choice& choice() const override;
    bool bind(const base::Vector *) override;

protected:
    void setupVecRecv(const base::Vector *) override;

    uint64 hashThis() const override;

private:
    mutable Choice mChoice;
};

struct DATA_EXPORT Selector::BindAction : Action {
    BindAction(const base::Vector *);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const base::Vector *mVec;
};

} // namespace data::hier

