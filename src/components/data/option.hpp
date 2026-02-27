#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/option.hpp
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

#include <vector>

#include "data/hierarchy/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

struct DATA_EXPORT Option : Model {
    struct Context;
    struct Receiver;

    struct ShowAction;
    struct EnableAction;
    struct SelectAction;

    Option(uint32 num, Node * = nullptr);
    Option(const Option&, Node * = nullptr);

    std::unique_ptr<Model> clone(Node *) const override;

private:
    std::vector<bool> mEnabled;
    std::vector<bool> mShown;
    uint32 mSelected;
};

struct DATA_EXPORT Option::Context : Model::Context {
    Context(Option&);
    ~Context();

    /**
     * Select option with index
     */
    void select(uint32);

    /**
     * Choices which are enabled will be shown regardless of their shown value
     */
    void showOpt(uint32 idx, bool show = true);
    void hideOpt(uint32 idx) { showOpt(idx, false); }

    void enableOpt(uint32 idx, bool enable = true);
    void disableOpt(uint32 idx) { enableOpt(idx, false); }

    /**
     * The number of options this model is configured to.
     */
    [[nodiscard]] uint32 num() const;
    [[nodiscard]] uint32 selected() const;

    [[nodiscard]] const std::vector<bool>&
        optEnabled() const [[clang::lifetimebound]];
    [[nodiscard]] const std::vector<bool>&
        optShown() const [[clang::lifetimebound]];
};

struct DATA_EXPORT Option::Receiver : Model::Receiver {
protected:
    friend Option;

    /**
     * Option is selected.
     */
    virtual void onSelection(uint32) {}

    /**
     * Option is now shown/hidden.
     * 
     * If an option was previously hidden, this is called automatically after
     * an opt is enabled, even if it is set to be hidden otherwise.
     */
    virtual void onOptShown(uint32, bool) {}

    /**
     * Option is now enabled.
     */
    virtual void onOptEnabled(uint32, bool) {}
};

struct DATA_EXPORT Option::ShowAction : Action {
    ShowAction(uint32, bool);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const uint32 mOpt;
    const bool mShown;
};

struct DATA_EXPORT Option::EnableAction : Action {
    EnableAction(uint32, bool);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const uint32 mOpt;
    const bool mEnabled;
};

struct DATA_EXPORT Option::SelectAction : Action {
    SelectAction(uint32);

    bool shouldPerform(Model&) override;
    void perform(Model&) override;
    void retract(Model&) override;

private:
    const uint32 mOpt;
    uint32 mLast;
};

} // namespace data

