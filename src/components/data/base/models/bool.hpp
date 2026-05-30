#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/bool.hpp
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

#include "data_export.h"

namespace data::base {

struct DATA_EXPORT Bool : virtual Model {
    struct ROContext;
    struct Context;
    struct RecvTable;

    using Filter = void (*)(const ROContext&, bool&);

    Bool() = default;
    Bool(const Bool&);

    void setFilter(Filter);

    virtual bool set(bool) = 0;

protected:
    bool setupSet(bool&);
    void doSet(bool undo, bool);

private:
    Filter mFilter{nullptr};
    bool mValue{false};
};

struct DATA_EXPORT Bool::ROContext : virtual Model::ROContext {
    ROContext(const Bool&);

    template <typename M = Bool>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] bool val() const;
};

struct DATA_EXPORT Bool::Context : Model::Context, ROContext {
    Context(Bool&);

    template <typename M = Bool>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void set(bool) const;
    void operator|=(bool) const;
};

struct DATA_EXPORT Bool::RecvTable : Model::RecvTable {
    Mapping<> onSet_;
};

} // namespace data::base

