#include "presets.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/presets.cpp
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
#include "config/priv/io.hpp"
#include "config/priv/parse/utils.hpp"
#include "config/priv/parse/preset/array.hpp"
#include "config/priv/parse/preset/blades.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

std::optional<std::string> config::priv::parse::presets(
    const std::string& buf, Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parsePresets()")};
    std::istringstream stream(buf);

    enum Read {
        None,
        Preset_Array_Name,
        Preset_Array,
        Blade_Arrays,
    } read{Read::None};
    uint32 depth{};
    uint32 start{};

    std::string tmp;
    std::string name;
    while (stream.good()) {
        utils::CommentData commentData{
            .stream_=stream,
            .skipSpaces_=false,
        };
        (void)utils::extractComments(commentData);

        auto chr{stream.get()};
        if (not stream.good())
            return {};

        if (read == Read::None) {
            if (chr == '#') {
                auto directive{cppDirective(stream, nullptr)};

                if (directive.type_ == CPPDirective::Type::Include)
                    tryAddInjection(config, directive.buf1_);

                continue;
            }

            if (chr == ';') {
                // At the end of Presets array, there's a semicolon *somewhere*
                // if the config is properly formatted. Since this could be all
                // that separates the end of the array from "BladeConfig", it
                // should be a special clear case.
                tmp.clear();
                continue;
            }

            if (std::isspace(chr)) {
                if (tmp == "Preset") {
                    read = Read::Preset_Array_Name;
                } else if (tmp == "BladeConfig") {
                    read = Read::Blade_Arrays;
                    depth = 0;
                }

                tmp.clear();
                continue;
            }

            tmp += static_cast<char>(chr);
        } else if (read == Read::Preset_Array_Name) {
            if (chr == '=' or std::isspace(chr)) {
                // This'll take care of the [] if it's there.
                utils::trimCppName(name);

                read = Read::Preset_Array;
                depth = 0;
                continue;
            }

            name += static_cast<char>(chr);
        } else if (read == Read::Preset_Array or read == Read::Blade_Arrays) {
            if (chr == '{') {
                ++depth;

                if (depth == 1)
                    start = stream.tellg();
            } else if (chr == '}') {
                if (depth == 0) {
                    if (read == Read::Preset_Array)
                        return errorMessage(logger, wxTRANSLATE("Preset array is missing start { before ending };"));

                    return errorMessage(logger, wxTRANSLATE("BladeConfig array is missing start { before ending };"));
                }

                --depth;

                if (depth == 0) {
                    const uint32 dataEndPos = stream.tellg();

                    // Include the outer brace set to provide an extra buffer
                    // for parse::preset::array(). On the off-chance the config
                    // is malformed with a missing last preset }, that may
                    // catch it. Every little bit helps the whacky configs
                    // people manage to make/find.
                    const auto data{buf.substr(start, dataEndPos - start)};
                    std::optional<std::string> ret;
                    if (read == Read::Preset_Array) {
                        auto presetArrays{data::context(config.presetArrays_)};

                        auto& array{presetArrays.append<presets::Array>(config)};
                        auto nameCtxt{data::context(array.name_)};
                        nameCtxt.change(std::move(name));

                        ret = parse::preset::array(
                            data,
                            array,
                            *logger.binfo("Parsing preset array \"" + nameCtxt.val() + "\"...")
                        );
                    } else /* read == Read::Blade_Arrays */ {
                        ret = parse::preset::blades(
                            data,
                            config,
                            *logger.binfo("Parsing blade arrays...")
                        );
                    }

                    if (ret) return ret;

                    read = Read::None;
                }
            }
        }
    }

    if (read != Read::None)
        logger.warn("Searching (" + std::to_string(read) + ") not complete before end.");

    return std::nullopt;
}

