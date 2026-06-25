#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/choice.hpp
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
#include "utils/types.hpp"

#include "data_export.h"

namespace data::base {

struct DATA_EXPORT Choice : virtual Model {
    struct DATA_EXPORT ROContext;
    struct DATA_EXPORT Context;
    struct DATA_EXPORT RecvTable;

    using Filter = void (*)(const ROContext&, int32&);

    struct UpdateInfo {
        bool choicePreserved_;
    };

    Choice() = default;
    Choice(const Choice&);

    void setFilter(Filter);

    /**
     * Choose a new choice.
     */
    virtual bool choose(int32) = 0;

    bool unchoose() { return choose(-1); }

    /**
     * Update the number of choices available.
     *
     * Optionally provide new index.
     */
    virtual bool update(uint32, int32 = -1) = 0;

protected:
    bool setupChoose(int32&);
    int32 doChoose(bool undo, int32);

    bool setupUpdate(uint32, int32&);
    std::pair<uint32, int32> doUpdate(bool undo, uint32, int32);

private:
    Filter mFilter{nullptr};

    int32 mIdx{-1};
    uint32 mNum{0};
};

struct DATA_EXPORT Choice::ROContext : virtual Model::ROContext {
    ROContext(const Choice&);

    template <typename M = Choice>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] int32 idx() const;
    [[nodiscard]] uint32 num() const;
};

struct DATA_EXPORT Choice::Context : Model::Context, ROContext {
    Context(Choice&);

    template <typename M = Choice>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void choose(int32) const;
    void unchoose() const { choose(-1); }

    void update(uint32, int32 = -1) const;
};

struct DATA_EXPORT Choice::RecvTable : Model::RecvTable {
    Mapping<> preChoice_;

    Mapping<> onChoice_;

    Mapping<UpdateInfo> onUpdate_;
};

} // namespace data::base

