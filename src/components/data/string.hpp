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

#include <string>

#include "data/hierarchy/model.hpp"

#include "data_export.h"

namespace data {

// TODO: The actions for this would like to be more space efficient, and
// preserve things like selection??
struct DATA_EXPORT String : Model {
    struct ROContext;
    struct Context;
    struct Receiver;
    struct Responder;

    struct ChangeAction;
    struct MoveAction;

    using Filter = void (*)(const Context&, std::string&, size&);

    String(Node * = nullptr);
    String(const String&, Node * = nullptr);
    ~String() override;

    std::unique_ptr<Model> clone(Node *) const override;

    void setFilter(Filter);

    [[nodiscard]] Responder& responder() const;

private:
    std::unique_ptr<Responder> mRsp;

    std::string mValue;
    size mPos{0};

    Filter mFilter;
};

struct DATA_EXPORT String::ROContext : virtual Model::ROContext {
    ROContext(const String&);
    ~ROContext();

    [[nodiscard]] const std::string& val() const [[clang::lifetimebound]];
    [[nodiscard]] size pos() const;
};

struct DATA_EXPORT String::Context : Model::Context, ROContext {
    Context(String&);
    ~Context();

    /**
     * Replace text with new text and pos.
     */
    void change(std::string&&, size) const;
    void change(std::string&&) const;

    void append(char) const;
    void append(std::string_view) const;

    void clear() const;

    /**
     * Move "cursor"
     * Position represents insertion point, or deletion point + 1
     */
    void move(size) const;
    void moveStart() const;
    void moveEnd() const;
};

struct DATA_EXPORT String::Receiver : Model::Receiver {
protected:
    friend String;

    /**
     * Text is changed
     */
    virtual void onChange() {}

    /**
     * Cursor position is moved.
     */
    virtual void onMove() {}
};

struct DATA_EXPORT String::Responder : Model::Responder<String> {
    Function<> onChange_;
    Function<> onMove_;

private:
    void onChange() override {
        if (onChange_) onChange_(context<String>());
    }

    void onMove() override {
        if (onMove_) onMove_(context<String>());
    }
};

struct DATA_EXPORT String::ChangeAction : Action {
    ChangeAction(std::string&&, size);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    std::string mStr;
    size mPos;
};

struct DATA_EXPORT String::MoveAction : Action {
    MoveAction(size);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    size mPos;
};

} // namespace data

