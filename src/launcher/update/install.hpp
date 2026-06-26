#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * launcher/update/install.hpp
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

#include "ui/dialogs/progress.hpp"
#include "log/branch.hpp"

#include "update.hpp"
#include "changelog.hpp"

namespace Update {

[[nodiscard]] bool pullNewFiles(
    const Changelog& changelog,
    const Data& data,
    pcui::ProgressDialog *,
    logging::Branch&
);

[[nodiscard]] bool installFiles(
    const Changelog& changelog,
    const Data& data,
    pcui::ProgressDialog *,
    logging::Branch&
);

} // namespace Update

