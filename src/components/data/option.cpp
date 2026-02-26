#include "option.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/option.cpp
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

data::Option::Option(uint32 num, Node *parent) :
    Model(parent) {
    assert(num > 0);

    mSelected = 0;
    mEnabled.resize(num, true);
    mShown.resize(num, true);
}

data::Option::Option(const Option& other, Node *parent) :
    Model(other, parent) {

    mSelected = other.mSelected;
    mEnabled = other.mEnabled;
    mShown = other.mShown;
}

auto data::Option::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Option>(*this, parent);
}

data::Option::Context::Context(Option& opt) :
    Model::Context{opt} {}

data::Option::Context::~Context() = default;

void data::Option::Context::select(uint32 idx) {
    pModel.processAction(std::make_unique<SelectAction>(
        idx
    ));
}

void data::Option::Context::showOpt(uint32 idx, bool show) {
    pModel.processAction(std::make_unique<ShowAction>(
        idx, show
    ));
}

void data::Option::Context::enableOpt(uint32 idx, bool enable) {
    pModel.processAction(std::make_unique<EnableAction>(
        idx, enable
    ));
}

uint32 data::Option::Context::num() const {
    auto& opt{static_cast<Option&>(pModel)};
    return opt.mEnabled.size();
}

uint32 data::Option::Context::selected() const {
    auto& opt{static_cast<Option&>(pModel)};
    return opt.mSelected;
}

const vector<bool>& data::Option::Context::optEnabled() const {
    auto& opt{static_cast<Option&>(pModel)};
    return opt.mEnabled;
}

const vector<bool>& data::Option::Context::optShown() const {
    auto& opt{static_cast<Option&>(pModel)};
    return opt.mShown;
}

data::Option::ShowAction::ShowAction(uint32 opt, bool shown) :
    mOpt{opt}, mShown{shown} {}

bool data::Option::ShowAction::shouldPerform(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    assert(mOpt < opt.mEnabled.size());
    return opt.mShown[mOpt] != mShown;
}

void data::Option::ShowAction::perform(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    opt.mShown[mOpt] = mShown;

    if (not opt.mEnabled[mOpt]) {
        opt.sendToReceivers(&Receiver::onOptShown, mOpt, opt.mShown[mOpt]);
    }
}

void data::Option::ShowAction::retract(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    opt.mShown[mOpt] = not mShown;

    if (not opt.mEnabled[mOpt]) {
        opt.sendToReceivers(&Receiver::onOptShown, mOpt, opt.mShown[mOpt]);
    }
}

data::Option::EnableAction::EnableAction(uint32 opt, bool enabled) : 
    mOpt{opt}, mEnabled{enabled} {}

bool data::Option::EnableAction::shouldPerform(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    assert(mOpt < opt.mEnabled.size());
    return opt.mEnabled[mOpt] != mEnabled;
}

void data::Option::EnableAction::perform(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    opt.mEnabled[mOpt] = mEnabled;

    opt.sendToReceivers(&Receiver::onOptEnabled, mOpt, opt.mEnabled[mOpt]);
    if (not opt.mShown[mOpt]) {
        // If not shown, then showing depends on the enable state.
        opt.sendToReceivers(&Receiver::onOptShown, mOpt, opt.mEnabled[mOpt]);
    }
}

void data::Option::EnableAction::retract(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    opt.mEnabled[mOpt] = not mEnabled;

    opt.sendToReceivers(&Receiver::onOptEnabled, mOpt, opt.mEnabled[mOpt]);
    if (not opt.mShown[mOpt]) {
        opt.sendToReceivers(&Receiver::onOptShown, mOpt, opt.mEnabled[mOpt]);
    }
}

data::Option::SelectAction::SelectAction(uint32 opt) : mOpt{opt} {}

bool data::Option::SelectAction::shouldPerform(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    assert(mOpt < opt.mEnabled.size());
    return mOpt != opt.mSelected;
}

void data::Option::SelectAction::perform(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    mLast = opt.mSelected;
    opt.mSelected = mOpt;

    opt.sendToReceivers(&Receiver::onSelection, opt.mSelected);
}

void data::Option::SelectAction::retract(Model& model) {
    auto& opt{static_cast<Option&>(model)};

    opt.mSelected = mLast;

    opt.sendToReceivers(&Receiver::onSelection, opt.mSelected);
}

