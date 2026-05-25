#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/string.hpp
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

#include "data/base/models/string.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

struct DATA_EXPORT String : base::String, Model {
    struct ChangeAction;
    struct MoveAction;

    String(Root&);
    String(const String&, Root&);

    using base::String::change;
    bool change(std::string&&, size) override;
    bool move(size) override;

protected:
    uint64 hashThis() const override;
};

struct DATA_EXPORT String::ChangeAction : Action {
    ChangeAction(std::string&&, size);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    std::string mStr;
    size mPos;
};

struct DATA_EXPORT String::MoveAction : Action {
    MoveAction(size);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    size mPos;
};

} // namespace data::hier

