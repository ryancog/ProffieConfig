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

#include <set>

#include "data/recvtable.hpp"
#include "utils/hash.hpp"
#include "utils/types.hpp"

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
    virtual bool tryLock() const = 0;
    virtual void unlock() const = 0;

    /**
     * Store reference data with the model.
     */
    void *ref_{nullptr};

    template <typename T>
    T& refAs() const {
        return *reinterpret_cast<T *>(ref_);
    }

protected:
    bool setupEnable(bool&);
    void doEnable(bool);

    struct RecvTableBinding {
        constexpr RecvTableBinding(uint64 id) : id_{id} {}

        virtual ~RecvTableBinding() = default;
        virtual void tryTable(Receiver&, const data::RecvTable&) const = 0;

        uint64 id_;
    };

    template <auto MEM_PTR>
    constexpr void sendToObservers(const auto&... args) const {
        BindingImpl<MEM_PTR> binding(*this, args...);
        sendToObservers(binding);
    }

    template <auto MEM_PTR>
    constexpr void responderHook(const auto&... args) const {
        BindingImpl<MEM_PTR> binding(*this, args...);
        responderHook(binding);
    }

    virtual void sendToObservers(const RecvTableBinding&) const;
    virtual void responderHook(const RecvTableBinding&) const;

    const std::set<Receiver *>& receivers() const;

private:
    friend Receiver;

    template <auto MEM_PTR>
    struct BindingImpl;

    template <
        typename Table,
        typename ...Args,
        data::RecvTable::Mapping<Args...> Table::*MEM_PTR
    >
    struct BindingImpl<MEM_PTR> : RecvTableBinding {
        BindingImpl(const Model& model, const Args&... args) :
            RecvTableBinding(
                utils::hash::combine(
                    typeid(Table).hash_code(),
                    utils::hash::single(MEM_PTR)
                )
            ), mLambda{lambda(model, args...)} {}

        void tryTable(
            Receiver& rcvr, const data::RecvTable& table
        ) const override {
            mLambda(rcvr, table);
        }

    private:
        static auto lambda(const Model& model, const Args&... args) {
            return [&model, &args...](
                Receiver& receiver, const data::RecvTable& table
            ) {
                if (auto *derived{dynamic_cast<const Table *>(&table)}) {
                    auto mapping{derived->*MEM_PTR};
                    if (mapping.func_)
                        mapping.func_(model, receiver, args...);
                }
            };
        };

        decltype(lambda(
            std::declval<Model&>(), std::declval<const Args&>()...
        )) mLambda;
    };

    bool mEnabled{true};
    mutable std::set<Receiver *> mReceivers;
};

struct DATA_EXPORT Model::ROContext {
    ROContext(const Model&);
    virtual ~ROContext();

    ROContext(const ROContext&) = delete;
    ROContext(ROContext&&) = delete;
    ROContext& operator=(const ROContext&) = delete;
    ROContext& operator=(ROContext&&) = delete;

    virtual void release();
    [[nodiscard]] bool released() const;

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
    ~Context() override;

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

