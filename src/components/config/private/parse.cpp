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

    while (not file.eof() and file.good()) {
        if (not Utils::extractComment(file).empty()) continue;
        char chr = file.get();
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
    config.processCustomDefines();

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
        } else ret += file.get();
    }
    return ret;
}

void Config::parseTop(std::ifstream& file, Config& config) {
    const auto top{extractSection(file)};
    std::istringstream topStream{top};

    string buffer;
    while (topStream.good()) {
        if (not Utils::extractComment(topStream).empty()) continue;
        std::getline(topStream, buffer);
        enum {
            NONE,
            DEFINE,
            PC_OPT,
        } type;
        if (buffer.starts_with(DEFINE_STR)) {
            type = DEFINE;
            buffer = buffer.substr(DEFINE_STR.length());
        } else if (buffer.starts_with(PC_OPT_STR)) {
            type = PC_OPT;
            buffer = buffer.substr(PC_OPT_STR.length());
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

        if (type == NONE) continue;

        Utils::trimSurroundingWhitespace(buffer);
        const auto keyEnd{buffer.find_first_of(" \t")};
        auto key{buffer.substr(0, keyEnd)};
        auto value{buffer.substr(keyEnd)};
        Utils::trimSurroundingWhitespace(value);

        if (type == DEFINE) config.settings.addCustomOption(std::move(key), std::move(value));
        else if (type == PC_OPT) {
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
        if (not Utils::extractComment(propStream).empty()) continue;
        std::getline(propStream, buffer);
        Utils::trimSurroundingWhitespace(buffer);
        if (not buffer.starts_with(INCLUDE_STR)) continue;

        buffer = buffer.substr(INCLUDE_STR.length());
        Utils::trimSurroundingWhitespace(buffer);

        constexpr string_view PROP_DIR_STR{"../props/"};
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
    string comments;
    const auto finishStyleReading{[&array, &comments, &depth]() {
        auto& style{array.presets().back()->styles().back()};
        Utils::trimWhiteSpace(style->style);
        style->comment = std::move(comments);
        comments.clear();
    }};

    while (dataStream.good()) {
        const auto newComments{Utils::extractComment(dataStream)};
        if (not newComments.empty()) {
            if (not comments.empty()) comments += '\n';
            comments += newComments;
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
                auto& style{array.presets().back()->addStyle()};
                // Comment is assigned later. Doesn't need removal now.
                style.style.clear(); // Remove default style
                reading = STYLE;
            }
        } else if (reading == STYLE) {
            if (depth.empty()) {
                if (chr == '"') {
                    // This is the name entry, silly!
                    if (not array.presets().back()->styles().empty()) {
                        array.presets().back()->popBackStyle();
                    }
                    reading = NAME;
                    continue;
                }
                if (chr == ',') {
                    finishStyleReading();
                    array.presets().back()->addStyle();
                    continue;
                }
            }
            if (chr == '}') {
                finishStyleReading();
                if (not depth.empty()) logger.warn("Hit preset end before finishing style. This will mean errors to correct later!");
                depth.clear();
                reading = NONE;
                continue;
            }

            if (chr == '<' or chr == '(') depth.push_back(chr);
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

                depth.pop_back();
            }

            array.presets().back()->styles().back()->style += chr;
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
                Utils::trimWhiteSpace(buffer);
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
                    Utils::trimWhiteSpace(buffer);
                    auto& array{*config.bladeArrays.arrays().back()};
                    auto& blade{array.addBlade()};
                    auto res{parseBlade(
                        buffer,
                        array,
                        blade,
                        *logger.binfo("Parsing blade...")
                    )};
                    if (blade.type == Blade::TYPE_MAX) {
                        logger.debug("Removing blade parser deemed unnecessary.");
                        config.bladeArrays.removeArray(config.bladeArrays.arrays().size() - 1);
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
                Utils::trimWhiteSpace(buffer);
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

    constexpr string_view SUBBLADE_STR{"SubBlade"};
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

        constexpr string_view REVERSE_STR{"Reverse("};
        constexpr string_view STRIDE_STR{"WithStride("};
        constexpr string_view ZIGZAG_STR{"ZZ("};
        constexpr string_view LIST_STR{"WithList<"};
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

    const auto addSplit{[&blade, &splitData, &firstBrightness]() {
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

    constexpr string_view WS281X_STR{"WS281XBladePtr<"};
    constexpr string_view WS2811_STR{"WS2811BladePtr<"};
    constexpr string_view SIMPLE_STR{"SimpleBladePtr<"};
    constexpr string_view NULL_STR{"NULL"};
    constexpr string_view NULLPTR_STR{"nullptr"};
    if (data.starts_with(WS281X_STR)) {
        data.erase(0, WS281X_STR.length());
        blade.type = Blade::WS281X;

        size_t idx;
        try {
            blade.ws281x().length = std::stoi(data.c_str(), &idx);
        } catch (std::exception e) {
            return errorMessage(logger, wxTRANSLATE("Failed to read ws281x length: %s"), e.what());
        }
        if (idx + 1 >= data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after ws281x length"));
        }
        data.erase(0, idx + 1); // length,

        idx = 0;
        for (; idx < data.length(); ++idx) {
            if (data[idx] == ',') break;
            blade.ws281x().dataPin += data[idx];
        }
        if (idx == data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after ws281x data pin"));
        }
        data.erase(0, idx + 1);

        constexpr string_view COLOR8_STR{"Color8::"};
        if (not data.starts_with(COLOR8_STR)) {
            return errorMessage(logger, wxTRANSLATE("Malformatted ws281x color order"));
        }
        data.erase(0, COLOR8_STR.length());
        const auto colorOrderEnd{data.find(',')};
        if (colorOrderEnd == string::npos) {
            return errorMessage(logger, wxTRANSLATE("Missing data after ws281x color order"));
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

        if (not data.starts_with(POWER_PINS_STR)) {
            return errorMessage(logger, wxTRANSLATE("Missing ws281x PowerPINS"));
        }
        data.erase(0, POWER_PINS_STR.length());
        string buffer;
        for (auto idx{0}; idx < data.length(); ++idx) {
            if (data[idx] == ',' or data[idx] == '>') {
                blade.ws281x().addPowerPin(std::move(buffer));
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
            return errorMessage(logger, wxTRANSLATE("Failed to read ws2811 length: %s"), e.what());
        }
        if (idx + 1 >= data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after ws2811 length"));
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
            return errorMessage(logger, wxTRANSLATE("Missing data after ws2811 config"));
        }

        idx = 0;
        for (; idx < data.length(); ++idx) {
            if (data[idx] == ',') break;
            blade.ws281x().dataPin += data[idx];
        }
        if (idx == data.length()) {
            return errorMessage(logger, wxTRANSLATE("Missing data after ws2811 data pin"));
        }
        data.erase(0, idx + 1);

        if (not data.starts_with(POWER_PINS_STR)) {
            return errorMessage(logger, wxTRANSLATE("Missing ws2811 PowerPINS"));
        }
        data.erase(0, POWER_PINS_STR.length());
        buffer.clear();
        for (auto idx{0}; idx < data.length(); ++idx) {
            if (data[idx] == ',' or data[idx] == '>') {
                blade.ws281x().addPowerPin(std::move(buffer));
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
        blade.type = Blade::TYPE_MAX;
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
        if (lastSplit.type != splitData.type) addSplit();
        else { // this split is same type as last split
            if (
                    lastSplit.type == Split::STANDARD or
                    lastSplit.type == Split::REVERSE or
                    lastSplit.type == Split::LIST
               ) {
                // These types aren't segmented, just add.
                addSplit();
            } else if (lastSplit.type == Split::STRIDE) {
                if (
                        // Just make sure this split is same segments
                        // and the start falls inside last split
                        lastSplit.segments != splitData.segments or
                        lastSplit.start > splitData.start or
                        lastSplit.end < splitData.start
                   ) {
                    addSplit();
                }
                // Last split is same as this. Nothing to do.
            } else if (lastSplit.type == Split::ZIG_ZAG) {
                if (
                        lastSplit.segments != splitData.segments or
                        lastSplit.start != splitData.start or
                        lastSplit.end != splitData.end
                   ) {
                    addSplit();
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
            addSplit();
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
        if (not newComments.empty()) {
            if (not comments.empty()) comments += '\n';
            comments += newComments;
            continue;
        }

        const auto chr{file.get()};
        if (chr == '\r') continue;

        if (reading == NONE) {
            readHistory += static_cast<char>(chr);
            const auto usingPos{readHistory.rfind("using")};
            const auto equalPos{readHistory.rfind('=')};

            if (
                    equalPos != string::npos and
                    usingPos != string::npos and
                    usingPos < equalPos
               ) {
                reading = STYLE;
                readHistory.clear();
            } else if (usingPos != string::npos) {
                reading = STYLE_NAME;
            }
        } else if (reading == STYLE) {
            if (chr == ';') {
                Utils::trimWhiteSpace(bladestyle);

                // How many nested for loops would you like? Cause here are all of 'em
                for (const auto& presetArray : config.presetArrays.arrays()) {
                    for (const auto& preset : presetArray->presets()) {
                        for (const auto& style : preset->styles()) {
                            auto styleString{static_cast<string>(style->style)};
                            auto commentString{static_cast<string>(style->comment)};
                            for (;;) { // Just because I can lol, another for loop
                                const auto usingStylePos{styleString.find(name)};
                                if (usingStylePos == string::npos) break;

                                styleString.erase(usingStylePos, name.length());
                                styleString.insert(usingStylePos, bladestyle);
                                if (not commentString.empty()) commentString += '\n';
                                commentString += comments;
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
        } else if (reading == STYLE_NAME) {
            if (chr == ' ') {
                if (not name.empty()) {
                    reading = NONE;
                }
                continue;
            }

            name += static_cast<char>(chr);
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


