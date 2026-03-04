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

#include "data_export.h"

namespace data {

namespace detail {

/**
 * An intentionally-flat structure for radio-style data.
 * Parent must register not only this, but also the child datas.
 */
struct DATA_EXPORT Exclusive : data::Model {
    struct Receiver;
    struct Responder;

    Exclusive(std::vector<std::unique_ptr<Bool>>, Node * = nullptr);
    Exclusive(const Exclusive&, Node * = nullptr);
    ~Exclusive() override;

    void attachReceiver(Receiver&);
    void detachReceiver(Receiver&);

    [[nodiscard]] std::span<const std::unique_ptr<Bool>> data() const;
    [[nodiscard]] Bool& operator[](size idx) const;

    // TODO: This should go into a context.
    [[nodiscard]] size selected();
    void select(size);

    [[nodiscard]] Responder& responder() const;

private:
    std::unique_ptr<Responder> mRsp;

    std::recursive_mutex mLock;
    size mSelected{0};

    std::vector<std::unique_ptr<Model::Receiver>> mRcvrs;
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

struct DATA_EXPORT Exclusive::Responder : Model::Responder<Exclusive> {
    Function<size> onSelection_;

private:
    void onSelection(size sel) override {
        if (onSelection_) onSelection_(context<Exclusive>(), sel);
    }
};

} // namespace detail

template<typename T = Bool>
struct DATA_EXPORT Exclusive : detail::Exclusive {
    Exclusive(size num, Node *parent = nullptr) :
        detail::Exclusive{[num, parent]() {
            std::vector<std::unique_ptr<Bool>> vec;
            for (size idx{0}; idx < num; ++idx) {
                vec.push_back(std::make_unique<Bool>(parent));
            }
            return vec;
        }(), parent} {};
};

} // namespace data

