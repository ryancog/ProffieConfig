#include "io.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
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

#include "log/branch.h"
#include "log/context.h"
#include "ui/message.h"
#include "paths/paths.h"
#include "utils/string.h"
#include "utils/version.h"
#include "versions/versions.h"

namespace Config {
    string extractSection(std::ifstream&);

    void parseTop(std::ifstream&, Config&);
    void parseProp(std::ifstream&, Config&);
    optional<string> parsePresets(std::ifstream&, Config&, Log::Branch&);
    optional<string> parsePresetArray(const string& data, PresetArray&, Log::Branch&);
    optional<string> parseBladeArrays(const string& data, Config&, Log::Branch&);
    optional<string> parseBlade(string data, BladeConfig&, Blade&, Log::Branch&);
    optional<string> parseStyles(std::ifstream&, Config&, Log::Branch&);

    void tryAddInjection(const string& buffer, Config&);
} // namespace Config


optional<string> Config::parse(const filepath& path, Config& config, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Config::parse()", lBranch)};

    std::ifstream file{path};
    if (not file.is_open()) {
        return errorMessage(logger, wxTRANSLATE("Failed to open config from %s"), path.string());
    }

    while (file.good()) {
        if (Utils::extractComment(file)) continue;

        const auto chr{file.get()};
        if (std::isspace(chr)) continue;
        if (chr == '#') {
            string buffer;
            file >> buffer;
            if (buffer == "ifdef") {
                file >> buffer;
                if (buffer == "CONFIG_TOP") parseTop(file, config);
                else if (buffer == "CONFIG_PROP") parseProp(file, config);
                else if (buffer == "CONFIG_PRESETS") {
                    auto ret{parsePresets(
                        file,
                        config,
                        *logger.binfo("Parsing presets...")
                    )};
                    if (ret) return ret;
                } else if (buffer == "CONFIG_STYLES") {
                    auto ret{parseStyles(
                        file,
                        config,
                        *logger.binfo("Parsing styles...")
                    )};
                    if (ret) return ret;
                }
            } else if (buffer == "include") {
                getline(file, buffer);
                tryAddInjection(buffer, config);
            }
            continue;
        }
    }

    config.presetArrays.syncStyles();
    config.settings.processCustomDefines();

    return nullopt;
}

// bool Config::importConfig(Config& editor) {
//     wxFileDialog configLocation(editor, "Choose ProffieOS Config File", "", "", "C Header Files (*.h)|*.h", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
// 
//     if (configLocation.ShowModal() == wxID_CANCEL) return false; // User Closed
// 
//     return Config::readConfig(configLocation.GetPath().ToStdString(), editor);
// }

string Config::extractSection(std::ifstream& file) {
    string ret;
    while (file.good()) {
        if (Utils::skipComment(file, &ret)) continue;

        const auto chr{file.get()};
        if (chr == '#') {
            string buffer;
            file >> buffer;
            if (buffer == "endif") break;
            else {
                ret += '#';
                ret += buffer;
            }
        } else ret += chr;
    }
    return ret;
}

void Config::parseTop(std::ifstream& file, Config& config) {
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

                for (auto idx{0}; idx < Settings::BOARD_MAX; ++idx) {
                    if (buffer == Settings::BOARD_STRS[idx]) {
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
            if (key == Settings::ENABLE_MASS_STORAGE_STR) {
                config.settings.massStorage = true;
            } else if (key == Settings::ENABLE_WEBUSB_STR) {
                config.settings.webUSB = true;
            } else if (key == Settings::OS_VERSION_STR) {
                Utils::Version version{value};
                if (version.err) continue;
                const auto& osVersions{Versions::getOSVersions()};
                for (auto idx{0}; idx < osVersions.size(); ++idx) {
                    if (version == osVersions[idx].verNum) {
                        config.settings.osVersion = idx;
                        break;
                    }
                }
            }
        }
    }
}

void Config::parseProp(std::ifstream& file, Config& config) {
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

optional<string> Config::parsePresets(std::ifstream& file, Config& config, Log::Branch& lBranch) {
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
                    else return errorMessage(logger, wxTRANSLATE("BladeConfig array is missing start { before ending };"));
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

optional<string> Config::parsePresetArray(const string& data, PresetArray& array, Log::Branch& lBranch) {
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
            if (std::isspace(chr)) continue;
            if (depth.empty()) {
                if (chr == '"') {
                    array.presets().back()->name.clear();
                    reading = NAME;
                    continue;
                }
                if (chr == ',') {
                    finishStyleReading();
                    continue;
                }
            }

            if (chr == '}') {
                finishStyleReading();
                if (not depth.empty()) logger.warn("Hit preset end before finishing style. This will mean errors to correct later!");
                depth.clear();
                auto& preset{*array.presets().back()};
                if (preset.name.empty()) preset.name = "preset" + std::to_string(array.presets().size());
                reading = NONE;
                continue;
            }

            if (chr == '<' or chr == '(') {
                if (depthBuffer.empty()) styleBuffer += chr;
                else depthBuffer.back() += chr;

                depth.push_back(chr);
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

                auto closedBuffer{depthBuffer.back()};
                depthBuffer.pop_back();
                string& readout{depthBuffer.empty() ? styleBuffer : depthBuffer.back()};

                bool closedSplitLines{false};
                if (closedBuffer.length() > 80) {
                    closedSplitLines = true;

                    string insertStr;
                    uint32 closeDepth{0};
                    for (auto idx{0}; idx < closedBuffer.length(); ++idx) {
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
                readout += chr;

                depth.pop_back();
                continue;
            }

            if (depthBuffer.empty()) styleBuffer += chr;
            else depthBuffer.back() += chr;
        } else if (reading == NAME) {
            if (chr == '"' or chr == '}') {
                reading = NONE;
                continue;
            }
            
            array.presets().back()->name += chr;
        }
    }

    return nullopt;
}

optional<string> Config::parseBladeArrays(const string& data, Config& config, Log::Branch& lBranch) {
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
                const auto id{
                    buffer == "NO_BLADE" ?
                        NO_BLADE :
                        std::strtol(buffer.c_str(), nullptr, 10)
                };
                config.bladeArrays.arrays().back()->id = id;
                reading = BLADE_ENTRY;
                buffer.clear();
                continue;
            }

            buffer += static_cast<char>(chr);
        } else if (reading == BLADE_ENTRY) {
            if (std::isspace(chr)) continue;

            if (depth.empty()) {
                if (chr == ',') {
                    Utils::trimWhitespace(buffer);
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

            if (chr == '<' or chr == '(') depth.push_back(chr);
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
            buffer += chr;
        } else if (reading == POST_CONFIGARRAY) {
            if (chr == '}') reading = NONE;
            if (chr == '"') reading = NAME;
        } else if (reading == NAME) {
            if (chr == '"' or chr == '}') {
                config.bladeArrays.arrays().back()->name = std::move(buffer);
                reading = NONE;
                continue;
            }

            buffer += chr;
        }
    }

    return nullopt;
}

optional<string> Config::parseBlade(string data, BladeConfig& array, Blade& blade, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Config::parseBlade()")};
    logger.verbose("Parsing blade \"" + data + "\"...");

    static constexpr string_view DIMBLADE_STR{"DimBlade("};
    const auto parseDimBlade{[&data, &logger](uint32& brightness) -> optional<string> {
        if (data.starts_with(DIMBLADE_STR)) {
            data.erase(0, DIMBLADE_STR.length());

            size_t numProcessed{};
            try {
                brightness = std::stod(data.c_str(), &numProcessed);
            } catch (std::exception e) {
                return errorMessage(logger, wxTRANSLATE("DimBlade has malformed brightness: %s"), e.what());
            }
            data.erase(0, numProcessed);

            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("DimBlade is missing blade"));
            }

            // Now should start at blade. Skipped ','
            data.erase(0, 1);
        }

        return nullopt;
    }};

    uint32 firstBrightness{100};
    auto res{parseDimBlade(firstBrightness)};
    if (res) return res;

    static constexpr string_view SUBBLADE_STR{"SubBlade"};
    struct {
        Split::Type type{Split::TYPE_MAX};
        uint32 start;
        uint32 end;
        uint32 segments;
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
            size_t numProcessed{};
            try {
                splitData.start = std::stoi(data.c_str(), &numProcessed);
            } catch (std::exception e) {
                return errorMessage(logger, wxTRANSLATE("Failed to read SubBlade start: %s"), e.what());
            }
            data.erase(0, numProcessed);
            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("SubBlade missing data after start"));
            }

            data.erase(0, 1); // ','

            try {
                splitData.end = std::stoi(data.c_str(), &numProcessed);
            } catch (std::exception e) {
                return errorMessage(logger, wxTRANSLATE("Failed to read SubBlade end: %s"), e.what());
            }
            data.erase(0, numProcessed);
            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("SubBlade missing data after end"));
            }

            data.erase(0, 1); // ','
        } 
        if (splitData.type == Split::STRIDE or splitData.type == Split::ZIG_ZAG) {
            size_t numProcessed{};
            try {
                splitData.segments = std::stoi(data.c_str(), &numProcessed);
            } catch (std::exception e) {
                return errorMessage(logger, wxTRANSLATE("Failed to read SubBlade stride: %s"), e.what());
            }
            data.erase(0, numProcessed);
            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("SubBlade missing data after stride"));
            }

            data.erase(0, 1); // ','
        } 
        if (splitData.type == Split::ZIG_ZAG) {
            size_t numProcessed{};
            uint32 dummyColumn;
            try {
                dummyColumn = std::stoi(data.c_str(), &numProcessed);
            } catch (std::exception e) {
                return errorMessage(logger, wxTRANSLATE("Failed to read SubBlade column: %s"), e.what());
            }
            data.erase(0, numProcessed);
            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("SubBlade missing data after column"));
            }

            data.erase(0, 1); // ','
        }
        if (splitData.type == Split::LIST) {
            auto idx{0};
            for (; idx < data.length(); ++idx) {
                if (data[idx] == '>') break;
            }
            if (idx == data.length()) {
                return errorMessage(logger, wxTRANSLATE("SubBlade list unterminated"));
            }
            if (idx + 2 >= data.length()) {
                return errorMessage(logger, wxTRANSLATE("SubBlade missing blade after list"));
            }

            splitData.list = data.substr(0, idx);
            data.erase(0, idx + 2); // 'list>('
        }
    }

    uint32 secondBrightness{100};
    res = parseDimBlade(secondBrightness);
    if (res) return res;

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

    static constexpr string_view WS281X_STR{"WS281XBladePtr<"};
    static constexpr string_view WS2811_STR{"WS2811BladePtr<"};
    static constexpr string_view SIMPLE_STR{"SimpleBladePtr<"};
    static constexpr string_view NULL_STR{"NULL"};
    static constexpr string_view NULLPTR_STR{"nullptr"};
    if (data.starts_with(WS281X_STR)) {
        data.erase(0, WS281X_STR.length());
        blade.type = Blade::WS281X;

        size_t idx;
        try {
            blade.ws281x().length = std::stoi(data.c_str(), &idx);
        } catch (std::exception e) {
            return errorMessage(logger, wxTRANSLATE("Failed to read WS281X length: %s"), e.what());
        }
        if (idx + 1 >= data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after WS281X length"));
        }
        data.erase(0, idx + 1); // length,

        idx = 0;
        for (; idx < data.length(); ++idx) {
            if (data[idx] == ',') break;
            blade.ws281x().dataPin += data[idx];
        }
        if (idx == data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after WS281X data pin"));
        }
        data.erase(0, idx + 1);

        static constexpr string_view COLOR8_STR{"Color8::"};
        if (not data.starts_with(COLOR8_STR)) {
            return errorMessage(logger, wxTRANSLATE("Malformatted WS281X color order"));
        }
        data.erase(0, COLOR8_STR.length());
        const auto colorOrderEnd{data.find(',')};
        if (colorOrderEnd == string::npos) {
            return errorMessage(logger, wxTRANSLATE("Missing data after WS281X color order"));
        }
        const auto colorOrder{data.substr(0, colorOrderEnd)};
        idx = 0;
        for (; idx < WS281XBlade::ORDER_MAX; ++idx) {
            if (colorOrder.find(WS281XBlade::ORDER_STRS[idx]) != string::npos) {
                blade.ws281x().colorOrder3 = idx;                
                break;
            }
        }
        const auto whitePos{colorOrder.find_first_of("Ww")};
        blade.ws281x().hasWhite = whitePos != string::npos;
        if (blade.ws281x().hasWhite) {
            blade.ws281x().useRGBWithWhite = data[whitePos] == 'W';
            if (whitePos == 0) {
                const auto selection{static_cast<uint32>(blade.ws281x().colorOrder4)};
                blade.ws281x().colorOrder4 = selection + WS281XBlade::ORDER4_WFIRST_START;
            }
        }
        data.erase(0, colorOrderEnd + 1);

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
    } else if (data.starts_with(WS2811_STR)) {
        data.erase(0, WS2811_STR.length());
        blade.type = Blade::WS281X;

        size_t idx;
        try {
            blade.ws281x().length = std::stoi(data.c_str(), &idx);
        } catch (std::exception e) {
            return errorMessage(logger, wxTRANSLATE("Failed to read WS2811 length: %s"), e.what());
        }
        if (idx + 1 >= data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after WS2811 length"));
        }
        data.erase(0, idx + 1); // length,
        
        const auto configEnd{data.find(',')};
        string buffer;
        buffer = data.substr(0, configEnd);
        for (auto idx{0}; idx < WS281XBlade::ORDER_MAX; ++idx) {
            if (buffer.find(WS281XBlade::ORDER_STRS[idx]) != string::npos) {
                blade.ws281x().colorOrder3 = idx;
                break;
            }
        }
        data.erase(0, configEnd);
        if (data.empty()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after WS2811 config"));
        }

        idx = 0;
        for (; idx < data.length(); ++idx) {
            if (data[idx] == ',') break;
            blade.ws281x().dataPin += data[idx];
        }
        if (idx == data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after WS2811 data pin"));
        }
        data.erase(0, idx + 1);

        if (not data.starts_with(POWER_PINS_STR)) {
            return errorMessage(logger, wxTRANSLATE("Missing WS2811 PowerPINS"));
        }
        data.erase(0, POWER_PINS_STR.length());
        buffer.clear();
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
    } else if (data.starts_with(SIMPLE_STR)) {
        if (splitData.type != Split::TYPE_MAX) {
            return errorMessage(logger, wxTRANSLATE("Attempted to SubBlade simple blade"));
        }

        data.erase(0, SIMPLE_STR.length());
        blade.type = Blade::SIMPLE;
        blade.brightness = firstBrightness;
        
        const auto parseLED{[&data, &logger](SimpleBlade::Star& star) -> optional<string> {
            auto idx{0};
            for (; idx < SimpleBlade::Star::LED_MAX; ++idx) {
                if (data.starts_with(SimpleBlade::Star::LED_STRS[idx])) {
                    star.led = idx;
                    data.erase(0, SimpleBlade::Star::LED_STRS[idx].length());
                    if (
                            idx >= SimpleBlade::Star::USE_RESISTANCE_START and
                            idx <= SimpleBlade::Star::USE_RESISTANCE_END
                       ) {
                        size_t numProcessed{};
                        try {
                            star.resistance = std::stoi(data.c_str(), &numProcessed);
                        } catch (std::exception e) {
                            return errorMessage(logger, wxTRANSLATE("Failed to read led resistance: %s"), e.what());
                        }
                        if (numProcessed == data.length()) {
                            return errorMessage(logger, wxTRANSLATE("Missing data after blade led resistance"));
                        }
                        data.erase(0, numProcessed + 1); // led>
                    }
                    if (data.empty()) {
                        return errorMessage(logger, wxTRANSLATE("Missing data after blade led"));
                    }
                    data.erase(0, 1); // ','
                    break;
                }
            }
            if (idx == SimpleBlade::Star::LED_MAX) {
                return errorMessage(logger, wxTRANSLATE("Unknown/malformed LED in SimpleBlade"));
            }

            return nullopt;
        }};

        res = parseLED(blade.simple().star1);
        if (res) return res;
        res = parseLED(blade.simple().star2);
        if (res) return res;
        res = parseLED(blade.simple().star3);
        if (res) return res;
        res = parseLED(blade.simple().star4);
        if (res) return res;

        const auto parsePin{[&data](SimpleBlade::Star& star) {
            if (data.length() >= 2 and data.starts_with("-1")) {
                data.erase(0, 2);
            } else {
                auto idx{0};
                for (; idx < data.length(); ++idx) {
                    if (data[idx] == ',' or data[idx] == '>') break;
                    star.powerPin += data[idx];
                }
                data.erase(0, idx);
            }
            if (data.empty()) return;
            data.erase(0, 1); // ',' or '>'
        }};
        parsePin(blade.simple().star1);
        parsePin(blade.simple().star2);
        parsePin(blade.simple().star3);
        parsePin(blade.simple().star4);
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

optional<string> Config::parseStyles(std::ifstream& file, Config& config, Log::Branch& lBranch) {
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
                Utils::trimWhitespace(bladestyle);

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

void Config::tryAddInjection(const string& buffer, Config& config) {
    auto& logger{Log::Context::getGlobal().createLogger("Config::tryAddInjection()")};

    auto strStart{buffer.find('"')};
    if (strStart == string::npos) return;
    auto strEnd{buffer.find('"', strStart + 1)};
    if (strEnd == string::npos) return;

    auto injectionPos{buffer.find(INJECTION_STR.data(), strStart + 1)};
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
    auto filePath{Paths::injections() / injectionFile};
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

            auto copyPath{Paths::injections() / filePath};
            fs::create_directories(copyPath.parent_path());
            const auto copyOptions{fs::copy_options::overwrite_existing};
            if (not fs::copy_file(fileDialog.GetPath().ToStdString(), copyPath, copyOptions, err)) {
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

void Config::Settings::processCustomDefines(Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Config::Settings::processCustomDefines()", lBranch)};
    for (auto idx{0}; idx < mCustomOptions.size(); ++idx) {
        auto& opt{customOption(idx)};

        bool processed{true};

        if (opt.define == NUM_BUTTONS_STR) {
            try {
                numButtons = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Couldn't parse num buttons: "} + e.what());
            }
        } else if (opt.define == NUM_BLADES_STR) {
        } else if (opt.define == ENABLE_AUDIO_STR) {
        } else if (opt.define == ENABLE_MOTION_STR) {
        } else if (opt.define == ENABLE_WS2811_STR) {
        } else if (opt.define == ENABLE_SD_STR) {
        } else if (opt.define == SHARED_POWER_PINS_STR) {
        } else if (opt.define == KEEP_SAVEFILES_STR) {
        } else if (opt.define == RFID_SERIAL_STR) {
            // Not Yet Implemented
        } else if (opt.define == BLADE_DETECT_PIN_STR) {
            bladeDetect = true;
            bladeDetectPin = static_cast<string>(opt.value);
        } else if (opt.define == BLADE_ID_CLASS_STR) {
            bladeID.enable = true;
            auto idx{0};
            for (; idx < BladeID::MODE_MAX; ++idx) {
                if (opt.value.startsWith(BladeID::MODE_STRS[idx])) break;
            }
            if (idx == BladeID::MODE_MAX) {
                logger.warn("Cannot parse invalid/unrecognized BladeID class");
            } else{
                bladeID.mode = idx;
                opt.value.erase(0, BladeID::MODE_STRS[idx].length());
                const auto idPinEnd{opt.value.find(',')};
                bladeID.pin = opt.value.substr(0, idPinEnd);
                if (idx == BladeID::EXTERNAL) {
                    if (idPinEnd == string::npos) {
                        logger.warn("Missing pullup value for external blade id");
                    } else {
                        opt.value.erase(0, idPinEnd + 1);
                        try {
                            bladeID.pullup = std::stoi(opt.value);
                        } catch (std::exception e) {
                            logger.warn(string{"Failed to parse pullup value for ext blade id: "} + e.what());
                        }
                    }
                } else if (idx == BladeID::BRIDGED) {
                    if (idPinEnd == string::npos) {
                        logger.warn("Missing bridge pin for blade id");
                    } else {
                        opt.value.erase(0, idPinEnd + 1);
                        bladeID.bridgePin = static_cast<string>(opt.value);
                    }
                }
            }
        } else if (opt.define == ENABLE_POWER_FOR_ID_STR) {
            bladeID.powerForID = true;
            if (not opt.value.startsWith(POWER_PINS_STR)) {
                logger.warn("Failed to parse BladeID PowerPINS");
            } else {
                opt.value.erase(0, POWER_PINS_STR.length());
                while (not false) {
                    const auto endPos{opt.value.find(',')};
                    // Use the entry for processing
                    bladeID.powerPinEntry = opt.value.substr(0, endPos);
                    bladeID.powerPins.select(bladeID.powerPinEntry);
                    if (endPos == string::npos) break;
                    opt.value.erase(0, endPos + 1);
                }
            }
        } else if (opt.define == BLADE_ID_SCAN_MILLIS_STR) {
            bladeID.continuousScanning = true;            
            try {
                bladeID.continuousInterval = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse blade id scan interval: "} + e.what());
            }
        } else if (opt.define == BLADE_ID_TIMES_STR) {
            bladeID.continuousScanning = true;            
            try {
                bladeID.continuousTimes = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse blade id scan times: "} + e.what());
            }
        } else if (opt.define == VOLUME_STR) {
            try {
                volume = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse volume: "} + e.what());
            }
        } else if (opt.define == BOOT_VOLUME_STR) {
            enableBootVolume = true;
            try {
                bootVolume = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse boot volume: "} + e.what());
            }
        } else if (opt.define == CLASH_THRESHOLD_STR) {
            try {
                clashThreshold = std::stod(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse clash threshold: "} + e.what());
            }
        } else if (opt.define == PLI_OFF_STR) {
            pliOffTime = Utils::doStringMath(opt.value) / 1000;
        } else if (opt.define == IDLE_OFF_STR) {
            idleOffTime = Utils::doStringMath(opt.value) / (60 * 1000);
        } else if (opt.define == MOTION_TIMEOUT_STR) {
            motionTimeout = Utils::doStringMath(opt.value) / (60 * 1000);
        } else if (opt.define == DISABLE_COLOR_CHANGE_STR) {
            disableColorChange = true;
        } else if (opt.define == DISABLE_BASIC_PARSERS_STR) {
            disableBasicParserStyles = true;
        } else if (opt.define == DISABLE_DIAG_COMMANDS_STR) {
            enableAllEditOptions = true;
        } else if (opt.define == ENABLE_DEV_COMMANDS_STR) {
            enableDeveloperCommands = true;
        } else if (opt.define == SAVE_STATE_STR) {
            saveState = true;
        } else if (opt.define == ENABLE_ALL_EDIT_OPTIONS_STR) {
            enableAllEditOptions = true;
        } else if (opt.define == SAVE_COLOR_STR) {
            saveColorChange = true;
        } else if (opt.define == SAVE_VOLUME_STR) {
            saveVolume = true;
        } else if (opt.define == SAVE_PRESET_STR) {
            savePreset = true;
        } else if (opt.define == ENABLE_OLED_STR) {
            enableOLED = true;
        } else if (opt.define == ORIENTATION_STR) {
            auto idx{0};
            for (; idx < ORIENTATION_MAX; ++idx) {
                if (opt.value == ORIENTATION_STRS[idx]) break;
            }
            if (idx == ORIENTATION_MAX) {
                logger.warn("Unknown/invalid orientation: " + static_cast<string>(opt.value));
            } else {
                orientation = idx;
            }
        } else if (opt.define == ORIENTATION_ROTATION_STR) {
            const auto firstComma{opt.value.find(',')};
            const auto secondComma{opt.value.find(',', firstComma + 1)};
            if (firstComma == string::npos or secondComma == string::npos) {
                logger.warn("Invalid formatting for orientation rotation: " + static_cast<string>(opt.value));
            } else {
                try {
                    orientationRotation.x = std::stod(opt.value);
                    orientationRotation.y = std::stod(opt.value.substr(firstComma + 1));
                    orientationRotation.z = std::stod(opt.value.substr(secondComma + 1));
                } catch (std::exception e) {
                    logger.warn(string{"Failed to parse orientation rotation: "} + e.what());
                }
            }
        } else if (opt.define == SPEAK_TOUCH_VALUES_STR) {
            speakTouchValues = true;
        } else if (opt.define == DYNAMIC_BLADE_DIMMING_STR) {
            dynamicBladeDimming = true;
        } else if (opt.define == DYNAMIC_BLADE_LENGTH_STR) {
            dynamicBladeLength = true;
        } else if (opt.define == DYNAMIC_CLASH_THRESHOLD_STR) {
            dynamicClashThreshold = true;
        } else if (opt.define == SAVE_BLADE_DIM_STR) {
            saveBladeDimming = true;
        } else if (opt.define == SAVE_CLASH_THRESHOLD_STR) {
            saveClashThreshold = true;
        } else if (opt.define == FILTER_CUTOFF_STR) {
            try {
                filterCutoff = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse filter cutoff: "} + e.what());
            }
        } else if (opt.define == FILTER_ORDER_STR) {
            try {
                filterOrder = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse filter order: "} + e.what());
            }
        } else if (opt.define == AUDIO_CLASH_SUPPRESSION_STR) {
            try {
                audioClashSuppressionLevel = std::stoi(opt.value);
            } catch (std::exception e) {
                logger.warn(string{"Failed to parse audio clash suppression: "} + e.what());
            }
        } else if (opt.define == DONT_USE_GYRO_FOR_CLASH_STR) {
            dontUseGyroForClash = true;
        } else if (opt.define == NO_REPEAT_RANDOM_STR) {
            noRepeatRandom = true;
        } else if (opt.define == FEMALE_TALKIE_STR) {
            femaleTalkie = true;
        } else if (opt.define == DISABLE_TALKIE_STR) {
            disableTalkie = true;
        } else if (opt.define == KILL_OLD_PLAYERS_STR) {
            killOldPlayers = true;
        } else {
            processed = false;
        }

        if (processed) {
            removeCustomOption(idx);
            --idx;
        }
    }
}

