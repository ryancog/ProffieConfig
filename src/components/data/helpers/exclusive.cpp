#include "exclusive.hpp"
#include <mutex>
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

data::Exclusive::Exclusive(size num, Node *parent) {
    assert(num > 1);

    struct Receiver : Bool::Receiver {
        Receiver(Exclusive& excl, Bool& bl) : excl_{excl}, bl_{bl} {
            attach(bl_);
        }
        ~Receiver() override {
            detach();
        }

        void onSet(bool set) override {
            std::lock_guard scopeLock{excl_.mLock};
            assert(set);

            size selIdx{};
            for (size idx{0}; idx < excl_.mData.size(); ++idx) {
                auto& testBl{*excl_.mData[idx]};

                if (&testBl == &bl_) {
                    selIdx = idx;
                    continue;
                }

                Bool::Context bl{testBl};
                bl.set(false);
            }

            excl_.sendToReceivers(&Exclusive::Receiver::onSelection, selIdx);
        }

        Exclusive& excl_;
        Bool& bl_;
    };

    mData.reserve(num);
    for (size idx{0}; idx < num; ++idx) {
        auto bl{std::make_unique<Bool>(parent)};
        auto rcvr{std::make_unique<Receiver>(*this, *bl)};
        manage(std::move(rcvr));
        mData.push_back(std::move(bl));
    }

    Bool::Context{*mData[0]}.set(true);
}

data::Exclusive::Exclusive(const Exclusive& other, Node *parent) :
    Exclusive(other.mData.size(), parent) {
    
    for (size idx{0}; idx < other.mData.size(); ++idx) {
        if (not Bool::Context{*other.mData[idx]}.val()) continue;

        Bool::Context{*mData[idx]}.set(true);
        break;
    }
}

std::span<const std::unique_ptr<data::Bool>> data::Exclusive::data() const {
    return mData;
}

data::Bool& data::Exclusive::operator[](size idx) const {
    return *data()[idx];
}

std::unique_ptr<data::pModel> data::Exclusive::clone(Node *) const {
    // TODO: There's no sane way to use this.
    assert(0);
    __builtin_unreachable();
}

