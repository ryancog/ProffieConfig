#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/model.hpp
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
#include <set>

#include "data/recvtable.hpp"

#include "data_export.h"

namespace data {

struct Receiver;

namespace base {

/**
 * The base for data models.
 *
 * Model actions can be accessed either via a context or directly.
 * When accessing directly, the actions return a boolean value whether or not
 * the action actually changed anything.
 *
 * Within a context, the state is locked and unchanging excluding any changes
 * made by the context itself. State accessors are then meaningful and so are
 * only available via context. In addition, the actions are exposed for
 * convenience (without the change bool return value, since it's not valuable
 * there).
 */
struct DATA_EXPORT Model {
    struct ROContext;
    struct Context;
    struct RecvTable;

    Model();
    Model(const Model&);
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

    virtual ~Model();

    /**
     * (Dis)allow input to the data
     */
    virtual bool enable(bool en) = 0;
    bool disable() { return enable(false); }

    /**
     * If the data has an associated UI, focus it.
     * This is a transient change. It only takes effect if UI is currently
     * bound.
     */
    void focus();

    virtual void lock() const = 0;
    virtual void unlock() const = 0;

protected:
    bool setupEnable(bool&);
    void doEnable(bool);

    template <typename Table, typename... Args>
    void sendToReceivers(
        data::RecvTable::Mapping<Args...> Table::*mapping, const Args&... args
    ) const {
        const auto tryTable{[&](
            Receiver *receiver, const data::RecvTable *table
        ) {
            if (auto *derived{dynamic_cast<const Table *>(table)}) {
                auto variant{derived->*mapping};
                if (auto *ptr{std::get_if<0>(&variant)}) {
                    if (*ptr)
                        (*ptr)(receiver, args...);
                } else if (auto *ptr{std::get_if<1>(&variant)}) {
                    if (*ptr)
                        (*ptr)(receiver, *this, args...);
                }
            }
        }};
        sendToReceivers(tryTable);
    }

private:
    friend Receiver;

    void sendToReceivers(
        const std::function<void(Receiver *, const data::RecvTable *)>&
    ) const;

    bool mEnabled{true};
    mutable std::set<Receiver *> mReceivers;
};

struct DATA_EXPORT Model::ROContext {
    ROContext(const Model&);
    ~ROContext();

    ROContext(const ROContext&) = delete;
    ROContext(ROContext&&) = delete;
    ROContext& operator=(const ROContext&) = delete;
    ROContext& operator=(ROContext&&) = delete;

    void release();

    template <typename M = Model>
    [[nodiscard]] const M& model() const {
        // This is virtually inherited from, so dynamic cast is needed.
        return dynamic_cast<const M&>(*mModel);
    }

    [[nodiscard]] bool enabled() const;

private:
    const Model *mModel;
};

struct DATA_EXPORT Model::Context : virtual ROContext {
    Context(Model&);
    ~Context();

    template <typename M = Model>
    [[nodiscard]] M& model() const {
        return const_cast<M&>(ROContext::model<M>());
    }

    void enable(bool en = true) const;
    void disable() const { enable(false); }

    void focus() const;
};

struct DATA_EXPORT Model::RecvTable : data::RecvTable {
    Mapping<> onEnable_;
    Mapping<> onFocus_;
};

} // namespace base

} // namespace data

