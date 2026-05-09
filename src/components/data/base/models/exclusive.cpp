#include "exclusive.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/exclusive.cpp
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

#include "data/context.hpp"

using namespace data::base;

// Another case where copy ctor can't really know what's actually going on, so
// parent will need to do the work.

void Exclusive::init(size num) {
    assert(num > 1);

    for (size idx{0}; idx < num; ++idx) {
        auto obj{create(idx)};

        const auto table{[] {
            Bool::RecvTable table;
            table.onSet_ = map(&Exclusive::onSet);
            return table;
        }()};
        amend(*obj, table);

        mData.push_back(std::move(obj));
    }

    mData[0]->set(true);
}

std::span<const std::unique_ptr<Bool>> Exclusive::data() const {
    return mData;
}

bool Exclusive::setupSelect(size& idx) {
    assert(idx < mData.size());
    return true;
}

size Exclusive::doSelect(size idx) {
    auto ret{mSelected};

    mData[idx]->set(true);
    // mSelected updated by receiver handler.

    return mSelected;
}

void Exclusive::onSet(const Model& model) {
    auto& bl{dynamic_cast<const Bool&>(model)};

    if (not context(bl).val()) return;

    size selIdx{};
    for (size idx{0}; idx < mData.size(); ++idx) {
        auto& testBl{*mData[idx]};

        if (&testBl == &bl) {
            selIdx = idx;
            continue;
        }

        Bool::Context bl{testBl};
        bl.set(false);
    }

    mSelected = selIdx;
    sendToReceivers(&RecvTable::onSelection_);
}

Exclusive::ROContext::ROContext(const Exclusive& excl) :
    Model::ROContext(excl) {}

size Exclusive::ROContext::num() const {
    return model().mData.size();
}

Bool& Exclusive::ROContext::operator[](size idx) const {
    return *model().mData.at(idx);
}

size Exclusive::ROContext::selected() const {
    return model().mSelected;
}

Exclusive::Context::Context(Exclusive& excl) :
    Model::Context(excl), ROContext(excl), Model::ROContext(excl) {}

void Exclusive::Context::select(size idx) const {
    model().select(idx);
}

