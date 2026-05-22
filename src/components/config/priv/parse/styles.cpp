#include "styles.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/styles.cpp
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

#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/styles/style.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

std::optional<std::string> config::priv::parse::styles(
    const std::string& buf, Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parse::styles()")};
    std::istringstream stream(buf);

    enum {
        eNone,
        eStyle,
        eStyle_Name,
    } reading{eNone};

    std::string readHistory;

    std::string comments;
    std::string bladestyle;
    std::string name;

    while (stream.good()) {
        utils::CommentData commentData{.stream_=stream};
        if (utils::extractComments(commentData)) {
            if (reading == eStyle) {
                if (commentData.type_ == utils::CommentData::eType_Block) {
                    if (not comments.empty())
                        comments += '\n';

                    comments += commentData.out_;
                } else /* Line */ {
                    if (not bladestyle.empty())
                        bladestyle += '\n';

                    bladestyle += "// ";
                    bladestyle += commentData.out_;
                    bladestyle += '\n';
                }
            }

            if (not comments.empty()) comments += '\n';
            comments += commentData.out_;
        }

        const auto chr{stream.get()};
        if (not stream.good())
            continue;

        if (reading == eNone) {
            if (name.empty()) {
                readHistory += static_cast<char>(chr);
                if (readHistory.rfind("using") != std::string::npos) {
                    reading = eStyle_Name;
                    readHistory.clear();
                }
            } else if (chr == '=') {
                reading = eStyle;
            }
        } else if (reading == eStyle_Name) {
            if (std::isspace(chr)) {
                if (not name.empty()) {
                    reading = eNone;
                }
                continue;
            }

            name += static_cast<char>(chr);
        } else if (reading == eStyle) {
            if (chr == ';') {
                utils::trimWhitespaceOutsideString(bladestyle);

                auto styles{data::context(config.styles_)};

                // RIP quadruple nested for loops.

                auto& style{styles.append<styles::Style>(config)};
                style.name_.change(std::move(name));
                style.style_.comment_.change(std::move(comments));
                style.style_.content_.change(std::move(bladestyle));
                style.style_.content_.change(style.style_.format());

                name.clear();
                bladestyle.clear();
                comments.clear();

                reading = eNone;
                continue;
            }

            bladestyle += static_cast<char>(chr);
        }
    }

    return std::nullopt;
}

