#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/exclusive.hpp
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

#include "data/base/model.hpp"
#include "data/base/models/bool.hpp"
#include "data/receiver.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data::base {

struct DATA_EXPORT Exclusive : virtual Model, virtual Receiver {
    struct ROContext;
    struct Context;
    struct RecvTable;

    // TODO: This should go into a context.
    virtual bool select(size) = 0;

protected:
    // Separate init to access `create`.
    void init(size);

    virtual std::unique_ptr<Bool> create(size) = 0;

    bool setupSelect(size&);
    size doSelect(size);

private:
    void onSet(const Model&);

    size mSelected{0};
    std::vector<std::unique_ptr<Bool>> mData;
};

struct DATA_EXPORT Exclusive::ROContext : virtual Model::ROContext {
    ROContext(const Exclusive&);

    template <typename M = Exclusive>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] size num() const;
    [[nodiscard]] Bool& operator[](size idx) const;

    [[nodiscard]] size selected() const;
};

struct DATA_EXPORT Exclusive::Context : Model::Context, ROContext {
    Context(Exclusive&);

    template <typename M = Exclusive>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void select(size) const;
};

struct DATA_EXPORT Exclusive::RecvTable : Model::RecvTable {
    /**
     * New option was selected
     */
    Mapping<size> onSelection_;
};

} // namespace data::base

