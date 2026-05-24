#include "model.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/data/hierarchic/model.cpp
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

#include "data/hierarchic/root.hpp"

using namespace data::hier;

Model::Model(Root& root) : mRoot{root} {}

Model::Model(const Model& other, Root& root) :
    base::Model(other), mRoot{root} {}

bool Model::enable(bool en) {
    return processAction(std::make_unique<EnableAction>(en));
}

void Model::lock() const {
    mRoot.mMutex.lock();
}

bool Model::tryLock() const {
    return mRoot.mMutex.try_lock();
}

void Model::unlock() const {
    mRoot.mMutex.unlock();
}

std::vector<Model *> Model::children() {
    auto tmp{const_cast<const Model *>(this)->children()};

    std::vector<Model *> ret;
    ret.reserve(tmp.size());

    for (auto *ptr : tmp)
        ret.push_back(const_cast<Model *>(ptr));

    return ret;
}

std::vector<const Model *> Model::children() const {
    return {};
}

bool Model::processAction(std::unique_ptr<Action>&& action) {
    std::lock_guard scopeLock(*this);

    action->mSource = this;

    if (not action->setup()) return false;

    if (not mRoot.capturePerformance()) return false;

    action->perform();

    if (mRoot.isActuallyCapturing()) {
        mRoot.recordAction(std::move(action));
    }

    return true;
}

Model::EnableAction::EnableAction(bool en) : mEnable{en} {}

bool Model::EnableAction::setup() {
    return source<Model>().setupEnable(mEnable);
}

void Model::EnableAction::perform() {
    source<Model>().doEnable(mEnable);
}

void Model::EnableAction::retract() {
    source<Model>().doEnable(not mEnable);
}

Model::CreationScope::CreationScope(Model *model) : mModel{*model} {
    assert(model);
    mModel.root().suppressActions();
}

Model::CreationScope::~CreationScope() {
    mModel.root().unsuppressActions(false);
}

