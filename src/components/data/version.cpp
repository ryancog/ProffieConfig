#include "version.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/version.cpp
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

data::Version::Version(Node *parent) : Model(parent) {
    mRsp = std::make_unique<Responder>();
    mRsp->attach(*this);
}

data::Version::Version(const Version& other, Node *parent) :
    Model(other, parent) {
    mValue = other.mValue;
    mRsp = std::make_unique<Responder>(*other.mRsp);
    mRsp->attach(*this);
}

data::Version::~Version() {
    mRsp->detach();
}

auto data::Version::clone(Node *parent) const -> std::unique_ptr<Model> {
    return std::make_unique<Version>(*this, parent);
}

auto data::Version::responder() const -> Responder& { return *mRsp; }

data::Version::ROContext::ROContext(const Version& ver) :
    Model::ROContext(ver) {}

data::Version::ROContext::~ROContext() = default;

const utils::Version& data::Version::ROContext::val() const {
    return model<Version>().mValue;
}

data::Version::Context::Context(Version& ver) :
    Model::Context(ver), ROContext(ver), Model::ROContext(ver) {}

data::Version::Context::~Context() = default;

void data::Version::Context::set(utils::Version val) const {
    model().processAction(std::make_unique<SetAction>(
        std::move(val)
    ));
}

data::Version::SetAction::SetAction(utils::Version val) :
    mValue{std::move(val)} {}

bool data::Version::SetAction::shouldPerform(Model& model) {
    auto& ver{static_cast<Version&>(model)};
    return utils::Version::RawComparator{}(ver.mValue, mValue) != 0;
}

void data::Version::SetAction::perform(Model& model) {
    auto& ver{static_cast<Version&>(model)};

    auto tmp{std::move(ver.mValue)};
    ver.mValue = std::move(mValue);
    mValue = std::move(tmp);

    ver.sendToReceivers(&Receiver::onSet);
}

void data::Version::SetAction::retract(Model& model) {
    perform(model);
}

