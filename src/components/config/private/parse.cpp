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
#include <system_error>

#include <wx/filedlg.h>

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
    optional<string> parseBlade(const string& data, Blade&, Log::Branch&);
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
                const auto id{buffer == "NO_BLADE" ? NO_BLADE : std::stoi(buffer)};
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
                    auto res{parseBlade(
                        buffer,
                        config.bladeArrays.arrays().back()->addBlade(),
                        *logger.binfo("Parsing blade...")
                    )};
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

optional<string> Config::parseBlade(const string& data, Blade& blade, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Config::parseBlade()")};

                bool subBlade{false};
                if (buffer.find("SubBlade") == 0) {
                    if (buffer.find("NULL") != string::npos or buffer.find("nullptr") != string::npos) { // Top Level SubBlade
                        bladeArray.blades.emplace_back();
                        bladeArray.blades.back().isSubBlade = true;
                        if (buffer.find("WithStride") != string::npos) bladeArray.blades.back().useStride = true;
                        if (buffer.find("ZZ") != string::npos) bladeArray.blades.back().useZigZag = true;
                    }

                    auto& blade{bladeArray.blades.back()};
                    buffer = buffer.substr(buffer.find('(') + 1);

                    auto paramEnd{buffer.find(',')};
                    const auto num1{std::stoi(buffer.substr(0, paramEnd))};
                    buffer = buffer.substr(paramEnd + 1);

                    paramEnd = buffer.find(',');
                    const auto num2{std::stoi(buffer.substr(0, paramEnd))};
                    buffer = buffer.substr(paramEnd + 1);

                    blade.subBlades.emplace_back(num1, num2);
                    subBlade = true;
                }

                constexpr string_view WS281X_STR{"WS281XBladePtr"};
                constexpr string_view SIMPLE_STR{"SimpleBladePtr"};
                if (buffer.find(WS281X_STR.data()) != string::npos) {
                    if (not subBlade) bladeArray.blades.emplace_back();
                    auto& blade{bladeArray.blades.back()};

                    buffer = buffer.substr(buffer.find(WS281X_STR.data()) + WS281X_STR.length());
                    buffer = buffer.substr(buffer.find('<') + 1);

                    auto paramEnd{buffer.find(',')};
                    blade.numPixels = std::stoi(buffer.substr(0, paramEnd));
                    buffer = buffer.substr(paramEnd + 1);

                    paramEnd = buffer.find(',');
                    auto dataPinStr{buffer.substr(0, paramEnd)};
                    trimWhiteSpace(dataPinStr);
                    blade.dataPin = dataPinStr;
                    buffer = buffer.substr(paramEnd + 1);

                    constexpr string_view COLOR8{"Color8"};
                    constexpr string_view NAMESPACE_SEPARATOR{"::"};
                    buffer = buffer.substr(buffer.find(COLOR8.data()) + COLOR8.length());
                    buffer = buffer.substr(buffer.find(NAMESPACE_SEPARATOR.data()) + NAMESPACE_SEPARATOR.length());

                    paramEnd = buffer.find(',');
                    auto colorStr{buffer.substr(0, paramEnd)};
                    trimWhiteSpace(colorStr);

                    blade.useRGBWithWhite = colorStr.find('W') != string::npos;
                    blade.type = (blade.useRGBWithWhite or colorStr.find('w') != string::npos) ? BD_PIXELRGBW : BD_PIXELRGB;
                    blade.colorType.assign(colorStr);

                    constexpr string_view POWER_PINS{"PowerPINS"};
                    buffer = buffer.substr(buffer.find(POWER_PINS.data()) + POWER_PINS.length());
                    buffer = buffer.substr(buffer.find('<') + 1);

                    while (!false) {
                        paramEnd = buffer.find(',');
                        bool done{paramEnd == string::npos};
                        if (paramEnd == string::npos) paramEnd = buffer.find('>');

                        auto pinStr{buffer.substr(0, paramEnd)};
                        trimWhiteSpace(pinStr);
                        if (not pinStr.empty()) blade.powerPins.emplace_back(pinStr);

                        if (done) break;

                        buffer = buffer.substr(paramEnd + 1);
                    }
                } else if (buffer.find(SIMPLE_STR.data()) != string::npos) {
                    bladeArray.blades.emplace_back();

                    auto& blade{bladeArray.blades.back()};
                    blade.type = BD_SIMPLE;

                    auto setupStar{[&](BladesPage::LED& led, int32_t& resistance) {
                        const auto paramEnd{buffer.find(',')};
                        auto paramStr{buffer.substr(0, paramEnd)};

                        const auto ledEnd{buffer.find('<')};
                        auto ledStr{paramStr.substr(0, ledEnd)};
                        trimWhiteSpace(ledStr);

                        led = BladesPage::strToLed(ledStr);;
                        if (led & BladesPage::USES_RESISTANCE) resistance = std::stoi(paramStr.substr(ledEnd + 1));

                        buffer = buffer.substr(paramEnd + 1);
                    }};

                    buffer = buffer.substr(buffer.find(SIMPLE_STR.data()) + SIMPLE_STR.length());
                    buffer = buffer.substr(buffer.find('<') + 1);

                    setupStar(blade.star1, blade.star1Resistance);
                    setupStar(blade.star2, blade.star2Resistance);
                    setupStar(blade.star3, blade.star3Resistance);
                    setupStar(blade.star4, blade.star4Resistance);

                    while (!false) {
                        auto paramEnd{buffer.find(',')};
                        bool done{paramEnd == string::npos};
                        if (paramEnd == string::npos) paramEnd = buffer.find('>');

                        auto pinStr{buffer.substr(0, paramEnd)};
                        trimWhiteSpace(pinStr);
                        if (not pinStr.empty() and pinStr != "-1") blade.powerPins.emplace_back(pinStr);

                        if (done) break;

                        buffer = buffer.substr(paramEnd + 1);
                    }
                }

                ++bladesRead;

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


