#include "prop.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/generate/prop.cpp
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

using namespace config;
using namespace config::priv;

void gen::prop(std::ostream& out, const Config& config) {
    auto *prop{config.prop()};

    if (prop == nullptr) return;
    if (prop->filename_.empty()) return;

    out << "#ifdef CONFIG_PROP\n";
    out << "#include \"../props/" << prop->filename_ << "\"\n";
    out << "#endif\n";
}

