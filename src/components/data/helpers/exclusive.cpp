#include "exclusive.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/helpers/exclusive.cpp
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

data::detail::Exclusive::Exclusive(
    std::vector<std::unique_ptr<data::Bool>> data, Node *parent
) : mData{std::move(data)} {
    assert(mData.size() > 1);
    for (auto& data : mData) {
        assert(data->parent() == parent);
    }

    mRsp = std::make_unique<Responder>();
    mRsp->attach(*this);

    struct Receiver : Bool::Receiver {
        Receiver(Exclusive& excl) : excl_{excl} {}

        void onSet() override {
            if (not context<Bool>().val()) return;
            std::lock_guard scopeLock{excl_.mLock};

            size selIdx{};
            for (size idx{0}; idx < excl_.mData.size(); ++idx) {
                auto& testBl{*excl_.mData[idx]};

                if (&testBl == &model<Bool>()) {
                    selIdx = idx;
                    continue;
                }

                Bool::Context bl{testBl};
                bl.set(false);
            }

            excl_.sendToReceivers(&Exclusive::Receiver::onSelection, selIdx);
        }

        Exclusive& excl_;
    };

    mRcvrs.reserve(mData.size());
    for (size idx{0}; idx < mData.size(); ++idx) {
        auto rcvr{std::make_unique<Receiver>(*this)};
        rcvr->attach(*mData[idx]);
        mRcvrs.push_back(std::move(rcvr));
    }

    Bool::Context{*mData[0]}.set(true);
}

data::detail::Exclusive::Exclusive(const Exclusive& other, Node *parent) :
    data::Model(other, parent) {

    mRsp = std::make_unique<Responder>(*other.mRsp);
    mRsp->attach(*this);

    mSelected = other.mSelected;
    Bool::Context{*mData[mSelected]}.set(true);
}

data::detail::Exclusive::~Exclusive() {
    mRsp->detach();
    for (auto& rcvr : mRcvrs) {
        rcvr->detach();
    }
}

std::span<const std::unique_ptr<data::Bool>> data::detail::Exclusive::data(
) const {
    return mData;
}

data::Bool& data::detail::Exclusive::operator[](size idx) const {
    return *data()[idx];
}

size data::detail::Exclusive::selected() {
    std::lock_guard scopeLock{pLock};
    return mSelected;
}

void data::detail::Exclusive::select(size pos) {
    std::lock_guard scopeLock{pLock};

    assert(pos < mData.size());
    data::Bool::Context{*mData[pos]}.set(true);
}

auto data::detail::Exclusive::responder() const -> Responder& { return *mRsp; }

