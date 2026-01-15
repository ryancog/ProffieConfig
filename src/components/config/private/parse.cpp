#include "io.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * components/config/private/parse.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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
#include <sstream>

#include <wx/filedlg.h>
#include <wx/translation.h>

#include "config/settings/settings.h"
#include "log/branch.h"
#include "log/context.h"
#include "ui/message.h"
#include "utils/paths.h"
#include "utils/string.h"
#include "utils/version.h"
#include "versions/prop.h"

namespace {

string extractSection(std::istream&);

void parseTop(std::istream&, Config::Config&);
void parseProp(std::istream&, Config::Config&);
optional<string> parsePresets(std::istream&, Config::Config&, Log::Branch&);
optional<string> parsePresetArray(const string& data, Config::PresetArray&, Log::Branch&);
optional<string> parseBladeArrays(const string& data, Config::Config&, Log::Branch&);
optional<string> parseBlade(string data, Config::BladeConfig&, Config::Blade&, Log::Branch&);
optional<string> parseStyles(std::istream&, Config::Config&, Log::Branch&);
optional<string> parseButtons(std::istream&, Config::Config&, Log::Branch&);

void tryAddInjection(const string& buffer, Config::Config&);

} // namespace

optional<string> Config::parse(const filepath& path, Config& config, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Config::parse()", lBranch)};

    auto file{Paths::openInputFile(path)};
    if (not file.is_open()) {
        return errorMessage(logger, wxTRANSLATE("Failed to open config from %s"), path.string());
    }

    while (file.good()) {
        const auto prePos{file.tellg()};
        if (Utils::extractComment(file)) continue;
        const auto postPos{file.tellg()};

        const auto chr{file.get()};
        if (std::isspace(chr)) continue;
        if (chr == '#') {
            string buffer;
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

    config.presetArrays.syncStyles();
    config.settings.processCustomDefines();
    config.processCustomToPropOptions();

    logger.info("Done");

    return nullopt;
}

namespace {

string extractSection(std::istream& file) {
    string ret;
    while (file.good()) {
        if (Utils::skipComment(file, &ret)) continue;

        const auto chr{file.get()};
        if (chr == '#') {
            string buffer;
            file >> buffer;

            if (buffer == "endif") break;

            ret += '#';
            ret += buffer;
        } else ret += static_cast<char>(chr);
    }
    return ret;
}

void parseTop(std::istream& file, Config::Config& config) {
    using namespace Config;

    const auto top{extractSection(file)};
    std::istringstream topStream{top};
    std::istringstream commentStream;

    string buffer;
    while (topStream.good() or (commentStream.good() and commentStream.rdbuf()->in_avail())) {
        enum {
            NONE,
            DEFINE,
            PC_OPT,
        } type{NONE};

        if (commentStream.good() and commentStream.rdbuf()->in_avail()) {
            std::getline(commentStream, buffer);
            if (buffer.starts_with(PC_OPT_NOCOMMENT_STR)) {
                type = PC_OPT;
                buffer = buffer.substr(PC_OPT_NOCOMMENT_STR.length());
            }
        } else {
            auto res{Utils::extractComment(topStream)};
            if (res) {
                commentStream.str(*res);
                continue;
            }
            std::getline(topStream, buffer);
            if (buffer.starts_with(DEFINE_STR)) {
                type = DEFINE;
                buffer = buffer.substr(DEFINE_STR.length());
            } else if (buffer.starts_with(INCLUDE_STR)) {
                buffer = buffer.substr(INCLUDE_STR.length());
                Utils::trimSurroundingWhitespace(buffer);
                if (buffer.size() < 2) continue;

                for (auto idx{0}; idx < BOARD_MAX; ++idx) {
                    if (buffer == BOARD_STRS[idx]) {
                        config.settings.board = idx;
                        break;
                    }
                }
                continue;
            }
        }

        if (type == NONE) continue;

        Utils::trimSurroundingWhitespace(buffer);
        const auto keyEnd{buffer.find_first_of(" \t")};
        auto key{buffer.substr(0, keyEnd)};
        auto value{keyEnd == string::npos ? "" : buffer.substr(keyEnd)};
        Utils::trimSurroundingWhitespace(value);

        if (type == DEFINE and not key.empty()) {
            config.settings.addCustomOption(std::move(key), std::move(value));
        } else if (type == PC_OPT) {
            if (key == ENABLE_MASS_STORAGE_STR) {
                config.settings.massStorage = true;
            } else if (key == ENABLE_WEBUSB_STR) {
                config.settings.webUSB = true;
            } else if (key == OS_VERSION_STR) {
                Utils::Version version{value};
                if (not version) continue;
                auto& osVersionMap{config.settings.osVersionMap};
                for (auto idx{0}; idx < osVersionMap.size(); ++idx) {
                    if (version == osVersionMap[idx]) {
                        config.settings.osVersion = idx;
                        break;
                    }
                }
            }
        }
    }
}

void parseProp(std::istream& file, Config::Config& config) {
    using namespace Config;
    string prop{extractSection(file)};
    std::istringstream propStream{prop};

    string buffer;
    while (propStream.good()) {
        if (Utils::extractComment(propStream)) continue;
        std::getline(propStream, buffer);
        Utils::trimSurroundingWhitespace(buffer);
        if (not buffer.starts_with(INCLUDE_STR)) continue;

        buffer = buffer.substr(INCLUDE_STR.length());
        Utils::trimSurroundingWhitespace(buffer);

        static constexpr string_view PROP_DIR_STR{"../props/"};
        // Trim quotes
        if (buffer.size() < 2) continue;
        buffer.pop_back();
        buffer.erase(0, 1);

        if (buffer.size() < PROP_DIR_STR.length()) continue;
        buffer = buffer.substr(PROP_DIR_STR.length());

        for (auto idx{0}; idx < config.props().size(); ++idx) {
            auto& prop{config.prop(idx)};
            if (prop.filename == buffer) {
                config.propSelection = idx;
                return;
            }
        }
    }
}

optional<string> parsePresets(std::istream& file, Config::Config& config, Log::Branch& lBranch) {
    using namespace Config;
    auto& logger{lBranch.createLogger("Config::parsePresets()")};
    string presets{extractSection(file)};
    std::istringstream presetsStream{presets};

    enum {
        NONE,
        PRESET_ARRAY,
        BLADE_ARRAYS,
    } search{NONE};
    uint32 depth{};
    uint32 start{};

    string element;
    while (presetsStream.good()) {
        if (Utils::skipComment(presetsStream)) continue;

        if (search == NONE) {
            presetsStream >> element;
            if (element == "Preset") {
                presetsStream >> element;
                search = PRESET_ARRAY;
                depth = 0;
            } else if (element == "BladeConfig") {
                search = BLADE_ARRAYS;
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
        } else if (search == PRESET_ARRAY or search == BLADE_ARRAYS) {
            const auto chr{presetsStream.get()};
            if (chr == '{') {
                ++depth;
                if (depth == 1) start = presetsStream.tellg();
            } else if (chr == '}') {
                if (depth == 0) {
                    if (search == PRESET_ARRAY) return errorMessage(logger, wxTRANSLATE("Preset array is missing start { before ending };"));
                    return errorMessage(logger, wxTRANSLATE("BladeConfig array is missing start { before ending };"));
                }
                --depth;
                if (depth == 0) {
                    const uint32 dataEndPos = presetsStream.tellg();
                    // Include the '}' we're currently at to provide an extra buffer for parsePresetArray()
                    // On the off-chance the config is malformed with a missing last preset }, that may
                    // catch it. Every little bit helps the whacky configs people manage to make/find.
                    const string data{presets.substr(start, dataEndPos - start + 1)};
                    optional<string> ret;
                    if (search == PRESET_ARRAY) {
                        auto& array{config.presetArrays.addArray(element)};
                        ret = parsePresetArray(data, array, *logger.binfo("Parsing preset array \"" + element + "\"..."));
                    } else if (search == BLADE_ARRAYS) {
                        ret = parseBladeArrays(data, config, *logger.binfo("Parsing blade arrays..."));
                    }
                    if (ret) return ret;
                    search = NONE;
                }
            }
        }
    }

    if (search != NONE) logger.warn("Searching (" + std::to_string(search) + ") not complete before end.");

    return nullopt;
}

optional<string> parsePresetArray(const string& data, Config::PresetArray& array, Log::Branch& lBranch) {
    using namespace Config;
    auto& logger{lBranch.createLogger("Config::parsePresetArray()")};
    std::istringstream dataStream{data};

    enum {
        NONE,

        POST_BRACE,
        DIR,
        POST_DIR,
        TRACK,
        POST_TRACK,

        STYLE,

        NAME,
    } reading{NONE};

    vector<char> depth;
    vector<string> depthBuffer;
    string commentBuffer;
    string styleBuffer;
    string tmp;

    const auto finishStyleReading{[&array, &styleBuffer, &commentBuffer]() {
        Utils::trimSurroundingWhitespace(styleBuffer);
        Utils::trimSurroundingWhitespace(commentBuffer);

        auto& style{array.presets().back()->addStyle()};
        style.comment = std::move(commentBuffer);
        style.style = std::move(styleBuffer);

        commentBuffer.clear();
        styleBuffer.clear();
    }};

    while (dataStream.good()) {
        const auto newComments{Utils::extractComment(dataStream)};
        if (newComments) {
            if (not commentBuffer.empty()) commentBuffer += '\n';
            commentBuffer += *newComments;
            continue;
        }

        const auto chr{dataStream.get()};
        if (chr == '\r') continue;

        if (reading == NONE) {
            if (chr == '{') {
                reading = POST_BRACE;
                array.addPreset();
            }
        } else if (reading == POST_BRACE or reading == POST_DIR) {
            if (chr == '"') {
                if (reading == POST_BRACE) {
                    reading = DIR;
                } else if (reading == POST_DIR) {
                    reading = TRACK;
                }
            }
        } else if (reading == DIR) {
            if (chr == '"') {
                reading = POST_DIR;
                continue;
            }
            array.presets().back()->fontDir += static_cast<char>(chr);
        } else if (reading == TRACK) {
            if (chr == '"') {
                reading = POST_TRACK;
                continue;
            }
            array.presets().back()->track += static_cast<char>(chr);
        } else if (reading == POST_TRACK) {
            if (chr == ',') {
                reading = STYLE;
            }
        } else if (reading == STYLE) {
            bool inSingleQuotes{not depth.empty() and depth.back() == '\''};
            bool inDoubleQuotes{not depth.empty() and depth.back() == '"'};
            if (std::isspace(chr)) {
                if (chr != ' ' or not (inSingleQuotes or inDoubleQuotes)) {
                    continue;
                }
            }

            if (depth.empty()) {
                if (chr == '"') {
                    array.presets().back()->name.clear();
                    reading = NAME;
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
                auto& preset{*array.presets().back()};
                preset.name = "preset" + std::to_string(array.presets().size());
                reading = NONE;
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
                string& readout{depthBuffer.empty() ? styleBuffer : depthBuffer.back()};

                bool closedSplitLines{false};
                if (closedBuffer.length() > 80) {
                    closedSplitLines = true;

                    string insertStr;
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
        } else if (reading == NAME) {
            const auto& preset{array.presets().back()};
            if (chr == '"' or chr == '}') {
                reading = NONE;
                if (tmp.empty()) preset->name = "preset" + std::to_string(array.presets().size());
                else preset->name = std::move(tmp);
                continue;
            }
            
            tmp += static_cast<char>(chr);
        }
    }

    return nullopt;
}

optional<string> parseBladeArrays(const string& data, Config::Config& config, Log::Branch& lBranch) {
    using namespace Config;
    auto& logger{lBranch.createLogger("Config::parseBladeArrays()")};
    std::istringstream dataStream{data};

    enum {
        NONE,

        ID,
        BLADE_ENTRY,
        CONFIGARRAY,
        CONFIGARRAY_INNER,
        POST_CONFIGARRAY,
        NAME,
    } reading{NONE};

    string buffer;
    vector<char> depth;

    while (dataStream.good()) {
        if (Utils::skipComment(dataStream)) continue;
        const auto chr{dataStream.get()};

        if (reading == NONE) {
            if (chr == '{') {
                config.bladeArrays.addArray();
                reading = ID;
                buffer.clear();
            }
        } else if (reading == ID) {
            if (chr == ',') {
                Utils::trimWhitespace(buffer);

                auto id{buffer == "NO_BLADE"
                    ? NO_BLADE
                    : Utils::doStringMath(buffer)
                };

                if (id) {
                    auto& lastArray{*config.bladeArrays.arrays().back()};
                    lastArray.id = static_cast<int32>(*id);
                } else logger.warn("Invalid blade ID for array " + std::to_string(config.bladeArrays.arrays().size()));

                reading = BLADE_ENTRY;
                buffer.clear();
                continue;
            }

            buffer += static_cast<char>(chr);
        } else if (reading == BLADE_ENTRY) {
            if (std::isspace(chr)) continue;

            if (depth.empty()) {
                if (chr == ',') {
                    auto& array{*config.bladeArrays.arrays().back()};
                    auto& blade{array.addBlade()};
                    auto res{parseBlade(
                        buffer,
                        array,
                        blade,
                        *logger.binfo("Parsing blade...")
                    )};
                    if (blade.type == Blade::INVALID) {
                        logger.debug("Removing blade parser deemed unnecessary.");
                        array.removeBlade(array.blades().size() - 1);
                    }
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
                reading = CONFIGARRAY;
                buffer.clear();
            }
        } else if (reading == CONFIGARRAY) {
            if (chr == '(') reading = CONFIGARRAY_INNER;
        } else if (reading == CONFIGARRAY_INNER) {
            if (chr == ')') {
                Utils::trimWhitespace(buffer);
                config.bladeArrays.arrays().back()->presetArray = buffer;
                reading = POST_CONFIGARRAY;
                buffer.clear();
                continue;
            }
            buffer += static_cast<char>(chr);
        } else if (reading == POST_CONFIGARRAY) {
            if (chr == '}') reading = NONE;
            if (chr == '"') reading = NAME;
        } else if (reading == NAME) {
            if (chr == '"' or chr == '}') {
                config.bladeArrays.arrays().back()->name = std::move(buffer);
                reading = NONE;
                continue;
            }

            buffer += static_cast<char>(chr);
        }
    }

    return nullopt;
}

optional<string> parseBlade(string data, Config::BladeConfig& array, Config::Blade& blade, Log::Branch& lBranch) {
    using namespace Config;
    auto& logger{lBranch.createLogger("Config::parseBlade()")};
    logger.verbose("Parsing blade \"" + data + "\"...");

    optional<string> err;

    static constexpr string_view DIMBLADE_STR{"DimBlade("};
    const auto parseDimBlade{[&data, &logger](int32& brightness) -> optional<string> {
        if (data.starts_with(DIMBLADE_STR)) {
            data.erase(0, DIMBLADE_STR.length());

            const auto brightCommaPos{data.find(',')};
            if (brightCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("DimBlade is missing end comma for brightness"));
            }

            const auto brightStr{data.substr(0, brightCommaPos)};
            data.erase(0, brightCommaPos + 1);

            const auto bright{Utils::doStringMath(brightStr)};
            if (not bright) {
                return errorMessage(logger, wxTRANSLATE("DimBlade has malformed brightness: %s"), brightStr);
            }

            brightness = static_cast<int32>(*bright);
        }

        return nullopt;
    }};

    int32 firstBrightness{100};
    err = parseDimBlade(firstBrightness);
    if (err) return err;

    static constexpr string_view SUBBLADE_STR{"SubBlade"};
    struct {
        Split::Type type{Split::TYPE_MAX};
        int32 start;
        int32 end;
        int32 segments;
        string list;
    } splitData;
    if (data.starts_with(SUBBLADE_STR)) {
        data.erase(0, SUBBLADE_STR.length());
        if (data.empty()) {
            return errorMessage(logger, wxTRANSLATE("Unexpected end when parsing SubBlade"));
        }

        static constexpr string_view REVERSE_STR{"Reverse("};
        static constexpr string_view STRIDE_STR{"WithStride("};
        static constexpr string_view ZIGZAG_STR{"ZZ("};
        static constexpr string_view LIST_STR{"WithList<"};
        if (data[0] == '(') {
            splitData.type = Split::STANDARD;
            data.erase(0, 1);
        } else if (data.starts_with(REVERSE_STR)) {
            splitData.type = Split::REVERSE;
            data.erase(0, REVERSE_STR.length());
        } else if (data.starts_with(STRIDE_STR)) {
            splitData.type = Split::STRIDE;
            data.erase(0, STRIDE_STR.length());
        } else if (data.starts_with(ZIGZAG_STR)) {
            splitData.type = Split::ZIG_ZAG;
            data.erase(0, ZIGZAG_STR.length());
        } else if (data.starts_with(LIST_STR)) {
            splitData.type = Split::LIST;
            data.erase(0, LIST_STR.length());
        } else {
            return errorMessage(logger, wxTRANSLATE("Encountered unknown/malformed SubBlade"));
        }

        if (
                splitData.type == Split::STANDARD or splitData.type == Split::REVERSE or
                splitData.type == Split::STRIDE or splitData.type == Split::ZIG_ZAG
           ) {
            auto startCommaPos{data.find(',')};
            if (startCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade start"));
            }

            auto startStr{data.substr(0, startCommaPos)};
            data.erase(0, startCommaPos + 1);

            auto start{Utils::doStringMath(startStr)};
            if (not start) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade start"));
            }
            splitData.start = static_cast<int32>(*start);

            auto endCommaPos{data.find(',')};
            if (endCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade end"));
            }

            auto endStr{data.substr(0, endCommaPos)};
            data.erase(0, endCommaPos + 1);

            auto end{Utils::doStringMath(endStr)};
            if (not end) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade end"));
            }
            splitData.end = static_cast<int32>(*end);
        } 
        if (splitData.type == Split::STRIDE or splitData.type == Split::ZIG_ZAG) {
            auto segCommaPos{data.find(',')};
            if (segCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade segments"));
            }

            auto segmentsStr{data.substr(0, segCommaPos)};
            data.erase(0, segCommaPos + 1);

            auto segments{Utils::doStringMath(segmentsStr)};
            if (not segments) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade segments"));
            }
            splitData.segments = static_cast<int32>(*segments);
        } 
        if (splitData.type == Split::ZIG_ZAG) {
            auto columnCommaPos{data.find(',')};
            if (columnCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade column"));
            }

            auto columnStr{data.substr(0, columnCommaPos)};
            data.erase(0, columnCommaPos + 1);

            auto column{Utils::doStringMath(columnStr)};
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
        if (splitData.type == Split::LIST) {
            // Find the `>` template closing chevron for the `SubBlade`, which
            // marks the end of the list.
            auto chevronPos{data.find('>')};
            if (chevronPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("SubBlade list unterminated"));
            }

            // The list will be processed for sanity once it's actually placed
            // into the config, just dump the raw string in for now.
            //
            // TODO: Maybe do some proper validation on this? Bad things 
            // could've happened (we've blown past into another template) here!
            splitData.list = data.substr(0, chevronPos);
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
        auto& split{blade.ws281x().addSplit()};
        split.brightness = firstBrightness;
        split.type = splitData.type;
        if (split.type == Split::STANDARD or split.type == Split::REVERSE or split.type == Split::ZIG_ZAG) {
            split.start = splitData.start;
            split.end = splitData.end;
        }
        if (split.type == Split::STRIDE) {
            split.start = splitData.start;
            split.end = splitData.end + splitData.segments - 1;
        }
        if (split.type == Split::STRIDE or split.type == Split::ZIG_ZAG) {
            split.segments = splitData.segments;
        }
        if (split.type == Split::LIST) {
            split.list = std::move(splitData.list);
        }
    }};
    
    const auto parseWS281XLength{[&]() -> optional<string> {
        const auto lengthCommaPos{data.find(',')};
        if (lengthCommaPos == string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X length"));
        }

        const auto lengthStr{data.substr(0, lengthCommaPos)};
        data.erase(0, lengthCommaPos + 1);

        const auto length{Utils::doStringMath(lengthStr)};
        if (not length) {
            return errorMessage(logger, wxTRANSLATE("Failed to parse WS281X length"));
        }

        blade.ws281x().length = static_cast<int32>(*length);
        return nullopt;
    }};

    const auto parseWS281XData{[&]() -> optional<string> {
        const auto dataPinCommaPos{data.find(',')};
        if (dataPinCommaPos == string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X data pin"));
        }

        auto dataPinStr{data.substr(0, dataPinCommaPos)};
        data.erase(0, dataPinCommaPos + 1);

        blade.ws281x().dataPin = std::move(dataPinStr);
        return nullopt;
    }};

    const auto parseWS281XPowerPins{[&]() -> optional<string> {
        if (not data.starts_with(POWER_PINS_STR)) {
            return errorMessage(logger, wxTRANSLATE("Missing WS281X PowerPINS"));
        }
        data.erase(0, POWER_PINS_STR.length());

        string buffer;
        for (auto idx{0}; idx < data.length(); ++idx) {
            if (data[idx] == ',' or data[idx] == '>') {
                // Use entry for processing
                blade.config().bladeArrays.powerPinNameEntry = std::move(buffer);
                blade.ws281x().powerPins.select(blade.config().bladeArrays.powerPinNameEntry);
                buffer.clear();
                if (data[idx] == '>') break;
                continue;
            }

            buffer += data[idx];
        }
        return nullopt;
    }};

    static constexpr string_view WS281X_STR{"WS281XBladePtr<"};
    static constexpr string_view WS2811_STR{"WS2811BladePtr<"};
    static constexpr string_view SIMPLE_STR{"SimpleBladePtr<"};
    static constexpr string_view NULL_STR{"NULL"};
    static constexpr string_view NULLPTR_STR{"nullptr"};
    if (data.starts_with(WS281X_STR)) {
        data.erase(0, WS281X_STR.length());
        blade.type = Blade::WS281X;

        err = parseWS281XLength();
        if (err) return err;

        err = parseWS281XData();
        if (err) return err;
        
        static constexpr string_view COLOR8_STR{"Color8::"};
        if (not data.starts_with(COLOR8_STR)) {
            return errorMessage(logger, wxTRANSLATE("Malformatted WS281X (missing color order)"));
        }
        data.erase(0, COLOR8_STR.length());

        const auto colorOrderCommaPos{data.find(',')};
        if (colorOrderCommaPos == string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X color order"));
        }

        const auto colorOrderStr{data.substr(0, colorOrderCommaPos)};
        data.erase(0, colorOrderCommaPos + 1);

        const auto parse3ColorOrder{[](
            const string& str
        ) -> WS281XBlade::ColorOrder3 {
            auto colorOrderIdx{0};
            for (; colorOrderIdx < WS281XBlade::ORDER_MAX; ++colorOrderIdx) {
                if (str == WS281XBlade::ORDER_STRS[colorOrderIdx]) break;
            }

            return static_cast<WS281XBlade::ColorOrder3>(colorOrderIdx);
        }};

        constexpr cstring INVALID_COLOR_MSG{wxTRANSLATE("Invalid/unrecognized WS281X color order: %s")};
        if (colorOrderStr.length() == 3) {
            const auto order{parse3ColorOrder(colorOrderStr)};
            if (order == WS281XBlade::ORDER_MAX) {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            blade.ws281x().hasWhite = false;
            blade.ws281x().colorOrder3 = order;
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
            if (order3 == WS281XBlade::ORDER_MAX) {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            const auto order4{
                static_cast<WS281XBlade::ColorOrder4>(order3) + 
                (offset ? WS281XBlade::ORDER4_WFIRST_START : 0)
            };

            blade.ws281x().hasWhite = true;
            blade.ws281x().colorOrder4 = order4;
                
        } else {
            return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
        }

        err = parseWS281XPowerPins();
        if (err) return err;
    } else if (data.starts_with(WS2811_STR)) {
        data.erase(0, WS2811_STR.length());
        blade.type = Blade::WS281X;

        err = parseWS281XLength();
        if (err) return err;
        
        const auto configCommaPos{data.find(',')};
        if (configCommaPos == string::npos) {
            return errorMessage(logger, wxTRANSLATE("Missing end comma for WS2811 config"));
        }

        auto configStr{data.substr(0, configCommaPos)};
        data.erase(0, configCommaPos + 1);

        // For the config bitmask, all I care to extract is the color order.
        // I don't support the other options for WS281X and no one uses them.
        for (auto idx{0}; idx < WS281XBlade::ORDER_MAX; ++idx) {
            if (configStr.find(WS281XBlade::ORDER_STRS[idx]) != string::npos) {
                blade.ws281x().colorOrder3 = idx;
                break;
            }
        }

        err = parseWS281XData();
        if (err) return err;

        err = parseWS281XPowerPins();
        if (err) return err;
    } else if (data.starts_with(SIMPLE_STR)) {
        if (splitData.type != Split::TYPE_MAX) {
            return errorMessage(logger, wxTRANSLATE("Attempted to SubBlade simple blade"));
        }

        data.erase(0, SIMPLE_STR.length());
        blade.type = Blade::SIMPLE;

        blade.brightness = firstBrightness;
        auto& simple{blade.simple()};

        const auto selectStar{[&simple](uint32 starIdx) -> SimpleBlade::Star& {
            switch (starIdx) {
                case 0: return simple.star1;
                case 1: return simple.star2;
                case 2: return simple.star3;
                case 3: return simple.star4;
                default: assert(0);
                __builtin_unreachable();
            }
        }};

        const auto parseLED{[&](uint32 starIdx) -> optional<string> {
            auto& star{selectStar(starIdx)};

            const auto ledCommaPos{data.find(',')};
            if (ledCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing end comma for SimpleBlade LED %u"), starIdx + 1);
            }

            const auto ledStr{data.substr(0, ledCommaPos)};
            data.erase(0, ledCommaPos + 1);

            auto ledIdx{0};
            for (; ledIdx < SimpleBlade::Star::LED_MAX; ++ledIdx) {
                const auto& testLedStr{SimpleBlade::Star::LED_STRS[ledIdx]};
                if (not ledStr.starts_with(testLedStr)) continue;

                star.led = ledIdx;

                if (
                        ledIdx >= SimpleBlade::Star::USE_RESISTANCE_START and
                        ledIdx <= SimpleBlade::Star::USE_RESISTANCE_END
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
                    auto resistance{Utils::doStringMath(resistanceStr)};
                    
                    if (not resistance) {
                        return errorMessage(logger, wxTRANSLATE("Simple blade has invalid resistance: %s"), ledStr);
                    }

                    star.resistance = static_cast<int32>(*resistance);
                } else {
                    // If it doesn't use resistance, should match length exactly
                    if (ledStr.length() != testLedStr.length()) {
                        return errorMessage(logger, wxTRANSLATE("Invalid/unrecognized LED for SimpleBlade: %s"), ledStr);
                    }
                }

                break;
            }

            if (ledIdx == SimpleBlade::Star::LED_MAX) {
                return errorMessage(logger, wxTRANSLATE("Unknown/malformed LED in SimpleBlade: %s"), ledStr);
            }

            return nullopt;
        }};

        err = parseLED(0);
        if (err) return err;
        err = parseLED(1);
        if (err) return err;
        err = parseLED(2);
        if (err) return err;
        err = parseLED(3);
        if (err) return err;

        const auto parsePin{[&](uint32 starIdx) -> optional<string> {
            auto& star{selectStar(starIdx)};

            const auto pinCommaPos{data.find(starIdx == 3 ? '>' : ',')};
            if (pinCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing end comma/chevron for SimpleBlade power pin %u"), starIdx + 1);
            }

            auto pinStr{data.substr(0, pinCommaPos)};
            data.erase(0, pinCommaPos + 1);

            if (pinStr != "-1") star.powerPin = std::move(pinStr);

            return nullopt;
        }};

        err = parsePin(0);
        if (err) return err;
        err = parsePin(1);
        if (err) return err;
        err = parsePin(2);
        if (err) return err;
        err = parsePin(3);
        if (err) return err;
        
        if (
                simple.star1.led == SimpleBlade::Star::LED::NONE and
                simple.star2.led == SimpleBlade::Star::LED::NONE and
                simple.star3.led == SimpleBlade::Star::LED::NONE and
                simple.star4.led == SimpleBlade::Star::LED::NONE
           ) {
            blade.type = Blade::UNASSIGNED;
        }
    } else if (data.starts_with(NULL_STR) or data.starts_with(NULLPTR_STR)) {
        blade.type = Blade::INVALID;
        if (array.blades().size() == 1) {
            return errorMessage(logger, wxTRANSLATE("SubBlade with no blade found first in array"));
        }
        auto& blade{array.blade(array.blades().size() - 2)};
        if (blade.type != Blade::WS281X) {
            return errorMessage(logger, wxTRANSLATE("Tried to add SubBlade to a non-WS281X blade"));
        }
        if (blade.ws281x().splits().empty()) {
            return errorMessage(logger, wxTRANSLATE("Tried to add SubBlade to a non-split WS281X blade"));
        }

        auto& lastSplit{*blade.ws281x().splits().back()};
        if (lastSplit.type != splitData.type) addSplit(blade);
        else { // this split is same type as last split
            if (
                    lastSplit.type == Split::STANDARD or
                    lastSplit.type == Split::REVERSE or
                    lastSplit.type == Split::LIST
               ) {
                // These types aren't segmented, just add.
                addSplit(blade);
            } else if (lastSplit.type == Split::STRIDE) {
                if (
                        // Just make sure this split is same segments
                        // and the start falls inside last split
                        lastSplit.segments != splitData.segments or
                        lastSplit.start > splitData.start or
                        lastSplit.end < splitData.start
                   ) {
                    addSplit(blade);
                }
                // Last split is same as this. Nothing to do.
            } else if (lastSplit.type == Split::ZIG_ZAG) {
                if (
                        lastSplit.segments != splitData.segments or
                        lastSplit.start != splitData.start or
                        lastSplit.end != splitData.end
                   ) {
                    addSplit(blade);
                }
                // Last split is same as this. Nothing to do.
            }
        }
    } else {
        return errorMessage(logger, wxTRANSLATE("Unknown/malformed blade"));
    }

    if (blade.type == Blade::WS281X) {
        if (splitData.type == Split::TYPE_MAX) {
            blade.brightness = firstBrightness;
        } else {
            blade.brightness = secondBrightness;
            addSplit(blade);
        }
    }

    return nullopt;
}

optional<string> parseStyles(std::istream& file, Config::Config& config, Log::Branch& lBranch) {
    using namespace Config;
    auto& logger{lBranch.createLogger("Config::parseStyles()")};
    string styles{extractSection(file)};
    std::istringstream stylesStream{styles};

    enum {
        NONE,
        STYLE,
        STYLE_NAME,
    } reading{NONE};

    string readHistory;

    string comments;
    string bladestyle;
    string name;

    while (stylesStream.good()) {
        auto newComments{Utils::extractComment(stylesStream)};
        if (newComments) {
            if (not comments.empty()) comments += '\n';
            comments += *newComments;
            continue;
        }

        const auto chr{stylesStream.get()};
        if (chr == '\r') continue;

        if (reading == NONE) {
            if (name.empty()) {
                readHistory += static_cast<char>(chr);
                if (readHistory.rfind("using") != string::npos) {
                    reading = STYLE_NAME;
                    readHistory.clear();
                }
            } else if (chr == '=') {
                reading = STYLE;
            }
        } else if (reading == STYLE_NAME) {
            if (std::isspace(chr)) {
                if (not name.empty()) {
                    reading = NONE;
                }
                continue;
            }

            name += static_cast<char>(chr);
        } else if (reading == STYLE) {
            if (chr == ';') {
                Utils::trimWhitespaceOutsideString(bladestyle);

                // How many nested for loops would you like? Cause here are all of 'em
                for (const auto& presetArray : config.presetArrays.arrays()) {
                    for (const auto& preset : presetArray->presets()) {
                        for (const auto& style : preset->styles()) {
                            for (;;) { // Just because I can lol, another for loop
                                const auto usingStylePos{style->style.find(name)};
                                if (usingStylePos == string::npos) break;

                                style->style.erase(usingStylePos, name.length());
                                style->style.insert(usingStylePos, bladestyle);
                                if (not style->comment.empty()) style->comment += '\n';
                                style->comment += comments;
                            }
                        }
                    }
                }

                name.clear();
                bladestyle.clear();
                comments.clear();
                reading = NONE;
                continue;
            }

            bladestyle += static_cast<char>(chr);
        }
    }

    return nullopt;
}

optional<string> parseButtons(std::istream& file, Config::Config& config, Log::Branch& lBranch) {
    using namespace Config;
    auto& logger{lBranch.createLogger("Config::parseButtons()")};
    string sectStr{extractSection(file)};
    std::istringstream buttonsStream{sectStr};

    enum {
        NONE,
        TYPE,
        POST_TYPE,
        CPP_NAME,
        POST_CPP_NAME,
        INNER,
        POST_INNER,
    } reading{NONE};

    string type;
    string inner;
    char openChar{};

    while (buttonsStream.good()) {
        if (Utils::extractComment(buttonsStream)) {
            if (reading == TYPE) reading = POST_TYPE;
            if (reading == CPP_NAME) reading = POST_CPP_NAME;
            continue;
        }

        const auto chr{buttonsStream.get()};

        if (reading == NONE) {
            if (std::isgraph(chr)) {
                reading = TYPE;
                type = static_cast<char>(chr);
                continue;
            }
        } else if (reading == TYPE) {
            if (std::isspace(chr)) {
                reading = POST_TYPE;
                continue;
            }

            type += static_cast<char>(chr);
        } else if (reading == POST_TYPE) {
            if (std::isgraph(chr)) {
                reading = CPP_NAME;
                continue;
            }
        } else if (reading == CPP_NAME or reading == POST_CPP_NAME) {
            if (chr == '{' or chr == '(') {
                openChar = static_cast<char>(chr);
                reading = INNER;
                continue;
            }

            if (reading == CPP_NAME) {
                if (std::isspace(chr)) reading = POST_CPP_NAME;
            } else { // POST_CPP_NAME
                if (std::isspace(chr)) continue;

                // Not a space and not an opener:
                return errorMessage(logger, wxTRANSLATE("Unexpected character %#x post cpp name."), chr);
            }
        } else if (reading == INNER) {
            if (
                    openChar == '{' and chr == '}' or 
                    openChar == '(' and chr == ')'
               ) {
                reading = POST_INNER;
                continue;
            }

            if (std::isgraph(chr)) {
                inner += static_cast<char>(chr);
            }
        } else if (reading == POST_INNER) {
            if (std::isspace(chr)) continue;
            if (chr != ';') {
                return errorMessage(logger, wxTRANSLATE("Unexpected character %#x post inner (prior to ;)."), chr);
            }

            reading = NONE;
            auto button{std::make_unique<Settings::ButtonData>()};

            int32 typeIdx{0};
            for (; typeIdx < BUTTON_TYPE_STRS.size(); ++typeIdx) {
                if (BUTTON_TYPE_STRS[typeIdx] == type) break;
            }
            if (typeIdx == BUTTON_TYPE_STRS.size()) {
                return errorMessage(logger, wxTRANSLATE("Unknown button type: %s"), type);
            } 
            button->type.setValue(typeIdx);

            auto eventCommaPos{inner.find(',')};
            if (eventCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing comma for button event."));
            }

            auto eventStr{inner.substr(0, eventCommaPos)};
            inner.erase(0, eventCommaPos + 1);

            int32 evtIdx{-1};
            if (eventStr == "BUTTON_FIRE") evtIdx = UP;
            else if (eventStr == "BUTTON_MODE_SELECT") evtIdx = DOWN;
            else if (eventStr == "BUTTON_CLIP_DETECT") evtIdx = LEFT;
            else if (eventStr == "BUTTON_RELOAD") evtIdx = RIGHT;
            else if (eventStr == "BUTTON_RANGE") evtIdx = SELECT;

            if (evtIdx == -1) {
                evtIdx = 0;
                for (; evtIdx < BUTTON_EVENT_STRS.size(); ++evtIdx) {
                    if (BUTTON_EVENT_STRS[evtIdx] == eventStr) break;
                }
            }

            if (evtIdx == BUTTON_EVENT_STRS.size()) {
                logger.warn("Unknown button event: " + eventStr);
                button->event = 0;
            } else {
                button->event = evtIdx;
            }

            auto pinCommaPos{inner.find(',')};
            if (pinCommaPos == string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing comma for button pin."));
            }

            auto pinStr{inner.substr(0, pinCommaPos)};
            inner.erase(0, pinCommaPos + 1);
            button->pin = std::move(pinStr);

            if (button->type == TOUCH_BUTTON) {
                auto threshCommaPos{inner.find(',')};
                if (threshCommaPos == string::npos) {
                    return errorMessage(logger, wxTRANSLATE("Missing comma for touch button threshold."));
                }

                auto threshStr{inner.substr(0, threshCommaPos)};
                inner.erase(0, threshCommaPos + 1);

                auto thresh{Utils::doStringMath(threshStr)};
                if (not thresh) {
                    logger.warn("Button threshold value \"" + threshStr + "\" could not be parsed.");
                } else {
                    button->touch = static_cast<int32>(*thresh);
                }
            }

            if (inner.front() != '"' or inner.back() != '"') {
                logger.warn("Button name doesn't seem to be surrounded by quotes, is it malformatted?");
                button->name = std::move(inner);
            } else {
                button->name = inner.substr(1, inner.length() - 2);
            }

            config.settings.addButton(std::move(button));

            inner.erase();
            continue;
        }
    }

    return nullopt;
}

void tryAddInjection(const string& buffer, Config::Config& config) {
    using namespace Config;
    auto& logger{Log::Context::getGlobal().createLogger("Config::tryAddInjection()")};

    auto strStart{buffer.find('"')};
    if (strStart == string::npos) return;
    auto strEnd{buffer.find('"', strStart + 1)};
    if (strEnd == string::npos) return;

    auto injectionPos{buffer.find(INJECTION_STR, strStart + 1)};
    string injectionFile;
    if (injectionPos != string::npos) {
        logger.verbose("Injection string found...");
        injectionFile = buffer.substr(injectionPos + INJECTION_STR.length() + 1, strEnd - injectionPos - INJECTION_STR.length() - 1);
    } else {
        logger.verbose("Injection string missing...");
        injectionFile = buffer.substr(strStart + 1, strEnd - strStart - 1);
    }

    logger.debug("Injection file: " + injectionFile); 
    if (injectionFile.find("../") != string::npos or injectionFile.find("/..") != string::npos) {
        PCUI::showMessage(
            wxString::Format(_("Injection file \"%s\" has an invalid name and cannot be registered.\nYou may add a substitute after import."), injectionFile),
            _("Unknown Injection Encountered")
        );
        return;
    }
    auto filePath{Paths::injectionDir() / injectionFile};
    std::error_code err;
    if (not fs::exists(filePath, err)) {
        if (wxYES != PCUI::showMessage(wxString::Format(_("Injection file \"%s\" has not been registered.\nWould you like to add the injection file now?"), injectionFile), _("Unknown Injection Encountered"), wxYES_NO | wxYES_DEFAULT)) {
            return;
        }

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

            auto copyPath{Paths::injectionDir() / filePath};
            fs::create_directories(copyPath.parent_path());
            if (not Paths::copyOverwrite(fileDialog.GetPath().ToStdString(), copyPath, err)) {
                auto res{PCUI::showMessage(err.message(), _("Injection file could not be added."), wxOK | wxCANCEL | wxOK_DEFAULT)};
                if (res == wxCANCEL) return;

                continue;
            }

            injectionFile = fileDialog.GetFilename().ToStdString();
            break;
        }
    }

    config.presetArrays.addInjection(injectionFile);
    logger.debug("Done");
}

} // namespace

