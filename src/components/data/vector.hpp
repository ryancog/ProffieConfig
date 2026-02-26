#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/vector.hpp
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

#include "data_export.h"

namespace data {

struct DATA_EXPORT Vector : Node {
    struct Context;
    struct Receiver;

    struct InsertAction;
    struct RemoveAction;
    struct SwapAction;

    Vector(Node * = nullptr);
    Vector(const Vector&, Node * = nullptr);

    std::unique_ptr<Model> clone(Node *) const override;

protected:
    bool enumerate(const EnumFunc&) override;

    Model *find(uint64) override;

private:
    vector<std::unique_ptr<Model>> mChildren;
};

struct DATA_EXPORT Vector::Context : Node::Context {
    enum class DuplicationMode {
        /**
         * Append duplicated item to the end of the list.
         */
        Append,
        /**
         * Insert duplicated item into the list after the source item.
         */
        Insert,
    };

    Context(Vector&);
    ~Context();

    /**
     * Insert model into list at pos.
     */
    void insert(size, std::unique_ptr<Model>&&);

    /**
     * Remove item at pos
     */
    void remove(size);

    /**
     * Move model up or down in the list.
     */
    void moveUp(size);
    void moveDown(size);

    void duplicate(size, DuplicationMode);

    [[nodiscard]] const vector<std::unique_ptr<Model>>&
        children() const [[clang::lifetimebound]];
};

struct DATA_EXPORT Vector::Receiver : Node::Receiver {
protected:
    friend Vector;

    /**
     * Model inserted at pos
     */
    virtual void onInsert(size) {}

    /**
     * Model removed from pos
     */
    virtual void onRemove(size) {}

    /**
     * Model at pos is about to be removed.
     */
    virtual void preRemove(size) {}

    /**
     * Models at pos and pos + 1 swapped.
     */
    virtual void onSwap(size) {}
};

struct DATA_EXPORT Vector::InsertAction : Action {
    InsertAction(size, std::unique_ptr<Model>&&);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const size mPos;
    std::unique_ptr<Model> mModel;
};

struct DATA_EXPORT Vector::RemoveAction : Action {
    RemoveAction(size);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const size mPos;
    std::unique_ptr<Model> mModel;
};

struct DATA_EXPORT Vector::SwapAction : Action {
    SwapAction(size);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const size mPos;
};

} // namespace data


