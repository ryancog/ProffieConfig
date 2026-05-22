#include "top.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/top.cpp
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

#include "config/priv/io.hpp"
#include "config/priv/parse/utils.hpp"
#include "config/settings/define.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

using namespace config;
using namespace config::priv;

namespace {

/**
 * @return if should re-loop
 */
bool parseCommentOrPCOpt(std::istream&, Config&, logging::Logger&);

} // namespace

std::optional<std::string> parse::top(
    const std::string& buf, Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parse::top()")};
    std::istringstream stream(buf);

    while (stream.good()) {
        if (parseCommentOrPCOpt(stream, config, logger))
            continue;

        // At least what's next isn't a comment.
        auto chr{stream.get()};
        if (not stream.good())
            return {};

        if (chr != '#')
            continue;

        auto directive{cppDirective(
            stream, logger.binfo("Parsing directive...")
        )};
        switch (directive.type_) {
            using enum CPPDirective::Type;
            case Define:
            {
                auto ctxt{data::context(config.settings_.defines_)};
                ctxt.append<settings::Define>(
                    config,
                    std::move(directive.buf1_),
                    std::move(directive.buf2_)
                );
                break;
            }
            case Include:
            {
                if (not config.os()) {
                    // If an OS hasn't been chosen yet, choose the first
                    // available.
                    auto ctxt{data::context(config.osChoice())};

                    // None are available, so this must be skipped.
                    if (ctxt.num() == 0)
                        break;

                    ctxt.choose(0);
                }

                auto& boards{config.os()->boards_};
                for (auto& [id, board] : boards) {
                    if (board.include_ != directive.buf1_) continue;

                    config.boardChoice().choose(static_cast<int32>(id));
                    break;
                }

                break;
            }
            case Unknown:
            case Ifdef:
            case Endif:
                logger.warn("Ignoring irrelevant directive: " + std::to_string(static_cast<size>(directive.type_)));
                break;
        }
    }

    return {};
}

namespace {

bool parseCommentOrPCOpt(
    std::istream& stream,
    Config& config,
    logging::Logger& /*logger*/
) {
    utils::CommentData commentData{
        .stream_=stream,
        .single_=true,
    };
    if (not utils::extractComments(commentData))
        return false;

    if (commentData.type_ != utils::CommentData::eType_Line)
        return true;

    if (not commentData.out_.starts_with(PC_OPT_NOCOMMENT_STR))
        return true;

    commentData.out_.erase(0, PC_OPT_NOCOMMENT_STR.length());

    auto splitPos{commentData.out_.find_first_of(" \t")};
    auto key{commentData.out_.substr(0, splitPos)};

    std::string value;
    if (splitPos != std::string::npos) {
        auto splitEndPos{commentData.out_.find_first_not_of(
            " \t", splitPos
        )};
        value = commentData.out_.substr(splitEndPos);
    }

    if (key == ENABLE_MASS_STORAGE_STR) {
        config.settings_.massStorage_.set(true);
    } else if (key == ENABLE_WEBUSB_STR) {
        config.settings_.webUsb_.set(true);
    } else if (key == OS_VERSION_STR) {
        utils::Version version(value);
        if (not version)
            return true;

        for (size idx{0}; idx < config.osVec().size(); ++idx) {
            auto& osVer{*config.osVec()[idx]};

            if (osVer.version_.compare(version) != 0) continue;

            auto osChoice{data::context(config.osChoice())};
            osChoice.choose(static_cast<int32>(idx));

            break;
        }
    }

    return true;
}

} // namespace

