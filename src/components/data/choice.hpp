#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/choice.hpp
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
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

struct DATA_EXPORT Choice : Model {
    struct ROContext;
    struct Context;
    struct Receiver;
    struct Responder;

    struct ChoiceAction;
    struct UpdateAction;

    using Filter = void (*)(const Context&, int32&);

    Choice(Node * = nullptr);
    Choice(const Choice&, Node * = nullptr);
    ~Choice() override;

    std::unique_ptr<Model> clone(Node *) const override;

    void setFilter(Filter);

    [[nodiscard]] Responder& responder() const;

private:
    std::unique_ptr<Responder> mRsp;

    Filter mFilter;

    uint32 mNumChoices{0};
    int32 mIdx{-1};
};

struct DATA_EXPORT Choice::ROContext : virtual Model::ROContext {
    ROContext(const Choice&);
    ~ROContext();

    [[nodiscard]] uint32 numChoices() const;
    [[nodiscard]] int32 choice() const;
    
    /**
     * Whether or not this has a choice.
     */
    explicit operator bool() const;
};

struct DATA_EXPORT Choice::Context : Model::Context, ROContext {
    Context(Choice&);
    ~Context();

    /**
     * Choose a new choice.
     */
    void choose(int32) const;

    /**
     * Remove choice
     */
    void unchoose() const;

    /**
     * Update the number of choices available.
     */
    void update(uint32) const;
};

struct DATA_EXPORT Choice::Receiver : Model::Receiver {
protected:
    friend Choice;

    /**
     * Choice was changed.
     */
    virtual void onChoice() {}

    /**
     * Number of choices updated.
     */
    virtual void onUpdate() {}
};

struct DATA_EXPORT Choice::Responder : Model::Responder<Choice> {
    Function<> onChoice_;
    Function<> onUpdate_;

private:
    void onChoice() override {
        if (onChoice_) onChoice_(context<Choice>());
    }

    void onUpdate() override {
        if (onUpdate_) onUpdate_(context<Choice>());
    }
};

struct DATA_EXPORT Choice::ChoiceAction : Action {
    ChoiceAction(int32 choice);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    int32 mChoice;
    int32 mLast;
};

struct DATA_EXPORT Choice::UpdateAction : Action {
    UpdateAction(uint32 num);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const uint32 mNum;
    uint32 mLast;
    int32 mLastChoice;
};

} // namespace data

