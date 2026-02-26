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

#include <cassert>
#include <memory>

data::Choice::Choice(Node *parent) : Model(parent) {}

data::Choice::Choice(const Choice& other, Node *parent) :
    Model(other, parent) {
    mNumChoices = other.mNumChoices;
    mIdx = other.mIdx;
}

auto data::Choice::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Choice>(*this, parent);
}

void data::Choice::setChoiceValidator(ChoiceValidator validator) {
    mChoiceValidator = std::move(validator);
}

void data::Choice::setUpdateValidator(UpdateValidator validator) {
    mUpdateValidator = std::move(validator);
}

data::Choice::Context::Context(Choice& choice):
    Model::Context{choice} {}

data::Choice::Context::~Context() = default;

void data::Choice::Context::choose(uint32 idx) {
    assert(idx <= std::numeric_limits<int32>::max());
    pModel.processAction(std::make_unique<ChoiceAction>(
        static_cast<int32>(idx)
    ));
}

void data::Choice::Context::unchoose() {
    pModel.processAction(std::make_unique<ChoiceAction>(
        -1
    ));
}

void data::Choice::Context::update(uint32 num) {
    pModel.processAction(std::make_unique<UpdateAction>(
        static_cast<int32>(num)
    ));
}

uint32 data::Choice::Context::numChoices() const {
    auto& choice{static_cast<Choice&>(pModel)};
    return choice.mNumChoices;
}

int32 data::Choice::Context::choice() const {
    auto& choice{static_cast<Choice&>(pModel)};
    return choice.mIdx;
}

data::Choice::Context::operator bool() const {
    auto& choice{static_cast<Choice&>(pModel)};
    return choice.mIdx >= 0;
}

data::Choice::ChoiceAction::ChoiceAction(int32 choice) : mChoice{choice} {}

bool data::Choice::ChoiceAction::shouldPerform(Model& model) {
    auto& choice{static_cast<Choice&>(model)};

    assert(mChoice < choice.mNumChoices);

    if (choice.mIdx == mChoice) return false;
    if (
            choice.mChoiceValidator and
            not choice.mChoiceValidator(mChoice)
       ) return false;

    return true;
}

void data::Choice::ChoiceAction::perform(Model& model) {
    auto& choice{static_cast<Choice&>(model)};

    mLast = choice.mIdx;
    choice.mIdx = static_cast<int32>(mChoice);

    choice.sendToReceivers(&Receiver::onChoice, choice.mIdx);
}

void data::Choice::ChoiceAction::retract(Model& model) {
    auto& choice{static_cast<Choice&>(model)};

    choice.mIdx = mLast;

    choice.sendToReceivers(&Receiver::onChoice, choice.mIdx);
}

data::Choice::UpdateAction::UpdateAction(uint32 num) : mNum{num} {}

bool data::Choice::UpdateAction::shouldPerform(Model& model) {
    auto& choice{static_cast<Choice&>(model)};
    return choice.mNumChoices != mNum;
}

void data::Choice::UpdateAction::perform(Model& model) {
    auto& choice{static_cast<Choice&>(model)};

    mLast = choice.mNumChoices;
    choice.mNumChoices = mNum;

    mLastChoice = choice.mIdx;
    choice.mIdx = -1;

    choice.sendToReceivers(&Receiver::onUpdate, choice.mNumChoices);
}

void data::Choice::UpdateAction::retract(Model& model) {
    auto& choice{static_cast<Choice&>(model)};

    choice.mIdx = mLastChoice;
    choice.mNumChoices = mLast;

    choice.sendToReceivers(&Receiver::onUpdate, choice.mNumChoices);
    choice.sendToReceivers(&Receiver::onChoice, choice.mIdx);
}

