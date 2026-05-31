#include "style.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/styles/style.cpp
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

#include "config/config.hpp"
#include "config/priv/utils/style.hpp"
#include "data/context.hpp"
#include "utils/parent.hpp"
#include "utils/string.hpp"

using namespace config::styles;

Style::Style(Config& config) :
    Model(config), name_(config), comments_(config), content_(config) {
    CreationScope createScope(this);

    const auto nameFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;
    }};
    name_.setFilter(nameFilter);

    comments_.setFilter(priv::style::commentFilter);

    const auto contentFilter{[] (
        const data::base::String::ROContext& ctxt, std::string& str, size& pos
    ) {
        auto& style{utils::parent<&Style::content_>(
            ctxt.model<data::hier::String>()
        )};

        auto commentMove{priv::style::extractComments(
            str, pos, style.comments_
        )};

        uint32 numTrimmed{};
        /*
         * Same as presets::Style except:
         * - No &
         * - No " or parents (no args)
         */
        utils::trim(
            str,
            {
                .allowAlpha=true,
                .allowNum=true,
                .safeList="+-/*<>,_:~\n\t "
            },
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;

        if (commentMove) 
            style.comments_.focus();
    }};
    content_.setFilter(contentFilter);

    content_.change("Layers<Black>");

    name_.change("StyleAlias");
}

Style::Style(const Style& other, Config& config) :
    Model(config),
    name_(other.name_, config),
    comments_(other.comments_, config),
    content_(other.content_, config) {}

auto Style::children() const -> std::vector<const Model *> {
    return {
        &name_,
        &comments_,
        &content_
    };
}

std::string Style::format(bool ignoreLength) {
    auto ctxt{data::context(content_)};
    return priv::style::format(ctxt.val(), ignoreLength);
}

