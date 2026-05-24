#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/vector.hpp
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

#include "data/base/models/vector.hpp"
#include "data/hierarchic/model.hpp"

#include "data_export.h"

namespace data::hier {

struct DATA_EXPORT Vector : base::Vector, Model {
    struct InsertAction;
    struct RemoveAction;
    struct SwapAction;

    Vector(Root&);

    using Model::children;
    std::vector<const Model *> children() const override;

    bool insert(size, std::unique_ptr<base::Model>&&) override;
    bool remove(size) override;
    bool swap(size) override;
};

struct DATA_EXPORT Vector::InsertAction : Action {
    InsertAction(size, std::unique_ptr<base::Model>&&);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const size mPos;
    std::unique_ptr<base::Model> mModel;
};

struct DATA_EXPORT Vector::RemoveAction : Action {
    RemoveAction(size);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const size mPos;
    std::unique_ptr<base::Model> mModel;
};

struct DATA_EXPORT Vector::SwapAction : Action {
    SwapAction(size);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    const size mPos;
};

} // namespace data::hier

