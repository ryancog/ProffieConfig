#include "node.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchy/node.cpp
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

#include "data/hierarchy/action.hpp"
#include "data/hierarchy/model.hpp"
#include "data/hierarchy/root.hpp"

data::Node::Node(Node *parent, Root *root) :
    Model(parent, root) {}

data::Node::Node(const Node& other, Node *parent, Root *root) :
    Model(other, parent, root) {}

data::Node::~Node() = default;

uint64 data::Node::idFor(Model& search) {
    uint64 ret{};

    const auto onEnum{[&search, &ret](
        Model& model, uint64 id, const std::string&
    ) -> bool {
        if (&model != &search) return false;

        ret = id;
        return true;
    }};

    if (not enumerate(onEnum)) {
        assert(0);
        __builtin_unreachable();
    }

    return ret;
}

void data::Node::sendDownAction(Action& action, ActionMode mode) {
    std::scoped_lock scopeLock{pLock};

    assert(not action.mTrace.empty());

    size idx{action.mTrace.size() - 1};

    Node *node{this};
    Model *child{};
    for (
            auto iter{action.mTrace.rbegin()};
            iter != action.mTrace.rend();
            ++iter
        ) {
        assert(node);

        child = node->find(*iter);
        assert(child);

        // Using a dynamic cast here so that the above assert is a valid debug
        // sanity check.
        node = dynamic_cast<Node *>(child);
    }

    if (mode == ActionMode::Perform) {
        action.perform(*child);
    } else if (mode == ActionMode::Rewind) {
        action.retract(*child);
    } else assert(0);
}

void data::Node::sendUpAction(Model& from, std::unique_ptr<Action>&& action) {
    std::scoped_lock scopeLock{pLock};

    action->mTrace.push_back(idFor(from));

    if (this == pRoot) {
        pRoot->recordAction(std::move(action));
        return;
    }

    pParent->sendUpAction(*this, std::move(action));
}

