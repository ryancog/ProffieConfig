#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/receiver.hpp
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

#include <map>
#include <mutex>

#include "data_export.h"

namespace data {

struct RecvTable;

namespace base {

struct Model;

} // namespace base

namespace hier {

struct Model;

} // namespace hier

struct DATA_EXPORT Receiver {
    static void maybeActivate(base::Model *);
    static void maybeDeactivate(base::Model *);

    Receiver(Receiver&&) = delete;
    Receiver(const Receiver&) = delete;

    virtual ~Receiver();

    [[nodiscard]] bool active() const { return mAttached; }

    void activate();
    void deactivate();

    void amend(const base::Model&, const RecvTable&);
    void repeal(const base::Model&);

protected:
    using RecvMap = std::map<const base::Model *, const RecvTable *>;

    Receiver();

    virtual void onActivate() {}
    virtual void onDeactivate() {}

    mutable std::recursive_mutex pMutex;

private:
    friend base::Model;

    static void activateHierarchic(hier::Model *);
    static void deactivateHierarchic(hier::Model *);

    /**
     * To be initialized by derived on creation.
     */
    RecvMap mRecvMap;
    bool mAttached{false};
};

} // namespace data

