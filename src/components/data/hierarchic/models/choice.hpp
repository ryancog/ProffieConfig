#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/choice.hpp
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

#include "data/base/models/choice.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

struct DATA_EXPORT Choice : base::Choice, Model {
    struct ChoiceAction;
    struct UpdateAction;

    Choice(Root&);
    Choice(const Choice&, Root&);

    bool choose(int32) override;
    bool update(uint32, int32 = -1) override;

protected:
    uint64 hashThis() const override;
};

struct DATA_EXPORT Choice::ChoiceAction : Action {
    ChoiceAction(int32 choice);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    int32 mIdx;
};

struct DATA_EXPORT Choice::UpdateAction : Action {
    UpdateAction(uint32 num, int32 idx);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    uint32 mNum;
    int32 mIdx;
};

} // namespace data::hier

