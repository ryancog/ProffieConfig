#include "prop.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/prop.cpp
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

#include "config/priv/parse/utils.hpp"
#include "utils/string.hpp"

std::optional<std::string> config::priv::parse::prop(
    const std::string& buf, Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parse::prop()")};
    std::istringstream stream(buf);

    auto propVec{config.propVec()};
    if (not propVec) return {};

    while (stream.good()) {
        utils::CommentData commentData{.stream_=stream};
        (void)utils::extractComments(commentData);

        auto chr{stream.get()};
        if (not stream.good())
            break;

        if (chr != '#')
            continue;

        auto directive{cppDirective(stream, logger.bverbose("Directive..."))};
        if (directive.type_ != CPPDirective::Type::Include)
            continue;

        constexpr std::string_view PROP_DIR_STR{"../props/"};
        if (not directive.buf1_.starts_with(PROP_DIR_STR))
            continue;

        std::string_view propFile{directive.buf1_};
        propFile.remove_prefix(PROP_DIR_STR.length());

        for (auto idx{0}; idx < propVec->size(); ++idx) {
            auto& prop{static_cast<versions::props::Prop&>(*(*propVec)[idx])};

            if (prop.filename_ == propFile) {
                config.propChoice().choose(idx);
                return {};
            }
        }
    }

    return {};
}

