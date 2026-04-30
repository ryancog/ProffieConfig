#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/primitive/models/selection.hpp
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

#include "data/base/models/selection.hpp"
#include "data/primitive/model.hpp"

#include "data_export.h"

namespace data::prim {

struct DATA_EXPORT Selection : base::Selection, Model {
    bool select(uint32 idx, bool select = true) override;
    bool select(std::string&&) override;
    bool setItems(std::vector<std::string>&&) override;
    bool add(std::string&&) override;
    bool remove(uint32) override;
};

} // namespace data::prim

