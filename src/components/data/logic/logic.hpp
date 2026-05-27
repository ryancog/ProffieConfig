#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/logic/logic.hpp
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

#include <functional>
#include <memory>
#include <mutex>
#include <set>

#include "data_export.h"

// TODO: Clean this up.

namespace data::logic {

struct Manager;

namespace detail {

struct DATA_EXPORT Base {
    virtual ~Base();

    /**
     * Function child can use to alert of changes.
     */
    using ChangeFunc = std::function<void(bool)>;

    /**
     * Prevent models this depends on from changing
     */
    virtual bool tryLock() = 0;
    virtual void unlock() = 0;

protected:
    bool activate(detail::Base&, ChangeFunc);

    void onChange(bool);

    /**
     * Perform activation routine.
     */
    virtual bool doActivate() = 0;

    std::recursive_mutex *pLock{nullptr};

private:
    friend Manager;

    /**
     * Internal call to perform activation.
     * Calls doActivate()
     */
    bool activate(ChangeFunc, std::recursive_mutex *);

    ChangeFunc mChangeFunc{nullptr};
};

} // namespace detail

/**
 * Extension to enable ADL
 */
struct DATA_EXPORT Element : std::unique_ptr<detail::Base> {
    using unique_ptr::unique_ptr;

    /**
     * Otherwise it's very easy to miss including the operators and use the
     * standard boolean operator not()
     */
    operator bool() = delete;
};

struct Receiver;

struct DATA_EXPORT Manager {
    Manager(Element&&);

    void lock();
    void unlock();

    /**
     * Should only be accessed when locked.
     */
    [[nodiscard]] bool val() const;

private:
    friend Receiver;

    bool mVal;
    Element mChild;

    mutable std::recursive_mutex mLock;
    std::set<Receiver *> mReceivers;
};

/**
 * Shared ptr wrapper to hold in a descriptor.
 */
struct DATA_EXPORT Holder : std::shared_ptr<Manager> {
    using shared_ptr::shared_ptr;

    Holder(Element&&);

    /**
     * Allow directly assigning a constant to the holder.
     */
    Holder(bool);
};

struct DATA_EXPORT Receiver {
    virtual ~Receiver();

    /**
     * Attach and immediately call onChange to update.
     */
    void attach(Manager& man);

    void detach();

protected:
    friend Manager;

    virtual void onChange() = 0;

private:
    std::recursive_mutex mLock;
    Manager *mMan{nullptr};
};

} // namespace data::logic

