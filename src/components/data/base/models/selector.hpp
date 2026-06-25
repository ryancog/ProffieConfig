#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/selector.hpp
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
#include "data/base/models/choice.hpp"
#include "data/base/models/vector.hpp"
#include "data/receiver.hpp"

#include "data_export.h"

namespace data::base {

struct DATA_EXPORT Selector : virtual Model, virtual Receiver {
    struct DATA_EXPORT ROContext;
    struct DATA_EXPORT Context;
    struct DATA_EXPORT RecvTable;

    Selector() = default;
    Selector(const Selector&) = delete;

    virtual Choice& choice() const = 0;

    /**
     * Bind a different vector
     */
    virtual bool bind(const Vector *) = 0;

protected:
    void init();

    bool setupBind(const Vector *);
    const Vector *doBind(bool undo, const Vector *);

    virtual void setupVecRecv(const base::Vector *);

    static bool canMoveUp(const Choice::ROContext&);
    static bool canMoveDown(const Choice::ROContext&);

    static const Vector::RecvTable VEC_TABLE;

private:
    void onChoice();

    void onInsert(size);
    void preRemove(size);
    void onRemove(size);
    void onSwap(size);

    bool mLastCanMoveUp{false};
    bool mLastCanMoveDown{false};

    const Vector *mVec{nullptr};

    static const Choice::RecvTable CHOICE_TABLE;
};

struct DATA_EXPORT Selector::ROContext : virtual Model::ROContext {
    ROContext(const Selector&);
    ~ROContext() override;

    template <typename M = Selector>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    void release() override;

    /**
     * Currently-bound vector
     */
    [[nodiscard]] const Vector *bound() const;

    [[nodiscard]] int32 choiceIdx() const;

    template <typename T = Model>
    [[nodiscard]] T *selected() const {
        return dynamic_cast<T *>(selectedImpl());
    }

    [[nodiscard]] bool canMoveUp() const;
    [[nodiscard]] bool canMoveDown() const;

private:
    [[nodiscard]] Model *selectedImpl() const;
};

struct DATA_EXPORT Selector::Context : Model::Context, ROContext {
    Context(Selector&);

    template <typename M = Selector>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void bind(const Vector *) const;
};

struct DATA_EXPORT Selector::RecvTable : Model::RecvTable {
    Mapping<> preRebound_;

    /**
     * Selector bound to a different Vector
     */
    Mapping<> onRebound_;

    Mapping<> onCanMoveUp_;
    Mapping<> onCanMoveDown_;
};

} // namespace data::base

