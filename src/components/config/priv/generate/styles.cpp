#include "styles.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/generate/styles.cpp
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

#include <sstream>

#include "config/styles/style.hpp"
#include "data/context.hpp"

using namespace config;
using namespace config::priv;

void gen::styles(std::ostream& out, const Config& config) {
    out << "#ifdef CONFIG_STYLES\n";

    auto ctxt{data::context(config.styles_)};
    for (auto& model : ctxt.children()) {
        auto& style{dynamic_cast<styles::Style&>(*model)};

        auto name{data::context(style.name_)};
        auto comment{data::context(style.comments_)};
        auto formatted{style.format(true)};

        if (not comment.val().empty()) {
            out << "/*\n";

            std::istringstream stream(comment.val());
            std::string line;
            while (std::getline(stream, line)) {
                out << " * " << line << '\n';;
            }

            out << " */\n";
        }

        out << "using " << name.val() << " = " << formatted;
        out << ";\n\n";
    }

    out << "#endif\n";
}

