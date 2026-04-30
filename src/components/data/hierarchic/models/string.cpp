#include "string.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/models/string.cpp
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

String::String(Root& root) : Model(root) {}

String::String(const String& other, Root& root) :
    base::String(other), Model(other, root) {}

bool String::change(std::string&& str, size pos) {
    return processAction(std::make_unique<ChangeAction>(std::move(str), pos));
}

bool String::move(size pos) {
    return processAction(std::make_unique<MoveAction>(pos));
}

String::ChangeAction::ChangeAction(std::string&& str, size pos) :
    mStr{std::move(str)}, mPos{pos} {}

bool String::ChangeAction::setup() {
    return source<String>().setupChange(mStr, mPos);
}

void String::ChangeAction::perform() {
    auto last{source<String>().doChange(std::move(mStr), mPos)};
    mStr = std::move(last.first);
    mPos = last.second;
}

void String::ChangeAction::retract() {
    auto orig{source<String>().doChange(std::move(mStr), mPos)};
    mStr = std::move(orig.first);
    mPos = orig.second;
}

String::MoveAction::MoveAction(size pos) : mPos{pos} {}

bool String::MoveAction::setup() {
    return source<String>().setupMove(mPos);
}

void String::MoveAction::perform() {
    mPos = source<String>().doMove(mPos);
}

void String::MoveAction::retract() {
    mPos = source<String>().doMove(mPos);
}

