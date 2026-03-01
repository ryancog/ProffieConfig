#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/selector.hpp
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

#include "data/hierarchy/node.hpp"
#include "data/choice.hpp"
#include "data/generic.hpp"
#include "data/vector.hpp"

#include "data_export.h"

namespace data {

struct DATA_EXPORT Selector final : Node, Choice::Receiver, Vector::Receiver {
    struct Context;
    struct Receiver;

    struct BindAction;

    Selector(Node * = nullptr);
    Selector(const Selector&, Node * = nullptr);
    ~Selector() override;

    [[nodiscard]] std::unique_ptr<Model> clone(Node *) const override;

    Choice choice_;

    /**
     * For each of these, dis/enable is managed by the selector, and should not
     * be modified manually.
     *
     * TODO: A way to enforce that?
     */
    Generic moveUp_;
    Generic moveDown_;
    Generic duplicate_;

protected:
    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

private:
    void onChoice() override;

    void onInsert(size) override;
    void onRemove(size) override;
    void onSwap(size) override;

    Vector *mVec{nullptr};
};

struct DATA_EXPORT Selector::Context : Node::Context {
    Context(Selector&);
    ~Context();

    /**
     * Bind a different vector
     */
    void bind(Vector *) const;

    /**
     * Currently-bound vector
     */
    [[nodiscard]] Vector *bound() const;
};

struct DATA_EXPORT Selector::Receiver : Model::Receiver {
protected:
    friend Selector;

    /**
     * Selector bound to a different Vector
     */
    virtual void onRebound(Vector *) {}
};

struct DATA_EXPORT Selector::BindAction : Action {
    BindAction(Vector *);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    Vector *const mVec;
    Vector *mLast;
    int32 mLastSel;
};

} // namespace data

