#include "bool.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/bool.cpp
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

#include "utils/hash.hpp"

using namespace data::hier;

Bool::Bool(Root& root) : Model(root) {}

Bool::Bool(const Bool& other, Root& root) :
    base::Bool(other), Model(other, root) {}

bool Bool::set(bool val) {
    return processAction(std::make_unique<SetAction>(val));
}

uint64 Bool::hashThis() const {
    return utils::hash::single(ROContext(*this).val());
}

Bool::SetAction::SetAction(bool val) : mValue{val} {}

bool Bool::SetAction::setup() {
    return source<Bool>().setupSet(mValue);
}

void Bool::SetAction::perform() {
    source<Bool>().doSet(mValue);
}

void Bool::SetAction::retract() {
    source<Bool>().doSet(not mValue);
}

