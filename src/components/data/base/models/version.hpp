#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/version.hpp
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
#include "utils/version.hpp"

#include "data_export.h"

namespace data::base {

struct DATA_EXPORT Version : virtual Model {
    struct DATA_EXPORT ROContext;
    struct DATA_EXPORT Context;
    struct DATA_EXPORT RecvTable;

    Version() = default;
    Version(const Version&);

    virtual bool set(utils::Version&&) = 0;

protected:
    bool setupSet(utils::Version&);
    utils::Version doSet(bool undo, utils::Version&&);

private:
    utils::Version mVer;
};

struct DATA_EXPORT Version::ROContext : virtual Model::ROContext {
    ROContext(const Version&);

    template <typename M = Version>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] const utils::Version& val() const LIFETIMEBOUND;
};

struct DATA_EXPORT Version::Context : Model::Context, ROContext {
    Context(Version&);

    template <typename M = Version>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void set(utils::Version&&) const;
};

struct DATA_EXPORT Version::RecvTable : Model::RecvTable {
    /**
     * Version changed.
     */
    Mapping<> onSet_;
};

} // namespace data::base

