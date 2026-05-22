#include "array.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/preset/array.cpp
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

#include <wx/translation.h>

#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "config/priv/io.hpp"
#include "data/context.hpp"
#include "log/branch.hpp"
#include "utils/string.hpp"

using namespace config;
using namespace config::priv;

std::optional<std::string> parse::preset::array(
    const std::string& buf, presets::Array& array, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parse::preset::array()")};
    std::istringstream stream(buf);

    enum {
        eNone,

        ePost_Brace,
        eDir,
        ePost_Dir,
        eTrack,
        ePost_Track,

        eStyle,

        eName,
    } reading{eNone};

    std::vector<char> depth;
    std::string tmp;
    std::string comments;

    const auto finishStyleReading{[&array, &tmp, &comments]() {
        utils::trimSurroundingWhitespace(tmp);

        auto presets{data::context(array.presets_)};
        auto& preset{dynamic_cast<presets::Preset&>(
            *presets.children().back()
        )};
        auto styles{data::context(preset.styles_)};
        auto& style{styles.append<presets::Style>(array.root<Config>())};

        style.content_.change(std::move(tmp));
        tmp.clear();

        style.comment_.change(std::move(comments));
        comments.clear();

        style.content_.change(style.format());
    }};

    auto presets{data::context(array.presets_)};

    while (stream.good()) {
        utils::CommentData commentData{
            .stream_=stream,
            .single_=true,
        };
        if (utils::extractComments(commentData)) {
            if (reading == eStyle) {
                if (commentData.type_ == utils::CommentData::eType_Block) {
                    if (not comments.empty())
                        comments += '\n';

                    comments += commentData.out_;
                } else /* Line */ {
                    if (not tmp.empty())
                        tmp += '\n';

                    tmp += "// ";
                    tmp += commentData.out_;
                    tmp += '\n';
                }
            }

            continue;
        }

        const auto chr{stream.get()};
        if (not stream.good())
            return {};

        if (reading == eNone) {
            if (chr == '{') {
                reading = ePost_Brace;
                presets.append<presets::Preset>(
                    array.root<Config>()
                );
            }
        } else if (reading == ePost_Brace or reading == ePost_Dir) {
            if (chr == '"') {
                if (reading == ePost_Brace) {
                    reading = eDir;
                } else if (reading == ePost_Dir) {
                    reading = eTrack;
                }
            }
        } else if (reading == eDir) {
            if (chr == '"') {
                reading = ePost_Dir;
                continue;
            }

            auto& preset{dynamic_cast<presets::Preset&>(
                *presets.children().back()
            )};

            auto fontDir{data::context(preset.fontDir_)};
            fontDir.append(static_cast<char>(chr));
        } else if (reading == eTrack) {
            if (chr == '"') {
                reading = ePost_Track;
                continue;
            }

            auto& preset{dynamic_cast<presets::Preset&>(
                *presets.children().back()
            )};

            auto track{data::context(preset.track_)};
            track.append(static_cast<char>(chr));
        } else if (reading == ePost_Track) {
            if (chr == ',') {
                reading = eStyle;
            }
        } else if (reading == eStyle) {
            bool inSingleQuotes{not depth.empty() and depth.back() == '\''};
            bool inDoubleQuotes{not depth.empty() and depth.back() == '"'};
            if (std::isspace(chr)) {
                if (chr != ' ' or not (inSingleQuotes or inDoubleQuotes))
                    continue;
            }

            if (depth.empty()) {
                // This loops in eStyle until the quotes are found, which
                // denotes the end of styles and now is at the very last
                // cstring name entry.
                if (chr == '"') {
                    auto& preset{dynamic_cast<presets::Preset&>(
                        *presets.children().back()
                    )};

                    preset.name_.clear();

                    reading = eName;
                    tmp.clear();
                    continue;
                }

                // Stays in eStyle, see above.
                if (chr == ',') {
                    finishStyleReading();
                    continue;
                }
            }

            if (chr == '"' and not inSingleQuotes) {
                if (not depth.empty() and depth.back() == '"') depth.pop_back();
                else depth.push_back(static_cast<char>(chr));
            }
            if (chr == '\'' and not inDoubleQuotes) {
                if (not depth.empty() and depth.back() == '\'') depth.pop_back();
                else depth.push_back(static_cast<char>(chr));
            }

            if (chr == '}') {
                finishStyleReading();

                if (not depth.empty())
                    logger.warn("Hit preset end before finishing style. This will mean errors to correct later!");

                depth.clear();

                auto& preset{dynamic_cast<presets::Preset&>(
                    *presets.children().back()
                )};

                preset.name_.change(
                    "preset" + std::to_string(presets.children().size())
                );

                reading = eNone;
                continue;
            }

            if (chr == '<' or chr == '(') {
                depth.push_back(static_cast<char>(chr));
            } else if (chr == '>' or chr == ')') {
                if (depth.empty()) {
                    // TODO: Make this a useful error?
                    return errorMessage(logger, wxTRANSLATE("Found %c before matching open when parsing style"), chr);
                }

                if (
                        (chr == '>' and depth.back() != '<') or
                        (chr == ')' and depth.back() != '(')
                   ) {
                    return errorMessage(logger, wxTRANSLATE("Found %c when expecting match for %c when parsing style"), chr, depth.back());
                }

                depth.pop_back();
            }

            tmp += static_cast<char>(chr);
        } else if (reading == eName) {

            if (chr == '"' or chr == '}') {
                reading = eNone;

                auto& preset{dynamic_cast<presets::Preset&>(
                    *presets.children().back()
                )};

                if (tmp.empty()) {
                    preset.name_.change(
                        "preset" + std::to_string(presets.children().size())
                    );
                } else {
                    preset.name_.change(std::move(tmp));
                }

                continue;
            }
            
            tmp += static_cast<char>(chr);
        }
    }

    return std::nullopt;
}

