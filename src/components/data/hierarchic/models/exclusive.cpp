#include "exclusive.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/heirarchic/models/exclusive.cpp
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

#include "data/hierarchic/models/bool.hpp"
#include "utils/hash.hpp"

using namespace data::hier;

Exclusive::Exclusive(Root& root, size num) :
    Model(root) {
    init(num);
}

Exclusive::Exclusive(Root& root) : Model(root) {}

auto Exclusive::children() const -> std::vector<const Model *> {
    std::vector<const Model *> ret;
    ret.reserve(data().size());

    for (const auto& child : data())
        ret.push_back(dynamic_cast<Model *>(child.get()));

    return ret;
}

bool Exclusive::select(size idx) {
    return processAction(std::make_unique<SelectAction>(idx));
}

std::unique_ptr<data::base::Bool> Exclusive::create(size) {
    return std::make_unique<Bool>(root());
}

uint64 Exclusive::hashThis() const {
    ROContext ctxt(*this);

    return utils::hash::combine(
        utils::hash::single(ctxt.selected()),
        utils::hash::single(ctxt.num())
    );
}

Exclusive::SelectAction::SelectAction(size idx) : mIdx{idx} {}

bool Exclusive::SelectAction::setup() {
    return source<Exclusive>().setupSelect(mIdx);
}

void Exclusive::SelectAction::perform() {
    mIdx = source<Exclusive>().doSelect(mIdx);
}

void Exclusive::SelectAction::retract() {
    mIdx = source<Exclusive>().doSelect(mIdx);
}

