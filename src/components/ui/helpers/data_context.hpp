#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/data_context.hpp
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

#include "data/base/model.hpp"
#include "data/context.hpp"
#include "utils/defer.hpp"

#include "ui_export.h"

namespace pcui {

namespace priv {

UI_EXPORT void doGuiDataLock(const data::base::Model&);

} // namespace priv

/**
 *
 */
template <typename T>
auto guiDataContext(T& t) {
    priv::doGuiDataLock(t);

    // Release to balance above lock
    defer { t.unlock(); };

    return data::context(t);
}

} // namespace pcui

