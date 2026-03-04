#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/parse.hpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <wx/translation.h>

#include "config/config.hpp"
#include "log/logger.hpp"

namespace config::priv {

/**
 * Parse a config from disk
 *
 * @return Error message on failure. nullopt on success.
 */
std::optional<std::string> parse(
    const fs::path&, Config&, logging::Branch *lBranch = nullptr
);

} // namespace config::priv

