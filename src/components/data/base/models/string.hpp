#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/base/models/string.hpp
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

#include <string>

#include "data/base/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data::base {

// TODO: The actions for this would like to be more space efficient, and
//       preserve things like selection?
struct DATA_EXPORT String : virtual Model {
    struct DATA_EXPORT ROContext;
    struct DATA_EXPORT Context;
    struct DATA_EXPORT RecvTable;

    using Filter = void (*)(const ROContext&, std::string&, size&);

    String() = default;
    String(const String&);

    void setFilter(Filter);

    bool clear();

    /**
     * Replace text, moving pos to end.
     */
    bool change(std::string&&);

    /**
     * Replace text with new text and pos.
     */
    virtual bool change(std::string&&, size) = 0;

    /**
     * Move "cursor"
     * Position represents insertion point, or deletion point + 1
     */
    virtual bool move(size) = 0;

protected:
    bool setupChange(std::string&, size&);
    std::pair<std::string, size> doChange(bool undo, std::string&&, size);

    bool setupMove(size);
    size doMove(bool undo, size);

private:
    Filter mFilter{nullptr};
    
    std::string mValue;
    size mPos{0};
};

struct DATA_EXPORT String::ROContext : virtual Model::ROContext {
    ROContext(const String&);

    template <typename M = String>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] const std::string& val() const LIFETIMEBOUND;
    [[nodiscard]] size pos() const;
};

struct DATA_EXPORT String::Context : Model::Context, ROContext {
    Context(String&);

    template <typename M = String>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void change(std::string&&, size) const;
    void change(std::string&&) const;

    void append(char) const;
    void append(std::string_view) const;

    void clear() const;

    void move(size) const;
    void moveStart() const;
    void moveEnd() const;
};

struct DATA_EXPORT String::RecvTable : Model::RecvTable {
    /**
     * Text is changed
     */
    Mapping<> onChange_;

    /**
     * Cursor position is moved.
     */
    Mapping<> onMove_;
};

} // namespace data::base

