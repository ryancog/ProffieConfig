#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/utils/style.hpp
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

#include "data/base/models/string.hpp"

namespace config::priv::style {

/**
 * Extract style comments from content.
 *
 * @return if extracted any.
 */
bool extractComments(
    std::string& content,
    size& pos,
    const data::base::String::Context& comments
);

void commentFilter(
    const data::base::String::ROContext&, std::string& str, size& pos
);

/**
 * Format the style content.
 *
 * @param ignoreLength Ignore the column limit when calculating whether to
 *                     explode or not.
 */
std::string format(const std::string&, bool ignoreLength);

} // namespace config::priv::style

