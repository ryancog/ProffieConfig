#include "version.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/version.cpp
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

using namespace data::base;

Version::Version(const Version& other) = default;

bool Version::setupSet(utils::Version& ver) {
    return utils::Version::RawComparator{}(ver, mVer) != 0;
}

utils::Version Version::doSet(bool undo, utils::Version&& ver) {
    if (undo)
        responderHook(&RecvTable::onSet_);

    auto ret{std::move(mVer)};
    mVer = std::move(ver);

    sendToObservers(&RecvTable::onSet_);

    if (not undo)
        responderHook(&RecvTable::onSet_);

    return ret;
}

Version::ROContext::ROContext(const Version& ver) : Model::ROContext(ver) {}

const utils::Version& Version::ROContext::val() const {
    return model().mVer;
}

Version::Context::Context(Version& ver) :
    Model::Context(ver), ROContext(ver), Model::ROContext(ver) {}

void Version::Context::set(utils::Version&& val) const {
    model().set(std::move(val));
}

