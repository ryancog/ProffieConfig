#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/vector.hpp
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

#include <memory>
#include <vector>

#include "data/base/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data::base {

struct DATA_EXPORT Vector : virtual Model {
    struct ROContext;
    struct Context;
    struct RecvTable;

    Vector() = default;
    Vector(const Vector&) = delete;

    /**
     * Insert model at pos.
     */
    virtual bool insert(size, std::unique_ptr<Model>&&) = 0;

    /**
     * Add model to end
     */
    bool append(std::unique_ptr<base::Model>&&);

    /**
     * Remove item at pos
     */
    virtual bool remove(size) = 0;

    /**
     * Move model up or down in the list.
     */
    bool moveUp(size);
    bool moveDown(size);

protected:
    virtual bool swap(size) = 0;

    bool setupInsert(size, const std::unique_ptr<Model>&);
    void doInsert(size, std::unique_ptr<Model>&&);

    bool setupRemove(size);
    std::unique_ptr<Model> doRemove(size);

    bool setupSwap(size);
    void doSwap(size);

private:
    std::vector<std::unique_ptr<Model>> mChildren;
};

struct DATA_EXPORT Vector::ROContext : virtual Model::ROContext {
    ROContext(const Vector&);

    template <typename M = Vector>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] const std::vector<std::unique_ptr<Model>>&
        children() const LIFETIMEBOUND;
};

struct DATA_EXPORT Vector::Context : Model::Context, ROContext {
    Context(Vector&);

    template <typename M = Vector>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void insert(size, std::unique_ptr<Model>&&) const;

    void append(std::unique_ptr<Model>&&) const;

    template <typename T, typename ...Args>
    T& append(Args&& ...args) const {
        auto obj{std::make_unique<T>(std::forward<Args>(args)...)};
        auto& ret{*obj};
        append(std::move(obj));
        return ret;
    }

    void remove(size) const;

    /**
     * Remove item by addr
     * @return if found and removed
     */
    bool remove(Model&) const;

    void moveUp(size) const;
    void moveDown(size) const;
};

struct DATA_EXPORT Vector::RecvTable : Model::RecvTable {
    /**
     * Model inserted at pos
     */
    Mapping<size> onInsert_;

    /**
     * Model removed from pos
     */
    Mapping<size> onRemove_;

    /**
     * Model at pos is about to be removed.
     */
    Mapping<size> preRemove_;

    /**
     * Models at pos and pos + 1 swapped.
     */
    Mapping<size> onSwap_;
};

} // namespace data::base

