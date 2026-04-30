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

namespace data {

struct RecvTable;

namespace base {

struct Model;

} // namespace base

struct Receiver {
    Receiver(Receiver&&) = delete;
    Receiver(const Receiver&) = delete;

    virtual ~Receiver();

    [[nodiscard]] bool active() const { return mAttached; }

    void activate();
    void deactivate();

protected:
    Receiver();

    virtual void onActivate() {}
    virtual void onDeactivate() {}

    /**
     * To be initialized by derived on creation.
     */
    std::map<const base::Model *, const RecvTable *> pRecvMap;

private:
    friend base::Model;

    bool mAttached{false};
};

} // namespace data

