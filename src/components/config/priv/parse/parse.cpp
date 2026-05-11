#include "parse.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * components/config/priv/parse/parse.cpp
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
#include <memory>
#include <sstream>

#include <wx/filedlg.h>
#include <wx/translation.h>

#include "config/blades/bladeconfig.hpp"
#include "config/blades/simple.hpp"
#include "config/blades/ws281x.hpp"
#include "config/buttons/button.hpp"
#include "config/misc/injection.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/priv/io.hpp"
#include "config/strings.hpp"
#include "config/settings/define.hpp"
#include "config/settings/settings.hpp"
#include "data/context.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "ui/dialogs/message.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/string.hpp"
#include "utils/version.hpp"
#include "versions/prop.hpp"
#include "versions/os.hpp"

namespace {

std::string extractSection(std::istream&);

void parseTop(std::istream&, config::Config&);
void parseProp(std::istream&, config::Config&);

std::optional<std::string> parsePresets(
    std::istream&, config::Config&, logging::Branch&
);

std::optional<std::string> parsePresetArray(
    const std::string& data, config::presets::Array&, logging::Branch&
);

std::optional<std::string> parseBladeArrays(
    const std::string& data, config::Config&, logging::Branch&
);

std::optional<std::string> parseBlade(
    std::string data,
    config::blades::BladeConfig&,
    config::blades::Blade&,
    logging::Branch&
);

std::optional<std::string> parseStyles(
    std::istream&, config::Config&, logging::Branch&
);

std::optional<std::string> parseButtons(
    std::istream&, config::Config&, logging::Branch&
);

void tryAddInjection(const std::string& buffer, config::Config&);

} // namespace

std::optional<std::string> config::priv::parse(
    const fs::path& path, Config& config, logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("config::parse()", lBranch)};

    auto file{files::openInput(path)};
    if (not file.is_open()) {
        return errorMessage(logger, wxTRANSLATE("Failed to open config from %s"), path.string());
    }

    while (file.good()) {
        const auto prePos{file.tellg()};
        if (utils::extractComment(file)) continue;
        const auto postPos{file.tellg()};

        const auto chr{file.get()};
        if (std::isspace(chr)) continue;
        if (chr == '#') {
            std::string buffer;
            file >> buffer;
            if (buffer == "ifdef") {
                file >> buffer;
                if (buffer == "CONFIG_TOP") {
                    logger.info("Parsing top...");
                    parseTop(file, config);
                } else if (buffer == "CONFIG_PROP") {
                    logger.info("Parsing prop...");
                    parseProp(file, config);
                } else if (buffer == "CONFIG_PRESETS") {
                    auto err{parsePresets(
                        file,
                        config,
                        *logger.binfo("Parsing presets...")
                    )};
                    if (err) return err;
                } else if (buffer == "CONFIG_STYLES") {
                    auto err{parseStyles(
                        file,
                        config,
                        *logger.binfo("Parsing styles...")
                    )};
                    if (err) return err;
                } else if (buffer == "CONFIG_BUTTONS") {
                    auto err{parseButtons(
                        file,
                        config,
                        *logger.binfo("Parsing buttons...")
                    )};
                    if (err) return err;
                }
            } else if (buffer == "include") {
                getline(file, buffer);
                tryAddInjection(buffer, config);
            }
            continue;
        }
    }

    logger.info("Parsing complete, finalizing...");

    config.settings_.processDefines();

    logger.info("Done");

    return std::nullopt;
}

namespace {

std::string extractSection(std::istream& file) {
    std::string ret;
    while (file.good()) {
        if (utils::skipComment(file, &ret)) continue;

        const auto chr{file.get()};
        if (chr == '#') {
            std::string buffer;
            file >> buffer;

            if (buffer == "endif") break;

            ret += '#';
            ret += buffer;
        } else ret += static_cast<char>(chr);
    }
    return ret;
}

void parseTop(std::istream& file, config::Config& config) {
    using namespace config;
    using namespace config::priv;

    const auto top{extractSection(file)};
    std::istringstream topStream{top};
    std::istringstream commentStream;

    std::string buffer;
    while (topStream.good() or (commentStream.good() and commentStream.rdbuf()->in_avail())) {
        enum {
            eNone,
            eDefine,
            ePC_Opt,
        } type{eNone};

        if (commentStream.good() and commentStream.rdbuf()->in_avail()) {
            std::getline(commentStream, buffer);
            if (buffer.starts_with(PC_OPT_NOCOMMENT_STR)) {
                type = ePC_Opt;
                buffer = buffer.substr(PC_OPT_NOCOMMENT_STR.length());
            }
        } else {
            auto res{utils::extractComment(topStream)};
            if (res) {
                commentStream.str(*res);
                continue;
            }
            std::getline(topStream, buffer);
            if (buffer.starts_with(DEFINE_STR)) {
                type = eDefine;
                buffer = buffer.substr(DEFINE_STR.length());
            } else if (buffer.starts_with(INCLUDE_STR)) {
                buffer = buffer.substr(INCLUDE_STR.length());
                utils::trimSurroundingWhitespace(buffer);
                if (buffer.size() < 2) continue;

                // Remove quotes
                buffer = buffer.substr(1, buffer.size() - 2);

                if (not config.os()) continue;

                auto& boards{config.os()->boards_};
                for (size idx{0}; idx < boards.size(); ++idx) {
                    const auto& board{std::next(
                        boards.begin(), static_cast<ssize>(idx)
                    )->second};
                    if (board.include_ != buffer) continue;

                    config.boardChoice().choose(static_cast<int32>(idx));
                    break;
                }

                continue;
            }
        }

        if (type == eNone) continue;

        utils::trimSurroundingWhitespace(buffer);
        const auto keyEnd{buffer.find_first_of(" \t")};
        auto key{buffer.substr(0, keyEnd)};
        auto value{keyEnd == std::string::npos ? "" : buffer.substr(keyEnd)};
        utils::trimSurroundingWhitespace(value);

        if (type == eDefine and not key.empty()) {
            auto defines{data::context(config.settings_.defines_)};
            defines.append(
                std::make_unique<settings::Define>(
                    config,
                    std::move(key),
                    std::move(value)
                )
            );
        } else if (type == ePC_Opt) {
            if (key == ENABLE_MASS_STORAGE_STR) {
                config.settings_.massStorage_.set(true);
            } else if (key == ENABLE_WEBUSB_STR) {
                config.settings_.webUsb_.set(true);
            } else if (key == OS_VERSION_STR) {
                utils::Version version{value};
                if (not version) continue;

                for (size idx{0}; idx < config.osVec().size(); ++idx) {
                    auto& osVer{*config.osVec()[idx]};

                    if (osVer.version_.compare(version) != 0) continue;

                    auto osChoice{data::context(config.osChoice())};
                    osChoice.choose(static_cast<int32>(idx));

                    break;
                }
            }
        }
    }
}

void parseProp(std::istream& file, config::Config& config) {
    using namespace config;
    using namespace config::priv;

    auto propVec{config.propVec()};
    if (not propVec) return;

    std::string prop{extractSection(file)};
    std::istringstream propStream{prop};

    std::string buffer;
    while (propStream.good()) {
        if (utils::extractComment(propStream)) continue;
        std::getline(propStream, buffer);
        utils::trimSurroundingWhitespace(buffer);
        if (not buffer.starts_with(INCLUDE_STR)) continue;

        buffer = buffer.substr(INCLUDE_STR.length());
        utils::trimSurroundingWhitespace(buffer);

        static constexpr std::string_view PROP_DIR_STR{"../props/"};
        // Trim quotes
        if (buffer.size() < 2) continue;
        buffer.pop_back();
        buffer.erase(0, 1);

        if (buffer.size() < PROP_DIR_STR.length()) continue;
        buffer = buffer.substr(PROP_DIR_STR.length());

        for (auto idx{0}; idx < propVec->size(); ++idx) {
            auto& prop{static_cast<versions::props::Prop&>(*(*propVec)[idx])};

            if (prop.filename_ == buffer) {
                config.propChoice().choose(idx);
                return;
            }
        }
    }
}

std::optional<std::string> parsePresets(
    std::istream& file, config::Config& config, logging::Branch& lBranch
) {
    using namespace config;
    using namespace config::priv;

    auto& logger{lBranch.createLogger("config::parsePresets()")};
    std::string presets{extractSection(file)};
    std::istringstream presetsStream{presets};

    enum {
        eNone,
        ePreset_Array,
        eBlade_Arrays,
    } search{eNone};
    uint32 depth{};
    uint32 start{};

    std::string element;
    std::string name;
    while (presetsStream.good()) {
        if (utils::skipComment(presetsStream)) continue;

        if (search == eNone) {
            presetsStream >> element;
            if (element == "Preset") {
                // The next extraction should be the name.
                // It could be a comment, but I'm not going to worry about that
                // for now...
                presetsStream >> name;

                // This'll take care of the [] if it's there.
                utils::trimCppName(name);

                search = ePreset_Array;
                depth = 0;
            } else if (element == "BladeConfig") {
                search = eBlade_Arrays;
                depth = 0;
            } else if (element == "#") {
                presetsStream >> element;                
                if (element == "include") {
                    getline(presetsStream, element);
                    tryAddInjection(element, config);
                }
            } else if (element == "#include") {
                getline(presetsStream, element);
                tryAddInjection(element, config);
            }
        } else if (search == ePreset_Array or search == eBlade_Arrays) {
            const auto chr{presetsStream.get()};
            if (chr == '{') {
                ++depth;
                if (depth == 1) start = presetsStream.tellg();
            } else if (chr == '}') {
                if (depth == 0) {
                    if (search == ePreset_Array) return errorMessage(logger, wxTRANSLATE("Preset array is missing start { before ending };"));
                    return errorMessage(logger, wxTRANSLATE("BladeConfig array is missing start { before ending };"));
                }
                --depth;
                if (depth == 0) {
                    const uint32 dataEndPos = presetsStream.tellg();
                    // Include the '}' we're currently at to provide an extra buffer for parsePresetArray()
                    // On the off-chance the config is malformed with a missing last preset }, that may
                    // catch it. Every little bit helps the whacky configs people manage to make/find.
                    const auto data{presets.substr(start, dataEndPos - start + 1)};
                    std::optional<std::string> ret;
                    if (search == ePreset_Array) {
                        auto presetArrays{data::context(config.presetArrays_)};

                        auto& array{presetArrays.append<presets::Array>(config)};
                        array.name_.change(std::move(name));

                        ret = parsePresetArray(
                            data,
                            array,
                            *logger.binfo("Parsing preset array \"" + element + "\"...")
                        );
                    } else if (search == eBlade_Arrays) {
                        ret = parseBladeArrays(
                            data,
                            config,
                            *logger.binfo("Parsing blade arrays...")
                        );
                    }
                    if (ret) return ret;
                    search = eNone;
                }
            }
        }
    }

    if (search != eNone) logger.warn("Searching (" + std::to_string(search) + ") not complete before end.");

    return std::nullopt;
}

std::optional<std::string> parsePresetArray(
    const std::string& data,
    config::presets::Array& array,
    logging::Branch& lBranch
) {
    using namespace config;
    using namespace config::priv;

    auto& logger{lBranch.createLogger("config::parsePresetArray()")};
    std::istringstream dataStream{data};

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
    std::vector<std::string> depthBuffer;
    std::string commentBuffer;
    std::string styleBuffer;
    std::string tmp;

    const auto finishStyleReading{[&array, &styleBuffer, &commentBuffer]() {
        utils::trimSurroundingWhitespace(styleBuffer);
        utils::trimSurroundingWhitespace(commentBuffer);

        auto presets{data::context(array.presets_)};
        auto& preset{dynamic_cast<presets::Preset&>(
            *presets.children().back()
        )};
        auto styles{data::context(preset.styles_)};
        auto style{std::make_unique<presets::Style>(array.root<Config>())};

        style->comment_.change(std::move(commentBuffer));
        style->content_.change(std::move(styleBuffer));

        styles.append(std::move(style));

        commentBuffer.clear();
        styleBuffer.clear();
    }};

    auto presets{data::context(array.presets_)};

    while (dataStream.good()) {
        const auto newComments{utils::extractComment(dataStream)};
        if (newComments) {
            if (not commentBuffer.empty()) commentBuffer += '\n';
            commentBuffer += *newComments;
            continue;
        }

        const auto chr{dataStream.get()};
        if (chr == '\r') continue;

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
                if (chr != ' ' or not (inSingleQuotes or inDoubleQuotes)) {
                    continue;
                }
            }

            if (depth.empty()) {
                if (chr == '"') {
                    auto& preset{dynamic_cast<presets::Preset&>(
                        *presets.children().back()
                    )};

                    preset.name_.clear();

                    reading = eName;
                    tmp.clear();
                    continue;
                }
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

                if (not depth.empty()) logger.warn("Hit preset end before finishing style. This will mean errors to correct later!");
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
                if (depthBuffer.empty()) styleBuffer += static_cast<char>(chr);
                else depthBuffer.back() += static_cast<char>(chr);

                depth.push_back(static_cast<char>(chr));
                depthBuffer.emplace_back();
                continue;
            }
            if (chr == '>' or chr == ')') {
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

                /*
                 * TODO: Split this out of parsing so that it can be formatted
                 *       by the user any time.
                 */
                auto closedBuffer{depthBuffer.back()};
                depthBuffer.pop_back();
                std::string& readout{
                    depthBuffer.empty() ? styleBuffer : depthBuffer.back()
                };

                bool closedSplitLines{false};
                if (closedBuffer.length() > 80) {
                    closedSplitLines = true;

                    std::string insertStr;
                    uint32 closeDepth{0};
                    for (size_t idx{0}; idx < closedBuffer.length(); ++idx) {
                        if (closedBuffer[idx] == '<' or closedBuffer[idx] == '(') {
                            ++closeDepth;
                        } else if (closedBuffer[idx] == '>' or closedBuffer[idx] == ')') {
                            --closeDepth;
                            continue;
                        }

                        if (closedBuffer[idx] != ',' or closeDepth != 0) continue;

                        insertStr = '\n';
                        for (auto idx{0}; idx < depth.size(); ++idx) insertStr += '\t';
                        closedBuffer.insert(idx + 1, insertStr);
                        idx += insertStr.length();
                    }
                }

                if (closedSplitLines) {
                    readout += '\n';
                    for (auto idx{0}; idx < depth.size(); ++idx) readout += '\t';
                }
                readout += closedBuffer;
                if (closedSplitLines) {
                    readout += '\n';
                    for (auto idx{0}; idx < depth.size() - 1; ++idx) readout += '\t';
                }
                readout += static_cast<char>(chr);

                depth.pop_back();
                continue;
            }

            if (depthBuffer.empty()) styleBuffer += static_cast<char>(chr);
            else depthBuffer.back() += static_cast<char>(chr);
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

std::optional<std::string> parseBladeArrays(
    const std::string& data, config::Config& config, logging::Branch& lBranch
) {
    using namespace config;
    using namespace config::priv;

    auto& logger{lBranch.createLogger("config::parseBladeArrays()")};
    std::istringstream dataStream{data};

    enum {
        eNone,

        eID,
        eBlade_Entry,
        eConfig_Array,
        eConfig_Array_Inner,
        ePost_Config_Array,
        eName,
    } reading{eNone};

    std::string buffer;
    std::vector<char> depth;

    auto bladeConfigs{data::context(config.bladeConfigs_)};

    while (dataStream.good()) {
        if (utils::skipComment(dataStream)) continue;
        const auto chr{dataStream.get()};

        if (reading == eNone) {
            if (chr == '{') {
                bladeConfigs.append(
                    std::make_unique<blades::BladeConfig>(config)
                );
                reading = eID;
                buffer.clear();
            }
        } else if (reading == eID) {
            if (chr == ',') {
                utils::trimWhitespace(buffer);

                auto idVal{buffer == "NO_BLADE"
                    ? blades::NO_BLADE
                    : utils::doStringMath(buffer)
                };

                if (idVal) {
                    auto& lastArray{dynamic_cast<blades::BladeConfig&>(
                        *bladeConfigs.children().back()
                    )};
                    lastArray.id_.set(static_cast<int32>(*idVal));
                } else logger.warn("Invalid blade ID for array " + std::to_string(bladeConfigs.children().size()));

                reading = eBlade_Entry;
                buffer.clear();
                continue;
            }

            buffer += static_cast<char>(chr);
        } else if (reading == eBlade_Entry) {
            if (std::isspace(chr)) continue;

            if (depth.empty()) {
                if (chr == ',') {
                    auto& array{dynamic_cast<blades::BladeConfig&>(
                        *bladeConfigs.children().back()
                    )};

                    auto blades{data::context(array.blades_)};
                    auto& blade{blades.append<blades::Blade>(config)};

                    auto res{parseBlade(
                        buffer,
                        array,
                        blade,
                        *logger.binfo("Parsing blade...")
                    )};

                    auto type{data::context(blade.type())};
                    if (type.choiceIdx() == -1) {
                        logger.debug("Removing blade parser deemed unnecessary.");
                        blades.remove(blades.children().size() - 1);
                    }

                    // TODO: Shouldn't this be right after parseBlade()...?
                    if (res) return res;

                    buffer.clear();
                    continue;
                }
            }

            if (chr == '<' or chr == '(') depth.push_back(static_cast<char>(chr));
            if (chr == '>' or chr == ')') {
                if (depth.empty()) {
                    return errorMessage(logger, wxTRANSLATE("Found %c before matching open when parsing blade"), chr);
                }

                if (
                        (chr == '>' and depth.back() != '<') or
                        (chr == ')' and depth.back() != '(')
                   ) {
                    return errorMessage(logger, wxTRANSLATE("Found %c when expecting match for %c when parsing blade"), chr, depth.back());
                }

                depth.pop_back();
            }

            buffer += static_cast<char>(chr);
            if (buffer == "CONFIGARRAY") {
                reading = eConfig_Array;
                buffer.clear();
            }
        } else if (reading == eConfig_Array) {
            if (chr == '(') reading = eConfig_Array_Inner;
        } else if (reading == eConfig_Array_Inner) {
            if (chr == ')') {
                utils::trimWhitespace(buffer);

                auto presetArrays{data::context(config.presetArrays_)};
                int32 idx{0};
                for (; idx < presetArrays.children().size(); ++idx) {
                    const auto& model{presetArrays.children()[idx]};
                    auto& presetArray{dynamic_cast<presets::Array&>(*model)};

                    auto name{data::context(presetArray.name_)};
                    if (name.val() == buffer) break;
                }
                if (idx == presetArrays.children().size()) idx = -1;

                auto& array{dynamic_cast<blades::BladeConfig&>(
                    *bladeConfigs.children().back()
                )};

                array.presetArray_.choice().choose(idx);

                reading = ePost_Config_Array;
                buffer.clear();
                continue;
            }
            buffer += static_cast<char>(chr);
        } else if (reading == ePost_Config_Array) {
            if (chr == '}') reading = eNone;
            if (chr == '"') reading = eName;
        } else if (reading == eName) {
            if (chr == '"' or chr == '}') {
                auto& array{dynamic_cast<blades::BladeConfig&>(
                    *bladeConfigs.children().back()
                )};

                array.name_.change(std::move(buffer));

                reading = eNone;
                continue;
            }

            buffer += static_cast<char>(chr);
        }
    }

    return std::nullopt;
}

std::optional<std::string> parseBlade(
    std::string data,
    config::blades::BladeConfig& array,
    config::blades::Blade& blade,
    logging::Branch& lBranch
) {
    using namespace config;
    using namespace config::priv;
    using namespace config::blades;

    auto& logger{lBranch.createLogger("config::parseBlade()")};
    logger.verbose("Parsing blade \"" + data + "\"...");

    std::optional<std::string> err;

    static constexpr std::string_view DIMBLADE_STR{"DimBlade("};
    const auto parseDimBlade{[&data, &logger](int32& brightness) -> std::optional<std::string> {
        if (data.starts_with(DIMBLADE_STR)) {
            data.erase(0, DIMBLADE_STR.length());

            const auto brightCommaPos{data.find(',')};
            if (brightCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("DimBlade is missing end comma for brightness"));
            }

            const auto brightStr{data.substr(0, brightCommaPos)};
            data.erase(0, brightCommaPos + 1);

            const auto bright{utils::doStringMath(brightStr)};
            if (not bright) {
                return errorMessage(logger, wxTRANSLATE("DimBlade has malformed brightness: %s"), brightStr);
            }

            brightness = static_cast<int32>(*bright);
        }

        return std::nullopt;
    }};

    int32 firstBrightness{100};
    err = parseDimBlade(firstBrightness);
    if (err) return err;

    static constexpr std::string_view SUBBLADE_STR{"SubBlade"};
    struct {
        blades::WS281X::Split::Type type_{blades::WS281X::Split::eMax};
        int32 start_;
        int32 end_;
        int32 segments_;
        std::string list_;
    } splitData;
    if (data.starts_with(SUBBLADE_STR)) {
        data.erase(0, SUBBLADE_STR.length());
        if (data.empty()) {
            return errorMessage(logger, wxTRANSLATE("Unexpected end when parsing SubBlade"));
        }

        static constexpr std::string_view REVERSE_STR{"Reverse("};
        static constexpr std::string_view STRIDE_STR{"WithStride("};
        static constexpr std::string_view ZIGZAG_STR{"ZZ("};
        static constexpr std::string_view LIST_STR{"WithList<"};

        using enum blades::WS281X::Split::Type;
        if (data[0] == '(') {
            splitData.type_ = eStandard;
            data.erase(0, 1);
        } else if (data.starts_with(REVERSE_STR)) {
            splitData.type_ = eReverse;
            data.erase(0, REVERSE_STR.length());
        } else if (data.starts_with(STRIDE_STR)) {
            splitData.type_ = eStride;
            data.erase(0, STRIDE_STR.length());
        } else if (data.starts_with(ZIGZAG_STR)) {
            splitData.type_ = eZig_Zag;
            data.erase(0, ZIGZAG_STR.length());
        } else if (data.starts_with(LIST_STR)) {
            splitData.type_ = eList;
            data.erase(0, LIST_STR.length());
        } else {
            return errorMessage(logger, wxTRANSLATE("Encountered unknown/malformed SubBlade"));
        }

        if (
                splitData.type_ == eStandard or splitData.type_ == eReverse or
                splitData.type_ == eStride or splitData.type_ == eZig_Zag
           ) {
            auto startCommaPos{data.find(',')};
            if (startCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade start"));
            }

            auto startStr{data.substr(0, startCommaPos)};
            data.erase(0, startCommaPos + 1);

            auto start{utils::doStringMath(startStr)};
            if (not start) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade start"));
            }
            splitData.start_ = static_cast<int32>(*start);

            auto endCommaPos{data.find(',')};
            if (endCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade end"));
            }

            auto endStr{data.substr(0, endCommaPos)};
            data.erase(0, endCommaPos + 1);

            auto end{utils::doStringMath(endStr)};
            if (not end) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade end"));
            }
            splitData.end_ = static_cast<int32>(*end);
        } 
        if (splitData.type_ == eStride or splitData.type_ == eZig_Zag) {
            auto segCommaPos{data.find(',')};
            if (segCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade segments"));
            }

            auto segmentsStr{data.substr(0, segCommaPos)};
            data.erase(0, segCommaPos + 1);

            auto segments{utils::doStringMath(segmentsStr)};
            if (not segments) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade segments"));
            }
            splitData.segments_ = static_cast<int32>(*segments);
        } 
        if (splitData.type_ == eZig_Zag) {
            auto columnCommaPos{data.find(',')};
            if (columnCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade column"));
            }

            auto columnStr{data.substr(0, columnCommaPos)};
            data.erase(0, columnCommaPos + 1);

            auto column{utils::doStringMath(columnStr)};
            if (not column) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade column"));
            }

            // Column entry is not used, they're assumed to be in order.
            // Not assuming so makes this parsing more complicated and I don't
            // feel like dealing with it right now.
            //
            // All of 3 people use ZZ subblade anyways. I'll cross that bridge
            // when someone actually gets there.
        }
        if (splitData.type_ == eList) {
            // Find the `>` template closing chevron for the `SubBlade`, which
            // marks the end of the list.
            auto chevronPos{data.find('>')};
            if (chevronPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("SubBlade list unterminated"));
            }

            // The list will be processed for sanity once it's actually placed
            // into the config, just dump the raw string in for now.
            //
            // TODO: Maybe do some proper validation on this? Bad things 
            // could've happened (we've blown past into another template) here!
            splitData.list_ = data.substr(0, chevronPos);
            data.erase(0, chevronPos + 1);

            // List is unique in that it's a variadic arg'd template with the
            // indexes part of the template, so the `(` also needs to be
            // cleared, like how a comma would be cleared for the former types.
            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("Missing data after sub blade list indexes"));
            }

            if (data[0] != '(') {
                return errorMessage(logger, wxTRANSLATE("SubBlade list has invalid chars in between '>' and '('"));
            }

            data.erase(0, 1);
        }
    }

    int32 secondBrightness{100};
    err = parseDimBlade(secondBrightness);
    if (err) return err;

    const auto addSplit{[&splitData, &firstBrightness](Blade& blade) {
        auto& ws281x{blade.ws281x()};

        auto splits{data::context(ws281x.splits_)};
        auto& split{splits.append<WS281X::Split>(ws281x)};

        split.brightness_.set(firstBrightness);
        split.type_.select(splitData.type_);

        using enum WS281X::Split::Type;

        if (
                splitData.type_ == eStandard or
                splitData.type_ == eReverse or
                splitData.type_ == eZig_Zag
           ) {
            split.start_.set(splitData.start_);
            split.end_.set(splitData.end_);
        }
        if (splitData.type_ == eStride) {
            split.start_.set(splitData.start_);
            split.end_.set(splitData.end_ + splitData.segments_ - 1);
        }
        if (splitData.type_ == eStride or splitData.type_ == eZig_Zag) {
            split.segments_.set(splitData.segments_);
        }
        if (splitData.type_ == eList) {
            split.list_.change(std::move(splitData.list_));
        }
    }};
    
    const auto parseWS281XLength{[&]() -> std::optional<std::string> {
        const auto lengthCommaPos{data.find(',')};
        if (lengthCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X length"));
        }

        const auto lengthStr{data.substr(0, lengthCommaPos)};
        data.erase(0, lengthCommaPos + 1);

        const auto lengthVal{utils::doStringMath(lengthStr)};
        if (not lengthVal) {
            return errorMessage(logger, wxTRANSLATE("Failed to parse WS281X length"));
        }

        blade.ws281x().length_.set(static_cast<int32>(*lengthVal));

        return std::nullopt;
    }};

    const auto parseWS281XData{[&]() -> std::optional<std::string> {
        const auto dataPinCommaPos{data.find(',')};
        if (dataPinCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X data pin"));
        }

        auto dataPinStr{data.substr(0, dataPinCommaPos)};
        data.erase(0, dataPinCommaPos + 1);

        blade.ws281x().dataPin_.change(std::move(dataPinStr));

        return std::nullopt;
    }};

    const auto parseWS281XPowerPins{[&]() -> std::optional<std::string> {
        if (not data.starts_with(POWER_PINS_STR)) {
            return errorMessage(logger, wxTRANSLATE("Missing WS281X PowerPINS"));
        }
        data.erase(0, POWER_PINS_STR.length());

        std::string buffer;
        for (auto idx{0}; idx < data.length(); ++idx) {
            if (data[idx] == ',' or data[idx] == '>') {
                blade.ws281x().powerPins_.select(std::move(buffer));
                buffer.clear();

                if (data[idx] == '>') break;

                continue;
            }

            buffer += data[idx];
        }

        return std::nullopt;
    }};

    static constexpr std::string_view WS281X_STR{"WS281XBladePtr<"};
    static constexpr std::string_view WS2811_STR{"WS2811BladePtr<"};
    static constexpr std::string_view SIMPLE_STR{"SimpleBladePtr<"};
    static constexpr std::string_view NULL_STR{"NULL"};
    static constexpr std::string_view NULLPTR_STR{"nullptr"};
    if (data.starts_with(WS281X_STR)) {
        data.erase(0, WS281X_STR.length());
        blade.type().choice().choose(Blade::eWS281X);

        err = parseWS281XLength();
        if (err) return err;

        err = parseWS281XData();
        if (err) return err;
        
        static constexpr std::string_view COLOR8_STR{"Color8::"};
        if (not data.starts_with(COLOR8_STR)) {
            return errorMessage(logger, wxTRANSLATE("Malformatted WS281X (missing color order)"));
        }
        data.erase(0, COLOR8_STR.length());

        const auto colorOrderCommaPos{data.find(',')};
        if (colorOrderCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X color order"));
        }

        const auto colorOrderStr{data.substr(0, colorOrderCommaPos)};
        data.erase(0, colorOrderCommaPos + 1);

        const auto parse3ColorOrder{[](
            const std::string& str
        ) -> ColorOrder3 {
            size colorOrderIdx{0};
            for (; colorOrderIdx < eOrder3_Max; ++colorOrderIdx) {
                if (str == ORDER_STRS[colorOrderIdx]) break;
            }

            return static_cast<ColorOrder3>(colorOrderIdx);
        }};

        constexpr cstring INVALID_COLOR_MSG{wxTRANSLATE("Invalid/unrecognized WS281X color order: %s")};
        if (colorOrderStr.length() == 3) {
            const auto order{parse3ColorOrder(colorOrderStr)};
            if (order == eOrder3_Max) {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            auto& ws281x{blade.ws281x()};
            ws281x.hasWhite_.set(false);
            ws281x.colorOrder3_.choose(order);
        } else if (colorOrderStr.length() == 4) {
            bool offset{false};
            if (colorOrderStr[0] == 'w' or colorOrderStr[0] == 'W') {
                offset = true;
            } else if (colorOrderStr[3] != 'w' and colorOrderStr[3] != 'W') {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            const auto order3{parse3ColorOrder(
                colorOrderStr.substr(offset ? 1 : 0)
            )};
            if (order3 == eOrder3_Max) {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            const auto order4Val{order3 + 
                (offset ? eOrder4_White_First_Start : 0)
            };

            auto& ws281x{blade.ws281x()};
            ws281x.hasWhite_.set(true);
            ws281x.colorOrder4_.choose(order4Val);
        } else {
            return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
        }

        err = parseWS281XPowerPins();
        if (err) return err;
    } else if (data.starts_with(WS2811_STR)) {
        data.erase(0, WS2811_STR.length());
        blade.type().choice().choose(Blade::eWS281X);

        err = parseWS281XLength();
        if (err) return err;
        
        const auto configCommaPos{data.find(',')};
        if (configCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Missing end comma for WS2811 config"));
        }

        auto configStr{data.substr(0, configCommaPos)};
        data.erase(0, configCommaPos + 1);

        // For the config bitmask, all I care to extract is the color order.
        // I don't support the other options for WS281X and no one uses them.
        for (size idx{0}; idx < eOrder3_Max; ++idx) {
            if (configStr.find(ORDER_STRS[idx]) != std::string::npos) {
                blade.ws281x().colorOrder3_.choose(static_cast<int32>(idx));
                break;
            }
        }

        err = parseWS281XData();
        if (err) return err;

        err = parseWS281XPowerPins();
        if (err) return err;
    } else if (data.starts_with(SIMPLE_STR)) {
        if (splitData.type_ != WS281X::Split::eMax) {
            return errorMessage(logger, wxTRANSLATE("Attempted to SubBlade simple blade"));
        }

        data.erase(0, SIMPLE_STR.length());
        blade.type().choice().choose(Blade::eSimple);

        blade.brightness_.set(firstBrightness);
        auto& simple{blade.simple()};

        const auto ledFromIdx{[&](size idx) -> Simple::LED& {
            switch (idx) {
                case 0: return simple.led1_;
                case 1: return simple.led2_;
                case 2: return simple.led3_;
                case 3: return simple.led4_;
                default:
                    assert(0);
                    __builtin_unreachable();
            }
        }};

        const auto parseProfile{[&](size ledIdx) -> std::optional<std::string> {
            auto& led{ledFromIdx(ledIdx)};

            const auto ledCommaPos{data.find(',')};
            if (ledCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing end comma for SimpleBlade LED %u"), ledIdx);
            }

            const auto ledStr{data.substr(0, ledCommaPos)};
            data.erase(0, ledCommaPos + 1);

            size profileIdx{0};
            for (; profileIdx < eLED_Max; ++profileIdx) {
                const auto& testLedStr{LED_STRS[profileIdx]};
                if (not ledStr.starts_with(testLedStr)) continue;

                led.profile_.choose(static_cast<int32>(profileIdx));

                if (
                        profileIdx >= eLED_Use_Resistance_Start and
                        profileIdx <= eLED_Use_Resistance_End
                   ) {
                    // At least long enough for chevrons
                    if (ledStr.length() < testLedStr.length() + 2) {
                        return errorMessage(logger, wxTRANSLATE("Simple blade which uses resistance missing chevrons: %s"), ledStr);
                    }

                    // Make sure chevrons are there
                    if (ledStr[testLedStr.length()] != '<' or ledStr.back() != '>') {
                        return errorMessage(logger, wxTRANSLATE("Simple blade which uses resistance malformed: %s"), ledStr);
                    }

                    // Parse the in-between
                    auto resistanceStr{ledStr.substr(
                        testLedStr.length() + 1,
                        ledStr.length() - testLedStr.length() - 2
                    )};
                    auto resistanceVal{utils::doStringMath(resistanceStr)};
                    
                    if (not resistanceVal) {
                        return errorMessage(logger, wxTRANSLATE("Simple blade has invalid resistance: %s"), ledStr);
                    }

                    led.resistance_.set(static_cast<int32>(*resistanceVal));
                } else {
                    // If it doesn't use resistance, should match length exactly
                    if (ledStr.length() != testLedStr.length()) {
                        return errorMessage(logger, wxTRANSLATE("Invalid/unrecognized LED for SimpleBlade: %s"), ledStr);
                    }
                }

                break;
            }

            if (profileIdx == eLED_Max) {
                return errorMessage(logger, wxTRANSLATE("Unknown/malformed LED in SimpleBlade: %s"), ledStr);
            }

            return std::nullopt;
        }};

        err = parseProfile(0);
        if (err) return err;
        err = parseProfile(1);
        if (err) return err;
        err = parseProfile(2);
        if (err) return err;
        err = parseProfile(3);
        if (err) return err;

        const auto parsePin{[&](size ledIdx) -> std::optional<std::string> {
            auto& led{ledFromIdx(ledIdx)};

            const auto pinCommaPos{data.find(ledIdx == 3 ? '>' : ',')};
            if (pinCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing end comma/chevron for SimpleBlade power pin %u"), ledIdx + 1);
            }

            auto pinStr{data.substr(0, pinCommaPos)};
            data.erase(0, pinCommaPos + 1);

            if (pinStr != "-1") {
                led.powerPin_.change(std::move(pinStr));
            }

            return std::nullopt;
        }};

        err = parsePin(0);
        if (err) return err;
        err = parsePin(1);
        if (err) return err;
        err = parsePin(2);
        if (err) return err;
        err = parsePin(3);
        if (err) return err;
        
        auto led1Profile{data::context(simple.led1_.profile_)};
        auto led2Profile{data::context(simple.led2_.profile_)};
        auto led3Profile{data::context(simple.led3_.profile_)};
        auto led4Profile{data::context(simple.led4_.profile_)};

        if (
                led1Profile.idx() == eLED_None and
                led2Profile.idx() == eLED_None and
                led3Profile.idx() == eLED_None and
                led4Profile.idx() == eLED_None
           ) {
            blade.type().choice().choose(Blade::eUnassigned);
        }
    } else if (data.starts_with(NULL_STR) or data.starts_with(NULLPTR_STR)) {
        blade.type().choice().unchoose();

        auto blades{data::context(array.blades_)};

        if (blades.children().size() == 1) {
            return errorMessage(logger, wxTRANSLATE("SubBlade with no blade found first in array"));
        }

        auto& blade{dynamic_cast<Blade&>(
            *blades.children()[blades.children().size() - 2]
        )};

        auto type{data::context(blade.type().choice())};
        if (type.idx() != Blade::eWS281X) {
            return errorMessage(logger, wxTRANSLATE("Tried to add SubBlade to a non-WS281X blade"));
        }

        auto splits{data::context(blade.ws281x().splits_)};

        if (splits.children().empty()) {
            return errorMessage(logger, wxTRANSLATE("Tried to add SubBlade to a non-split WS281X blade"));
        }

        auto& lastSplit{dynamic_cast<WS281X::Split&>(
            *splits.children().back()
        )};

        auto lastSplitType{data::context(lastSplit.type_)};
        if (lastSplitType.selected() != splitData.type_) {
            addSplit(blade);
        } else { // this split is same type as last split
            if (
                    lastSplitType.selected() == WS281X::Split::eStandard or
                    lastSplitType.selected() == WS281X::Split::eReverse or
                    lastSplitType.selected() == WS281X::Split::eList
               ) {
                // These types aren't segmented, just add.
                addSplit(blade);
            } else if (lastSplitType.selected() == WS281X::Split::eStride) {
                auto segments{data::context(lastSplit.segments_)};
                auto start{data::context(lastSplit.start_)};
                auto end{data::context(lastSplit.end_)};

                if (
                        // Just make sure this split is same segments
                        // and the start falls inside last split
                        segments.val() != splitData.segments_ or
                        start.val() > splitData.start_ or
                        end.val() < splitData.start_
                   ) {
                    addSplit(blade);
                }
                // Last split is same as this. Nothing to do.
            } else if (lastSplitType.selected() == WS281X::Split::eZig_Zag) {
                auto segments{data::context(lastSplit.segments_)};
                auto start{data::context(lastSplit.start_)};
                auto end{data::context(lastSplit.end_)};

                if (
                        segments.val() != splitData.segments_ or
                        start.val() != splitData.start_ or
                        end.val() != splitData.end_
                   ) {
                    addSplit(blade);
                }
                // Last split is same as this. Nothing to do.
            }
        }
    } else {
        return errorMessage(logger, wxTRANSLATE("Unknown/malformed blade"));
    }

    auto type{data::context(blade.type().choice())};

    if (type.idx() == Blade::eWS281X) {
        if (splitData.type_ == WS281X::Split::eMax) {
            blade.brightness_.set(firstBrightness);
        } else {
            blade.brightness_.set(secondBrightness);
            addSplit(blade);
        }
    }

    return std::nullopt;
}

std::optional<std::string> parseStyles(
    std::istream& file, config::Config& config, logging::Branch& lBranch
) {
    using namespace config;
    using namespace config::priv;

    auto& logger{lBranch.createLogger("config::parseStyles()")};
    std::string styles{extractSection(file)};
    std::istringstream stylesStream{styles};

    enum {
        eNone,
        eStyle,
        eStyle_Name,
    } reading{eNone};

    std::string readHistory;

    std::string comments;
    std::string bladestyle;
    std::string name;

    while (stylesStream.good()) {
        auto newComments{utils::extractComment(stylesStream)};
        if (newComments) {
            if (not comments.empty()) comments += '\n';
            comments += *newComments;
            continue;
        }

        const auto chr{stylesStream.get()};
        if (chr == '\r') continue;

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

                auto presetArrays{data::context(config.presetArrays_)};

                // How many nested for loops would you like? Cause here are all of 'em
                for (const auto& model : presetArrays.children()) {
                    auto& array{dynamic_cast<presets::Array&>(*model)};
                    auto presets{data::context(array.presets_)};

                    for (const auto& model : presets.children()) {
                        auto& preset{dynamic_cast<presets::Preset&>(*model)};
                        auto styles{data::context(preset.styles_)};

                        for (const auto& model : styles.children()) {
                            auto& style{dynamic_cast<presets::Style&>(*model)};

                            auto content{data::context(style.content_)};
                            auto comment{data::context(style.comment_)};

                            for (;;) { // Just because I can lol, another for loop
                                const auto usingStylePos{content.val().find(name)};
                                if (usingStylePos == std::string::npos) break;

                                auto tmp{content.val()};
                                tmp.erase(usingStylePos, name.length());
                                tmp.insert(usingStylePos, bladestyle);
                                content.change(std::move(tmp));

                                if (not comment.val().empty())
                                    comment.append('\n');

                                comment.append(std::string{comments});
                            }
                        }
                    }
                }

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

std::optional<std::string> parseButtons(
    std::istream& file, config::Config& config, logging::Branch& lBranch
) {
    using namespace config;
    using namespace config::priv;

    auto& logger{lBranch.createLogger("config::parseButtons()")};
    std::string sectStr{extractSection(file)};
    std::istringstream buttonsStream{sectStr};

    enum {
        eNone,
        eType,
        ePost_Type,
        eCpp_Name,
        ePost_Cpp_Name,
        eInner,
        ePost_Inner,
    } reading{eNone};

    std::string typeStr;
    std::string inner;
    char openChar{};

    while (buttonsStream.good()) {
        if (utils::extractComment(buttonsStream)) {
            if (reading == eType) reading = ePost_Type;
            if (reading == eCpp_Name) reading = ePost_Cpp_Name;
            continue;
        }

        const auto chr{buttonsStream.get()};

        if (reading == eNone) {
            if (std::isgraph(chr)) {
                reading = eType;
                typeStr = static_cast<char>(chr);
                continue;
            }
        } else if (reading == eType) {
            if (std::isspace(chr)) {
                reading = ePost_Type;
                continue;
            }

            typeStr += static_cast<char>(chr);
        } else if (reading == ePost_Type) {
            if (std::isgraph(chr)) {
                reading = eCpp_Name;
                continue;
            }
        } else if (reading == eCpp_Name or reading == ePost_Cpp_Name) {
            if (chr == '{' or chr == '(') {
                openChar = static_cast<char>(chr);
                reading = eInner;
                continue;
            }

            if (reading == eCpp_Name) {
                if (std::isspace(chr)) reading = ePost_Cpp_Name;
            } else { // POST_CPP_NAME
                if (std::isspace(chr)) continue;

                // Not a space and not an opener:
                return errorMessage(logger, wxTRANSLATE("Unexpected character %#x post cpp name."), chr);
            }
        } else if (reading == eInner) {
            if (
                    openChar == '{' and chr == '}' or 
                    openChar == '(' and chr == ')'
               ) {
                reading = ePost_Inner;
                continue;
            }

            if (std::isgraph(chr)) {
                inner += static_cast<char>(chr);
            }
        } else if (reading == ePost_Inner) {
            if (std::isspace(chr)) continue;
            if (chr != ';') {
                return errorMessage(logger, wxTRANSLATE("Unexpected character %#x post inner (prior to ;)."), chr);
            }

            reading = eNone;

            auto buttons{data::context(config.buttons_)};
            auto& button{buttons.append<buttons::Button>(config)};

            int32 typeIdx{0};
            for (; typeIdx < BUTTON_TYPE_STRS.size(); ++typeIdx) {
                if (BUTTON_TYPE_STRS[typeIdx] == typeStr) break;
            }
            if (typeIdx == BUTTON_TYPE_STRS.size()) {
                return errorMessage(logger, wxTRANSLATE("Unknown button type: %s"), typeStr);
            } 

            button.type_.choose(typeIdx);

            auto eventCommaPos{inner.find(',')};
            if (eventCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing comma for button event."));
            }

            auto eventStr{inner.substr(0, eventCommaPos)};
            inner.erase(0, eventCommaPos + 1);

            int32 evtIdx{-1};
            if (eventStr == "BUTTON_FIRE") evtIdx = eBtn_Evt_Up;
            else if (eventStr == "BUTTON_MODE_SELECT") evtIdx = eBtn_Evt_Down;
            else if (eventStr == "BUTTON_CLIP_DETECT") evtIdx = eBtn_Evt_Left;
            else if (eventStr == "BUTTON_RELOAD") evtIdx = eBtn_Evt_Right;
            else if (eventStr == "BUTTON_RANGE") evtIdx = eBtn_Evt_Select;

            if (evtIdx == -1) {
                evtIdx = 0;
                for (; evtIdx < BUTTON_EVENT_STRS.size(); ++evtIdx) {
                    if (BUTTON_EVENT_STRS[evtIdx] == eventStr) break;
                }
            }

            if (evtIdx == BUTTON_EVENT_STRS.size()) {
                logger.warn("Unknown button event: " + eventStr);
                button.event_.choose(0);
            } else {
                button.event_.choose(evtIdx);
            }

            auto pinCommaPos{inner.find(',')};
            if (pinCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing comma for button pin."));
            }

            auto pinStr{inner.substr(0, pinCommaPos)};
            inner.erase(0, pinCommaPos + 1);
            button.pin_.change(std::move(pinStr));

            if (typeIdx == eBtn_Type_Touch) {
                auto threshCommaPos{inner.find(',')};
                if (threshCommaPos == std::string::npos) {
                    return errorMessage(logger, wxTRANSLATE("Missing comma for touch button threshold."));
                }

                auto threshStr{inner.substr(0, threshCommaPos)};
                inner.erase(0, threshCommaPos + 1);

                auto thresh{utils::doStringMath(threshStr)};
                if (not thresh) {
                    logger.warn("Button threshold value \"" + threshStr + "\" could not be parsed.");
                } else {
                    button.touch_.set(static_cast<int32>(*thresh));
                }
            }

            if (inner.front() != '"' or inner.back() != '"') {
                logger.warn("Button name doesn't seem to be surrounded by quotes, is it malformatted?");
                button.name_.change(std::move(inner));
            } else {
                button.name_.change(inner.substr(1, inner.length() - 2));
            }

            inner.erase();
            continue;
        }
    }

    return std::nullopt;
}

void tryAddInjection(const std::string& buffer, config::Config& config) {
    using namespace config;
    using namespace config::priv;

    auto& logger{logging::Context::getGlobal().createLogger("config::tryAddInjection()")};

    auto strStart{buffer.find('"')};
    if (strStart == std::string::npos) return;
    auto strEnd{buffer.find('"', strStart + 1)};
    if (strEnd == std::string::npos) return;

    auto injectionPos{buffer.find(INJECTION_STR, strStart + 1)};
    std::string injectionFile;
    if (injectionPos != std::string::npos) {
        logger.verbose("Injection string found...");
        injectionFile = buffer.substr(
            injectionPos + INJECTION_STR.length() + 1,
            strEnd - injectionPos - INJECTION_STR.length() - 1
        );
    } else {
        logger.verbose("Injection string missing...");
        injectionFile = buffer.substr(strStart + 1, strEnd - strStart - 1);
    }

    logger.debug("Injection file: " + injectionFile); 
    if (
            injectionFile.find("../") != std::string::npos or
            injectionFile.find("/..") != std::string::npos
       ) {
        pcui::showMessage(
            wxString::Format(
                _("Injection file \"%s\" has an invalid name and cannot be registered.") + '\n' +
                _("You may add a substitute after import."),
                injectionFile
            ),
            {.caption_=_("Unknown Injection Encountered")}
        );
        return;
    }
    auto filePath{paths::injectionDir() / injectionFile};
    std::error_code err;
    if (not fs::exists(filePath, err)) {
        const auto registerChoice{pcui::showMessage(
            wxString::Format(
                _("Injection file \"%s\" has not been registered.") + '\n' +
                _("Would you like to add the injection file now?"),
                injectionFile
            ),
            {
                .caption_=_("Unknown Injection Encountered"),
                .style_=wxYES_NO | wxYES_DEFAULT
            }
        )};
        if (wxYES != registerChoice) return;

        while (not false) {
            wxFileDialog fileDialog{
                nullptr,
                "Choose injection file for \"" + injectionFile + '"',
                wxEmptyString,
                wxEmptyString,
                "C Header (*.h)|*.h",
                wxFD_OPEN | wxFD_FILE_MUST_EXIST
            };
            if (fileDialog.ShowModal() == wxID_CANCEL) return;

            auto copyPath{paths::injectionDir() / filePath};
            fs::create_directories(copyPath.parent_path());
            if (not files::copyOverwrite(fileDialog.GetPath().ToStdString(), copyPath, err)) {
                auto choice{pcui::showMessage(
                    err.message(),
                    {
                        .caption_=_("Injection file could not be added."),
                        .style_=wxOK | wxCANCEL | wxOK_DEFAULT
                    }
                )};

                if (choice == wxCANCEL) return;

                continue;
            }

            injectionFile = fileDialog.GetFilename().ToStdString();
            break;
        }
    }

    config.injections_.append(
        std::make_unique<Injection>(config, std::move(injectionFile))
    );

    logger.debug("Done");
}

} // namespace

