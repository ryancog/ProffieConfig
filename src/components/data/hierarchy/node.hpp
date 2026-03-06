#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchy/node.hpp
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

#include <functional>
#include <string>

#include "data/hierarchy/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

struct Root;
struct Action;

enum class ActionMode {
    Perform,
    Rewind,
};

struct DATA_EXPORT Node : Model {
    /**
     * Called for each child model of a node.
     *
     * If the model has a representative string the id is derived from, it may
     * be passed to be used as the preferred method for identification w.r.t.
     * serialization. If not, it may be left empty.
     *
     * @return if enumeration should terminate
     */
    using EnumFunc = std::function<bool(Model&, uint64, const std::string&)>;

    Node(Node *, Root * = nullptr);
    Node(const Node&, Node *, Root * = nullptr);
    ~Node() override;

    Node(const Node&) = delete;

    /**
     * Send an action down to be processed, and sent back up/recorded after
     * completion. If the model could not be found, nothing happens.
     *
     * @return if the action found a model
     */
    bool forwardAction(std::unique_ptr<Action>&& action);

    /**
     * Called with a function which should be called once for every child of
     * the node.
     *
     * @return if function terminated enumeration early
     */
    virtual bool enumerate(const EnumFunc&) = 0;

protected:
    /**
     * Find the model with specified id
     */
    [[nodiscard]] virtual Model *find(uint64) [[clang::lifetimebound]] = 0;

    /**
     * Get ID For Model
     */
    uint64 idFor(Model&);

private:
    friend Root;
    friend Model;

    /**
     * Send an action to a child, going through the hierarchy as needed.
     */
    void sendDownAction(Action&, ActionMode);

    /**
     * Send action to root, building trace.
     */
    void sendUpAction(Model& from, std::unique_ptr<Action>&&);
};

} // namespace data

