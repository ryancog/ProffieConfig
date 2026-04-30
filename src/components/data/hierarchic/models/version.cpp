#include "version.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/version.cpp
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

using namespace data::hier;

Version::Version(Root& root) : Model(root) {}

Version::Version(const Version& other, Root& root) :
    base::Version(other), Model(other, root) {}

bool Version::set(utils::Version&& ver) {
    return processAction(std::make_unique<SetAction>(std::move(ver)));
}

Version::SetAction::SetAction(utils::Version ver) : mVer{std::move(ver)} {}

bool Version::SetAction::setup() {
    return source<Version>().setupSet(mVer);
}

void Version::SetAction::perform() {
    mVer = source<Version>().doSet(std::move(mVer));
}

void Version::SetAction::retract() {
    mVer = source<Version>().doSet(std::move(mVer));
}

