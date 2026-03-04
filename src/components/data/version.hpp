#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/version.hpp
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
#include "utils/version.hpp"

#include "data_export.h"
#include <memory>

namespace data {

struct DATA_EXPORT Version : Model {
    struct Context;
    struct Receiver;
    struct Responder;

    struct SetAction;

    Version(Node * = nullptr);
    Version(const Version&, Node * = nullptr);
    ~Version() override;

    std::unique_ptr<Model> clone(Node *) const override;

    [[nodiscard]] Responder& responder() const;

private:
    std::unique_ptr<Responder> mRsp;
    utils::Version mValue;
};

struct DATA_EXPORT Version::Context : Model::Context {
    Context(Version&);
    ~Context();

    void set(utils::Version) const;
    [[nodiscard]] const utils::Version& val() const [[clang::lifetimebound]];
};

struct DATA_EXPORT Version::Receiver : Model::Receiver {
protected:
    friend Version;

    /**
     * Version changed.
     */
    virtual void onSet() {}
};

struct DATA_EXPORT Version::Responder : Model::Responder<Version> {
    Function<> onSet_;

private:
    void onSet() override {
        if (onSet_) onSet_(context<Version>());
    }
};

struct DATA_EXPORT Version::SetAction : Action {
    SetAction(utils::Version);
    
    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    utils::Version mValue;
};

} // namespace data

