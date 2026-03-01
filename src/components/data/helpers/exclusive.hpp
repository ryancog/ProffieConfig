#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/helpers/exclusive.hpp
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

#include <memory>
#include <span>
#include <vector>

#include "data/bool.hpp"

#include "data/helpers/rcvr_manager.hpp"
#include "data_export.h"

namespace data {

/**
 * An intentionally-flat structure for radio-style data.
 * Parent must register not only this, but also the child datas.
 */
struct DATA_EXPORT Exclusive : data::pModel, data::RcvrManager {
    struct Receiver;
    struct FuncRcvr;

    Exclusive(size, Node * = nullptr);
    Exclusive(const Exclusive&, Node * = nullptr);

    void attachReceiver(Receiver&);
    void detachReceiver(Receiver&);

    [[nodiscard]] std::span<const std::unique_ptr<Bool>> data() const;
    [[nodiscard]] Bool& operator[](size idx) const;

    std::unique_ptr<Model> clone(Node *) const override;

private:
    std::recursive_mutex mLock;
    std::set<Receiver *> mReceivers;
    std::vector<std::unique_ptr<Bool>> mData;
};

struct DATA_EXPORT Exclusive::Receiver : Model::Receiver {
protected:
    friend Exclusive;

    /**
     * New option was selected
     */
    virtual void onSelection(size) {}
};

struct DATA_EXPORT Exclusive::FuncRcvr : Model::FuncRcvr<Exclusive::Receiver> {
    std::function<void(size)> onSelection_;

private:
    void onSelection(size sel) override {
        if (onSelection_) onSelection_(sel);
    }
};

} // namespace data

