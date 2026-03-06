#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/hierarchy/model.hpp
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
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "data/hierarchy/action.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

struct Action;
struct Node;
struct Root;

/**
 * Basis for data model structures.
 */
struct DATA_EXPORT Model {
    struct ROContext;
    struct Context;
    struct Receiver;
    template<typename>
    struct Responder;

    struct EnableAction;

    virtual ~Model();

    /**
     * If not overridden, asserts.
     */
    [[nodiscard]] virtual std::unique_ptr<Model> clone(Node *) const;

    /**
     * Generate a 64-bit ID from a string.
     */
    static uint64 strID(const std::string&);

    /**
     * Attach a receiver to this model.
     */
    void attachReceiver(Receiver&);
    void detachReceiver(Receiver&);

    /**
     * Process an action for non-UI. Waits until the action is accepted.
     */
    void processAction(std::unique_ptr<Action>&&);

    /**
     * Process a UI action. May fail depending on state.
     *
     * @return if the action was accepted.
     */
    [[nodiscard]] bool processUIAction(std::unique_ptr<Action>&&);

    template<typename T = Root>
    T *root() const {
        return static_cast<T *>(mRoot);
    }

    template<typename T = Node>
    T *parent() const {
        return static_cast<T *>(mParent);
    }

protected:
    friend Node;

    Model(Node * = nullptr, Root * = nullptr);
    Model(const Model&, Node * = nullptr, Root * = nullptr);
    
    template <typename SomeReceiver, typename... Args>
    void sendToReceivers(
        void (SomeReceiver::*mp)(Args...), const auto&... args
    ) {
        const auto lambda{[mp, &args...](Receiver *receiver) {
            if (auto *derived{dynamic_cast<SomeReceiver *>(receiver)}) {
                (derived->*mp)(args...);
            }
        }};
        sendToReceivers(lambda);
    }

    std::recursive_mutex pLock;

private:
    bool mEnabled{true};

    void sendToReceivers(const std::function<void(Receiver *)>&);
    bool processAction(std::unique_ptr<Action>&&, bool);

    std::set<Receiver *> mReceivers;

    Node *const mParent;
    Root *const mRoot;
};

struct DATA_EXPORT Model::ROContext {
    ROContext(const Model&);
    ~ROContext();

    ROContext(const ROContext&) = delete;
    ROContext(ROContext&&) = delete;
    ROContext& operator=(const ROContext&) = delete;
    ROContext& operator=(ROContext&&) = delete;

    template<typename T = Model>
    [[nodiscard]] const T& model() const {
        return static_cast<const T&>(mModel);
    }

    [[nodiscard]] bool enabled() const;

private:
    const Model& mModel;
};

struct DATA_EXPORT Model::Context : virtual ROContext {
    Context(Model&);
    ~Context();

    template<typename T = Model>
    [[nodiscard]] T& model() const {
        return const_cast<T&>(model<T>());
    }

    /**
     * (Dis)allow input to the data
     */
    void enable(bool en = true) const;
    void disable() const { enable(false); }

    /**
     * If the data has an associated UI, focus it.
     * This is a transient change. It only takes effect if UI is currently
     * bound.
     */
    void focus() const;
};

struct DATA_EXPORT Model::Receiver {
    Receiver();
    Receiver(const Receiver&);
    virtual ~Receiver();

    void attach(Model& model) {
        model.attachReceiver(*this);
    }

    void detach() {
        assert(mModel);
        mModel->detachReceiver(*this);
    }

protected:
    template<typename T = Model>
    T& model() const {
        assert(mModel);
        return static_cast<T&>(*mModel);
    }

    template<typename T = Model>
    typename T::Context context() const {
        assert(mModel);
        return typename T::Context(static_cast<T&>(*mModel));
    }
    
    /**
     * Whenever an action occurs, this should be called to (maybe) affect the
     * model.
     *
     * @return if the action was accepted. If not the control needs to reload.
     */
    [[nodiscard]] bool processAction(std::unique_ptr<Action>&&);

    /**
     * Receiver is attached to a new model.
     */
    virtual void onAttach() {}

    /**
     * Receiver is being detached from the model.
     * After this the model will be null'd
     */
    virtual void onDetach() {}

    /**
     * Called whenever the UI should be enabled.
     */
    virtual void onEnabled() {}

    /**
     * Focus the UI Element
     */
    virtual void onFocus() {}

private:
    friend Model;

    Model *mModel{nullptr};

    std::recursive_mutex mLock;
};

template<typename BaseModel>
struct DATA_EXPORT Model::Responder : BaseModel::Receiver {
    template <typename ...Args>
    using Function = void(*)(const typename BaseModel::Context&, Args...);

    Function<> onEnabled_;

private:
    void onEnabled() override {
        if (not onEnabled_) return;

        onEnabled_(Receiver::context<BaseModel>());
    }
};

struct DATA_EXPORT Model::EnableAction : Action {
    EnableAction(bool);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    static void enable(Model&, bool);

    const bool mEnable;
};

} // namespace data

