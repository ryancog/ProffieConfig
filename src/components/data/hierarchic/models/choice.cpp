#include "choice.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/choice.cpp
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

Choice::Choice(Root& root) : Model(root) {}

Choice::Choice(const Choice& other, Root& root) :
    base::Choice(other), Model(other, root) {}

bool Choice::choose(int32 idx) {
    return processAction(std::make_unique<ChoiceAction>(idx));
}

bool Choice::update(uint32 num, int32 idx) {
    return processAction(std::make_unique<UpdateAction>(num, idx));
}

uint64 Choice::hashThis() const {
    ROContext ctxt(*this);
    return utils::hash::combine(
        utils::hash::single(ctxt.idx()),
        utils::hash::single(ctxt.num())
    );
}

Choice::ChoiceAction::ChoiceAction(int32 choice) : mIdx{choice} {}

bool Choice::ChoiceAction::setup() {
    return source<Choice>().setupChoose(mIdx);
}

void Choice::ChoiceAction::perform() {
    mIdx = source<Choice>().doChoose(false, mIdx);
}

void Choice::ChoiceAction::retract() {
    mIdx = source<Choice>().doChoose(true, mIdx);
}

Choice::UpdateAction::UpdateAction(uint32 num, int32 idx) :
    mNum{num}, mIdx{idx} {}

bool Choice::UpdateAction::setup() {
    return source<Choice>().setupUpdate(mNum, mIdx);
}

void Choice::UpdateAction::perform() {
    auto last{source<Choice>().doUpdate(false, mNum, mIdx)};
    mNum = last.first;
    mIdx = last.second;
}

void Choice::UpdateAction::retract() {
    auto orig{source<Choice>().doUpdate(true, mNum, mIdx)};
    mNum = orig.first;
    mIdx = orig.second;
}

