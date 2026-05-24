#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/hierarchic/model.hpp
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

#include <cassert>
#include <memory>

#include "data/base/model.hpp"
#include "data/hierarchic/action.hpp"

#include "data_export.h"

namespace data::hier {

struct Root;

/**
 * Basis for data model structures.
 */
struct DATA_EXPORT Model : virtual base::Model {
    struct CreationScope;
    struct EnableAction;

    Model(Root&);
    Model(const Model&, Root&);

    Model(const Model &) = delete;

    template<typename T = Root>
    T& root() const {
        return static_cast<T&>(mRoot);
    }

    bool enable(bool en) override;

    void lock() const override;
    bool tryLock() const override;
    void unlock() const override;

    std::vector<Model *> children();
    virtual std::vector<const Model *> children() const;

protected:
    bool processAction(std::unique_ptr<Action>&&);
    
private:
    Root& mRoot;
};

struct DATA_EXPORT Model::EnableAction : Action {
    EnableAction(bool);

    bool setup() override;
    void perform() override;
    void retract() override;

private:
    bool mEnable;
};

struct Model::CreationScope {
    // ptr for convenience with `this`
    CreationScope(Model *);
    ~CreationScope();

private:
    Model& mModel;
};

} // namespace data::hier

