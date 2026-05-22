#include "io.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * components/config/priv/io.cpp
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

#include <cstring>
#include <fstream>

#include <wx/translation.h>

#include "config/blades/bladeconfig.hpp"
#include "config/blades/simple.hpp"
#include "config/blades/ws281x.hpp"
#include "config/buttons/button.hpp"
#include "config/misc/injection.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/priv/data.hpp"
#include "config/priv/generate/check/check.hpp"
#include "config/priv/io.hpp"
#include "config/priv/generate/top.hpp"
#include "config/priv/generate/prop.hpp"
#include "config/priv/generate/presets.hpp"
#include "config/priv/generate/buttons.hpp"
#include "config/priv/generate/styles.hpp"
#include "config/priv/parse/top.hpp"
#include "config/priv/parse/prop.hpp"
#include "config/priv/parse/presets.hpp"
#include "config/priv/parse/buttons.hpp"
#include "config/priv/parse/styles.hpp"
#include "config/priv/parse/utils.hpp"
#include "config/settings/define.hpp"
#include "config/settings/settings.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "utils/files.hpp"
#include "utils/string.hpp"

using namespace config;
using namespace config::priv;

namespace {

std::string extractSection(std::istream&);

} // namespace

std::optional<std::string> io::parse(
    const fs::path& path, Config& config, logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("config::parse()", lBranch)};

    auto file{files::openInput(path)};
    if (not file.is_open()) {
        return errorMessage(logger, wxTRANSLATE("Failed to open config from %s"), path.string());
    }

    while (file.good()) {
        utils::CommentData commentData{.stream_=file};
        (void)utils::extractComments(commentData);

        const auto chr{file.get()};
        if (not file.good()) break;

        if (chr != '#')
            continue;

        auto directive{parse::cppDirective(
            file, logger.bverbose("Parsing directive...")
        )};

        if (directive.type_ == parse::CPPDirective::Type::Include) {
            parse::tryAddInjection(config, directive.buf1_);
        } else if (directive.type_ == parse::CPPDirective::Type::Ifdef) {
            auto sectStr{extractSection(file)};

            if (directive.buf1_ == "CONFIG_TOP") {
                auto err{parse::top(
                    sectStr,
                    config,
                    *logger.binfo("Parsing top...")
                )};
                if (err) return err;
            } else if (directive.buf1_ == "CONFIG_PROP") {
                auto err{parse::prop(
                    sectStr,
                    config,
                    *logger.binfo("Parsing prop...")
                )};
                if (err) return err;
            } else if (directive.buf1_ == "CONFIG_PRESETS") {
                auto err{parse::presets(
                    sectStr,
                    config,
                    *logger.binfo("Parsing presets...")
                )};
                if (err) return err;
            } else if (directive.buf1_ == "CONFIG_STYLES") {
                auto err{parse::styles(
                    sectStr,
                    config,
                    *logger.binfo("Parsing styles...")
                )};
                if (err) return err;
            } else if (directive.buf1_ == "CONFIG_BUTTONS") {
                auto err{parse::buttons(
                    sectStr,
                    config,
                    *logger.binfo("Parsing buttons...")
                )};
                if (err) return err;
            } else {
                logger.warn("Unknown config ifdef-guarded section: " + directive.buf1_);
            }
        }
    }

    if (file.bad())
        return errorMessage(logger, wxTRANSLATE("Failed reading file"));

    logger.info("Parsing complete, finalizing...");

    config.settings_.processDefines();

    logger.info("Done");

    return std::nullopt;
}

std::optional<std::string> io::generate(
    const fs::path& filePath, const Config& config, logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("generate()", lBranch)};

    std::lock_guard scopeLock{config};

    auto precheckErr{gen::preCheck(config, *logger.binfo("Running prechecks..."))};
    if (precheckErr) return precheckErr;

    auto outFile{files::openOutput(filePath)};
    if (not outFile.is_open()) {
        return errorMessage(logger, wxTRANSLATE("Could not open config file for output."));
    }
    
    outFile << "/*\n";
    outFile << " * This configuration file was generated by ProffieConfig, created by Ryryog25.\n";
    outFile << " * ProffieConfig is an All-In-One utility for managing your Proffieboard.\n";
    outFile << " * https://proffieconfig.kafrenetrading.com/\n";
    outFile << " *\n";
    outFile << " * Version: " << executableVersion << ", Generator Version: " wxSTRINGIZE(BIN_VERSION) "\n";
    outFile << " */\n";

    outFile << '\n';
    gen::top(outFile, config);
    outFile << '\n';
    gen::prop(outFile, config);
    outFile << '\n';
    gen::presets(outFile, config);
    outFile << '\n';
    gen::buttons(outFile, config);
    outFile << '\n';
    gen::styles(outFile, config);

    outFile.close();

    logger.info("Done");
    return std::nullopt;
}

namespace {

std::string extractSection(std::istream& file) {
    std::string ret;

    while (file.good()) {
        auto startPos{file.tellg()};

        utils::CommentData commentData{
            .stream_=file,
            .skipNewlines_=false,
            .skipSpaces_=false,
        };
        if (utils::extractComments(commentData)) {
            auto endPos{file.tellg()};
            file.seekg(startPos);

            while (file.tellg() != endPos)
                ret += static_cast<char>(file.get());
        }

        const auto chr{file.get()};
        if (not file.good()) break;

        if (chr != '#') {
            ret += static_cast<char>(chr);
            continue;
        }

        startPos = file.tellg();
        auto directive{parse::cppDirective(file, nullptr)};
        if (directive.type_ != parse::CPPDirective::Type::Endif) {
            auto endPos{file.tellg()};

            file.seekg(startPos);

            ret += '#';
            while (file.tellg() != endPos)
                ret += static_cast<char>(file.get());

            continue;
        }

        break;
    }

    return ret;
}

} // namespace

