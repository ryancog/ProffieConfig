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

#include "data/hierarchic/models/bool.hpp"

using namespace data::base;

// Another case where copy ctor can't really know what's actually going on, so
// parent will need to do the work.

void Exclusive::init(size num) {
    assert(num > 1);

    for (size idx{0}; idx < num; ++idx) {
        auto obj{create(idx)};

        static const auto table{[] {
            Bool::RecvTable table;
            table.onSet_ = map<&Exclusive::onSet>();
            return table;
        }()};
        if (auto *ptr{dynamic_cast<data::hier::Bool *>(obj.get())})
            respondWith(*ptr, table);
        else
            observeWith(*obj, table);

        mData.push_back(std::move(obj));
    }

    mData[0]->set(true);
}

std::span<const std::unique_ptr<Bool>> Exclusive::data() const {
    return mData;
}

bool Exclusive::setupSelect(size& idx) {
    assert(idx < mData.size());
    return mSelected != idx;
}

size Exclusive::doSelect(bool undo, size sel) {
    if (undo)
        responderHook<&RecvTable::onSelection_>();

    for (size idx{0}; idx < mData.size(); ++idx)
        mData[idx]->set(idx == sel);

    auto ret{mSelected};
    mSelected = sel;
    sendToObservers<&RecvTable::onSelection_>();

    if (not undo)
        responderHook<&RecvTable::onSelection_>();

    return ret;
}

void Exclusive::onSet(const Model& model) {
    for (size idx{0}; idx < mData.size(); ++idx) {
        if (mData[idx].get() == &model) {
            select(idx);
            break;
        }
    }
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

