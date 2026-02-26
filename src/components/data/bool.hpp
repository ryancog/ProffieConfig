#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/bool.hpp
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

struct DATA_EXPORT Bool final : Model {
    struct Context;
    struct Receiver;

    struct SetAction;

    Bool(Node * = nullptr);
    Bool(const Bool&, Node * = nullptr);

    std::unique_ptr<Model> clone(Node *) const override;

private:
    bool mValue{false};
};

struct DATA_EXPORT Bool::Context : Model::Context {
    Context(Bool&);
    ~Context();

    void set(bool);
    void operator|=(bool);

    [[nodiscard]] bool val() const;
};

struct DATA_EXPORT Bool::Receiver : Model::Receiver {
protected:
    friend Bool;

    virtual void onSet(bool) {}
};

struct DATA_EXPORT Bool::SetAction : Action {
    SetAction(bool);
    
    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const bool mValue;
};

} // namespace data

