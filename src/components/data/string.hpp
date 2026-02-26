#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/string.hpp
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

#include "data/hierarchy/model.hpp"

#include "data_export.h"

namespace data {

struct DATA_EXPORT String : Model {
    struct Context;
    struct Receiver;

    struct InsertAction;
    struct RemoveAction;
    struct MoveAction;

    String(Node * = nullptr);
    String(const String&, Node * = nullptr);

    std::unique_ptr<Model> clone(Node *) const override;

protected:
    string pValue;
    size pPos{0};
};

struct DATA_EXPORT String::Context : Model::Context {
    Context(String&);
    ~Context();

    /**
     * Insert text at the "cursor" position.
     */
    void insert(string);

    /**
     * Remove text, starting from the "cursor" position.
     */
    void remove(size);
    void clear();

    /**
     * Move "cursor"
     * Position represents insertion point, or deletion point + 1
     */
    void move(size);
    void moveStart();
    void moveEnd();

    [[nodiscard]] const string& val() const [[clang::lifetimebound]];
    [[nodiscard]] size pos() const;
};

struct DATA_EXPORT String::Receiver : Model::Receiver {
protected:
    friend String;

    /**
     * Text is changed
     */
    virtual void onChange(const string&) {}

    /**
     * Cursor position is moved.
     */
    virtual void onMove(size) {}
};

struct DATA_EXPORT String::InsertAction : Action {
    InsertAction(string&&);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const string mStr;
};

struct DATA_EXPORT String::RemoveAction : Action {
    RemoveAction(size);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const size mNum;
    string mRemoved;
};

struct DATA_EXPORT String::MoveAction : Action {
    MoveAction(size);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const size mPos;
    size mLast;
};

} // namespace data

