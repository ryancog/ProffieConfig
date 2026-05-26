#include "style.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/presets/style.cpp
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

#include <algorithm>
#include <optional>
#include <sstream>
#include <stack>

#include <wx/translation.h>

#include "data/context.hpp"
#include "config/config.hpp"
#include "config/priv/utils/style.hpp"
#include "utils/parent.hpp"
#include "utils/string.hpp"

using namespace config::presets;

Style::Style(Config& config) :
    Model(config),
    comment_(root()),
    content_(root()) {
    CreationScope createScope(this);

    comment_.setFilter(priv::style::commentFilter);

    const auto contentFilter{[] (
        const data::base::String::ROContext& ctxt, std::string& str, size& pos
    ) {
        auto& style{utils::parent<&Style::content_>(
            ctxt.model<data::hier::String>()
        )};

        uint32 numTrimmed{};

        /*
         * - Only allow chars for the start of a block comment. No other need 
         *   for slash
         *
         * - <>(), are self-explanatory
         *
         * - & for global style objects like the charging style.
         *
         * - : for scope resolution operator (there's scoped enums with effects)
         *
         * - "" may be used for the dynamic args defaults.
         *
         * - \n\t and ' ' are self-explanatory.
         *
         * - '-' for negative numbers.
         *
         * - '_' because it's just generally used.
         */
        utils::trim(
            str,
            {
                .allowAlpha=true,
                .allowNum=true,
                .safeList="/*<>(),&_:-\"\n\t "
            },
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;

        auto commentMove{priv::style::extractComments(
            str, pos, style.comment_
        )};

        // Erase everything after last ), e.g. `StylePtr<...>(),` erase the
        // comma or anything else that shouldn't be there 
        auto endPos{str.find(')')};
        if (endPos != std::string::npos) {
            str.erase(endPos + 1);
            pos = std::min<size_t>(pos, endPos + 1);
        }

        if (commentMove) 
            style.comment_.focus();
    }};
    content_.setFilter(contentFilter);

    comment_.change("ProffieConfig Default Blue AudioFlicker");
    content_.change("StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,Blue,300,800>()");
}

Style::Style(const Style& other, Config& config) :
    Model(config),
    comment_(other.comment_, root()),
    content_(other.content_, root()) {}

auto Style::children() const -> std::vector<const Model *> {
    return {
        &comment_,
        &content_,
    };
}

std::string Style::format(bool ignoreLength) {
    auto ctxt{data::context(content_)};
    return priv::style::format(ctxt.val(), ignoreLength);
}

