#include "configuration.h"
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
#include <wx/event.h>
#include <wx/msgdlg.h>

#include "log/branch.h"
#include "log/context.h"
#include "ui/message.h"
#include "paths/paths.h"

#include "../../core/config/settings.h"
#include "../../core/config/propfile.h"
#include "../../core/utilities/misc.h"
#include "../../editor/editorwindow.h"
#include "../../editor/pages/generalpage.h"
#include "../../editor/pages/presetspage.h"
#include "../../editor/pages/propspage.h"
#include "../../editor/pages/bladespage.h"
#include "../../editor/dialogs/bladearraydlg.h"
#include "../../tools/arduino.h"

namespace Config {

    void tryAddInjection(const string& buffer, EditorWindow *);

    void readConfigTop(std::ifstream&, EditorWindow*);
    void readConfigProp(std::ifstream&, EditorWindow*);
    void readConfigPresets(std::ifstream&, EditorWindow*);
    void readConfigStyles(std::ifstream&, EditorWindow*);
    void readPresetArray(std::ifstream&, EditorWindow*);
    void readBladeArray(std::ifstream&, EditorWindow*);
    void setCustomDefines(EditorWindow* editor);
} // namespace Config


bool Config::readConfig(const filepath& filePath, EditorWindow* editor) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    try {
        string buffer;
        while (!file.eof()) {
            file >> buffer;
            if (buffer == "//") {
                getline(file, buffer);
                continue;
            }
            if (wxStrstr(buffer, "/*")) {
                while (!file.eof()) {
                    if (wxStrstr(buffer, "*/")) break;
                    file >> buffer;
                }
                continue;
            }
            if (buffer == "#ifdef") {
                file >> buffer;
                if (buffer == "CONFIG_TOP") Config::readConfigTop(file, editor);
                if (buffer == "CONFIG_PROP") Config::readConfigProp(file, editor);
                if (buffer == "CONFIG_PRESETS") Config::readConfigPresets(file, editor);
                if (buffer == "CONFIG_STYLES") Config::readConfigStyles(file, editor);
            }
            if (buffer == "#include") {
                getline(file, buffer);
                tryAddInjection(buffer, editor);
            }
        }
        // Wait to call remaining defines "custom" until prop file stuffage has been read
        setCustomDefines(editor);

    } catch (std::exception& e) {
        wxString errorMessage = "There was an error parsing config, please ensure it is valid:\n\n";
        errorMessage += e.what();

        std::cerr << errorMessage << std::endl;
        return false;
    }

    //GeneralPage::update();
    editor->propsPage->update();
    editor->bladesPage->update();
    editor->bladesPage->bladeArrayDlg->update();
    editor->presetsPage->update();

    return true;
}

void Config::tryAddInjection(const string& buffer, EditorWindow *editor) {
    auto& logger{Log::Context::getGlobal().createLogger("Config::tryAddInjection()")};

    auto strStart{buffer.find('"')};
    if (strStart == string::npos) return;
    auto strEnd{buffer.find('"', strStart + 1)};
    if (strEnd == string::npos) return;

    auto injectionPos{buffer.find(INJECTION_STR.data(), strStart + 1)};
    string injectionFile;
    if (injectionPos != string::npos) {
        logger.verbose("Injection wxString found...");
        injectionFile = buffer.substr(injectionPos + INJECTION_STR.length() + 1, strEnd - injectionPos - INJECTION_STR.length() - 1);
    } else {
        logger.verbose("Injection wxString missing...");
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

    editor->presetsPage->injections.emplace_back(injectionFile);
    logger.debug("Done");
}

bool Config::importConfig(EditorWindow* editor) {
    wxFileDialog configLocation(editor, "Choose ProffieOS Config File", "", "", "C Header Files (*.h)|*.h", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (configLocation.ShowModal() == wxID_CANCEL) return false; // User Closed

    return Config::readConfig(configLocation.GetPath().ToStdString(), editor);
}

void Config::readConfigTop(std::ifstream& file, EditorWindow* editor) {
    string element;
    editor->settings->readDefines.clear();
    while (!file.eof() && element != "#endif") {
        file >> element;
        if (element == "//") {
            getline(file, element);
            continue;
        }
        if (wxStrstr(element, "/*")) {
            while (!file.eof()) {
                if (wxStrstr(element, "*/")) break;
                file >> element;
            }
            continue;
        }
        if (element == "#define" && !file.eof()) {
            getline(file, element);
            editor->settings->readDefines.emplace_back(element);
        } else if (element == "const" && !file.eof()) {
            getline(file, element);

            auto equalPos{element.find('=')};
            if (equalPos == string::npos) continue;
            editor->generalPage->maxLEDs->entry()->SetValue(std::stoi(element.substr(equalPos + 1)));
        } else if (element == "#include" && !file.eof()) {
            file >> element;
            if (element.find("v1") != string::npos) {
                editor->generalPage->board->entry()->SetStringSelection(PROFFIEBOARD[Arduino::PROFFIEBOARDV1].first);
            } else if (element.find("v2") != string::npos) {
                editor->generalPage->board->entry()->SetStringSelection(PROFFIEBOARD[Arduino::PROFFIEBOARDV2].first);
            } else if (element.find("v3") != string::npos) {
                editor->generalPage->board->entry()->SetStringSelection(PROFFIEBOARD[Arduino::PROFFIEBOARDV3].first);
            }
        } else if (element == "//PROFFIECONFIG") {
            file >> element;
            if (element == "ENABLE_MASS_STORAGE") editor->generalPage->massStorage->SetValue(true);
            if (element == "ENABLE_WEBUSB") editor->generalPage->webUSB->SetValue(true);
        }
    }
    editor->settings->parseDefines(editor->settings->readDefines);
}

void Config::setCustomDefines(EditorWindow* editor) {
    for (const auto& define : editor->settings->readDefines) {
        auto key = Settings::ProffieDefine::parseKey(define);
        if (!key.first.empty()) editor->generalPage->customOptDlg->addDefine(key.first, key.second);
    }
}

void Config::readConfigProp(std::ifstream& file, EditorWindow* editor) {
    string element;
    while (!file.eof() && element != "#endif") {
        file >> element;
        for (const auto& prop : editor->propsPage->getLoadedProps()) {
            auto *propSettings = prop->getSettings();
            if (element.find(prop->getFileName()) != string::npos) {
                editor->propsPage->updateSelectedProp(prop->getName());
                for (auto define = editor->settings->readDefines.begin(); define < editor->settings->readDefines.end();) {
                    std::istringstream defineStream(define->ToStdString());
                    string defineName{};
                    double value{0};

                    defineStream >> defineName;
                    auto key = propSettings->find(defineName);
                    if (key == propSettings->end()) {
                        define++;
                        continue;
                    }

                    if (
                            key->second.type == PropFile::Setting::SettingType::TOGGLE ||
                            key->second.type == PropFile::Setting::SettingType::OPTION
                       ) {
                        key->second.setValue(true);
                    } else {
                        defineStream >> value;
                        key->second.setValue(value);
                    }
                    define = editor->settings->readDefines.erase(define);
                }
                break;
            }
        }
    }
}

void Config::readConfigPresets(std::ifstream& file, EditorWindow* editor) {
    editor->bladesPage->bladeArrayDlg->bladeArrays.clear();
    string element;
    while (!file.eof() && element != "#endif") {
        file >> element;
        if (element.find("//") == 0) {
            getline(file, element);
            continue;
        }
        if (wxStrstr(element, "/*")) {
            while (!file.eof()) {
                if (wxStrstr(element, "*/")) break;
                file >> element;
            }
            continue;
        }
        if (element == "Preset") readPresetArray(file, editor);
        if (element == "BladeConfig") readBladeArray(file, editor);
        if (element == "#include") {
            getline(file, element);
            tryAddInjection(element, editor);
        }
    }
}

void Config::readConfigStyles(std::ifstream& file, EditorWindow* editor) {
    int32 chr{0};
    enum {
        NONE,
        STYLE,
        STYLE_NAME,
        LINE_COMMENT,
        LONG_COMMENT,
    } reading{NONE}, prevReading{NONE};

    string readHistory;
    string commentString;
    string styleString;
    string styleName;

    while (chr != '}' and not file.eof() and not file.bad()) {
        chr = file.get();

        if (chr == '\r') continue;
        if (
                        chr == '/' and
                        reading != LONG_COMMENT and
                        reading != LINE_COMMENT
                        ) {
            if (file.peek() == '*') {
                prevReading = reading;
                reading = LONG_COMMENT;
                file.get();
                continue;
            } 
            if (file.peek() == '/') {
                prevReading = reading;
                reading = LINE_COMMENT;
                file.get();
                continue;
            }
        }

        if (reading != LINE_COMMENT and reading != LONG_COMMENT) {
            if (chr == '#') {
                const auto initPos{file.tellg()};
                std::array<char, 6> checkArray;
                file.read(checkArray.data(), checkArray.size() - 1);
                checkArray.back() = 0;
                if (string{checkArray.data()} == "endif") {
                    return;
                }
                file.seekg(initPos);
            }
        }

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
        } else if (reading == LINE_COMMENT) {
            commentString += static_cast<char>(chr);
            if (chr == '\n')  {
                reading = prevReading;
            }
        } else if (reading == LONG_COMMENT) {
            // Deal with Fett's comments
            if (chr == '*' and file.peek() == '/') {
                commentString += '\n';
                reading = prevReading;
                file.get();
                continue;
            }
            commentString += static_cast<char>(chr);
        } else if (reading == STYLE) {
            if (chr == ';') {
                trimWhiteSpace(styleName);
                trimWhiteSpace(styleString);
                trimWhiteSpace(commentString);

                // How many nested for loops would you like? Cause here are all of 'em
                for (auto& bladeArray : editor->bladesPage->bladeArrayDlg->bladeArrays) {
                    for (auto& preset : bladeArray.presets) {
                        for (auto& [ comment, style ] : preset.styles) {
                            for (;;) { // Just because I can lol, another for loop
                                const auto usingStylePos{style.find(styleName)};
                                if (usingStylePos == string::npos) break;

                                style.erase(usingStylePos, styleName.length());
                                style.insert(usingStylePos, styleString);
                                if (comment.find(commentString) == string::npos) {
                                    comment += '\n';
                                    comment += commentString;
                                }
                            }
                        }
                    }
                }
                styleName.clear();
                styleString.clear();
                commentString.clear();
                reading = NONE;
                continue;
            }

            styleString += static_cast<char>(chr);
        } else if (reading == STYLE_NAME) {
            if (chr == ' ') {
                if (not styleName.empty()) {
                    reading = NONE;
                }
                continue;
            }

            styleName += static_cast<char>(chr);
        }
    }
}

void Config::readPresetArray(std::ifstream& file, EditorWindow* editor) {
    editor->bladesPage->bladeArrayDlg->bladeArrays.emplace_back();
    BladeArrayDlg::BladeArray& bladeArray = editor->bladesPage->bladeArrayDlg->bladeArrays.at(editor->bladesPage->bladeArrayDlg->bladeArrays.size() - 1);

    string element{};
    file >> element;
    bladeArray.name.assign(element.substr(0, element.find_first_of("[]")));

    int32 chr{};
    element.clear();
    do {
        chr = file.get();
        element += static_cast<char>(chr);
        if (
                file.eof() or
                file.bad() or
                element.rfind("#endif") != string::npos or
                element.rfind("};") != string::npos
           ) {
            return;
        }
    } while (chr != '{');

    auto presetIdx{static_cast<uint32_t>(-1)};
    bladeArray.presets.clear();
    while (!false) {
        bladeArray.presets.emplace_back();
        ++presetIdx;

        enum {
            NONE,
            LINE_COMMENT,
            LONG_COMMENT,
            LONG_COMMENT_NEW_LINE,
            POST_BRACE,
            DIR,
            POST_DIR,
            TRACK,

            NAME,
            PRUNE,
        } reading{NONE}, prevReading{NONE};
        while (not file.eof() and not file.bad()) {
            chr = file.get();

            if (chr == '\r') continue;
            if (reading != LONG_COMMENT and reading != LONG_COMMENT_NEW_LINE and reading != LINE_COMMENT) {
                if (chr == '/') {
                    if (file.peek() == '*') {
                        prevReading = reading;
                        reading = LONG_COMMENT;
                        file.get();
                        continue;
                    } 
                    if (file.peek() == '/') {
                        prevReading = reading;
                        reading = LINE_COMMENT;
                        file.get();
                        continue;
                    }
                }
                if (chr == '#') {
                    const auto initPos{file.tellg()};
                    std::array<char, 6> checkArray;
                    file.read(checkArray.data(), checkArray.size() - 1);
                    checkArray.back() = 0;
                    if (string{checkArray.data()} == "endif") {
                        return;
                    }
                    file.seekg(initPos);
                }
                if (chr == '}') {
                    if (file.peek() == ';') {
                        bladeArray.presets.pop_back();
                        return;
                    } 
                    break;
                }
            }

            if (reading == NONE) {
                if (chr == '{') {
                    reading = POST_BRACE;
                }
            } else if (reading == POST_BRACE or reading == POST_DIR) {
                if (chr == '"') {
                    if (reading == POST_BRACE) {
                        reading = DIR;
                    } else if (reading == POST_DIR) {
                        reading = TRACK;
                    }
                }
            } else if (reading == LINE_COMMENT) {
                if (chr == '\n')  {
                    reading = prevReading;
                }
            } else if (reading == LONG_COMMENT) {
                if (chr == '*' and file.peek() == '/') {
                    file.get();
                    reading = prevReading;
                }
            } else if (reading == DIR) {
                if (chr == '"') {
                    trimWhiteSpace(bladeArray.presets[presetIdx].dirs);
                    reading = POST_DIR;
                    continue;
                }
                bladeArray.presets[presetIdx].dirs += static_cast<char>(chr);
            } else if (reading == TRACK) {
                if (chr == '"') {
                    trimWhiteSpace(bladeArray.presets[presetIdx].track);
                    break;
                }
                bladeArray.presets[presetIdx].track += static_cast<char>(chr);
            }
        }


        // Read actual styles
        for (int32_t blade = 0; blade < editor->settings->numBlades; ++blade) {
            int32 styleDepth{0};
            int32 parenDepth{0};
            string styleString;
            string commentString;

            while (not file.eof() and not file.bad()) {
                chr = file.get();

                if (chr == '\r') continue;
                if (
                        chr == '/' and
                        reading != LONG_COMMENT and
                        reading != LONG_COMMENT_NEW_LINE and
                        reading != LINE_COMMENT
                   ) {
                    if (file.peek() == '*') {
                        prevReading = reading;
                        reading = LONG_COMMENT;
                        file.get();
                        continue;
                    } 
                    if (file.peek() == '/') {
                        prevReading = reading;
                        reading = LINE_COMMENT;
                        file.get();
                        continue;
                    }
                }
                if (chr == '#') {
                    const auto initPos{file.tellg()};
                    std::array<char, 6> checkArray;
                    file.read(checkArray.data(), checkArray.size() - 1);
                    checkArray.back() = 0;
                    if (string{checkArray.data()} == "endif") {
                        return;
                    }
                    file.seekg(initPos);
                }
                if (chr == '}') {
                    if (file.peek() == ';') {
                        bladeArray.presets.pop_back();
                        return;
                    }
                    break;
                }

                if (reading == NONE) {
                    if (chr == '<') ++styleDepth;
                    else if (chr == '>') --styleDepth;
                    else if (chr == '(') ++parenDepth;
                    else if (chr == ')') --parenDepth;
                    else if (chr == ',' and styleDepth == 0 and parenDepth == 0) {
                        // Reached end of style
                        break;
                    }

                    // Do actual reading
                    styleString += static_cast<char>(chr);
                } else if (reading == LINE_COMMENT) {
                    commentString += static_cast<char>(chr);
                    if (chr == '\n')  {
                        reading = prevReading;
                    }
                } else if (reading == LONG_COMMENT) {
                    // Deal with Fett's comments
                    if (chr == '*' and file.peek() == '/') {
                        commentString += '\n';
                        reading = prevReading;
                        continue;
                    } 

                    if (chr == '\n') {
                        reading = LONG_COMMENT_NEW_LINE;
                    }
                    commentString += static_cast<char>(chr);
                } else if (reading == LONG_COMMENT_NEW_LINE) {
                    if (chr == '*' and file.peek() == '/') {
                        commentString += '\n';
                        reading = prevReading;
                        file.get();
                        continue;
                    }

                    if (std::isspace(chr) or chr == '*') continue;
                    commentString += static_cast<char>(chr);
                    reading = LONG_COMMENT;
                } else if (reading == TRACK) {
                    // Purge comma after track
                    if (chr == ',') {
                        reading = NONE;
                    }
                }
            }

            // Trim whitespace
            trimWhiteSpace(styleString);
            trimWhiteSpace(commentString);

            bladeArray.presets[presetIdx].styles.push_back({commentString, styleString});
        }

        if (chr != '}') {
            reading = NONE;
            while (not file.eof() and not file.bad()) {
                chr = file.get();

                if (chr == '\r') continue;
                if (
                        chr == '/' and
                        reading != LONG_COMMENT and
                        reading != LONG_COMMENT_NEW_LINE and
                        reading != LINE_COMMENT
                        ) {
                    if (file.peek() == '*') {
                        prevReading = reading;
                        reading = LONG_COMMENT;
                        file.get();
                        continue;
                    } 

                    if (file.peek() == '/') {
                        prevReading = reading;
                        reading = LINE_COMMENT;
                        file.get();
                        continue;
                    }
                }
                if (chr == '#') {
                    const auto initPos{file.tellg()};
                    std::array<char, 6> checkArray;
                    file.read(checkArray.data(), checkArray.size() - 1);
                    if (string{checkArray.data()} == "endif") {
                        return;
                    }
                    file.seekg(initPos);
                }
                if (chr == '}') {
                    if (file.peek() == ';') {
                        bladeArray.presets.pop_back();
                        return;
                    }                          

                    break;
                }

                if (reading == NONE) {
                    if (chr == '"') {
                        reading = NAME;
                    }
                } else if (reading == LINE_COMMENT) {
                    if (chr == '\n')  {
                        reading = prevReading;
                    }
                } else if (reading == LONG_COMMENT) {
                    if (chr == '*' and file.peek() == '/') {
                        reading = prevReading;
                        file.get();
                    }
                } else if (reading == NAME) {
                    if (chr == '"') {
                        trimWhiteSpace(bladeArray.presets[presetIdx].name);
                        reading = PRUNE;
                        continue;
                    }
                    bladeArray.presets[presetIdx].name += static_cast<char>(chr);
                }
            }
        }

        trimWhiteSpace(bladeArray.presets[presetIdx].name);
        if (bladeArray.presets[presetIdx].name.empty()) {
            bladeArray.presets[presetIdx].name = "noname";
        }
    }
}

void Config::readBladeArray(std::ifstream& file, EditorWindow* editor) {
    enum {
        NONE,
        LINE_COMMENT,
        LONG_COMMENT,
        LONG_COMMENT_NEW_LINE,

        IN_ARRAY,
        ID_NUM,
        BLADE_ENTRY,
        CONFIG_ARRAY,
    } reading{NONE}, prevReading{NONE};

    int32 chr{};

    BladeArrayDlg::BladeArray bladeArray;

    string buffer;
    int32 bladesRead{0};

    std::vector<wchar_t> bladeSects{};

    while (not file.eof() and not file.bad()) {
        chr = file.get();

        if (chr == '\r') continue;
        if (reading != LONG_COMMENT and reading != LONG_COMMENT_NEW_LINE and reading != LINE_COMMENT) {
            if (chr == '/') {
                if (file.peek() == '*') {
                    prevReading = reading;
                    reading = LONG_COMMENT;
                    file.get();
                    continue;
                } 

                if (file.peek() == '/') {
                    prevReading = reading;
                    reading = LINE_COMMENT;
                    file.get();
                    continue;
                }
            }
            if (chr == '#') {
                const auto initPos{file.tellg()};
                std::array<char, 6> checkArray;
                file.read(checkArray.data(), checkArray.size() - 1);
                if (string{checkArray.data()} == "endif") {
                    return;
                }
                file.seekg(initPos);
            }
            if (chr == '}') {
                if (file.peek() == ';') {
                    return;
                } 

                reading = NONE;
                bladeArray = {};
                continue;
            }
        }

        if (reading == LINE_COMMENT) {
            if (chr == '\n')  {
                reading = prevReading;
            }
        } else if (reading == LONG_COMMENT) {
            if (chr == '*' and file.peek() == '/') {
                file.get();
                reading = prevReading;
            }
        } else if (reading == NONE) {
            if (chr == '{') {
                reading = IN_ARRAY;
                bladeArray = {};
            }
        } else if (reading == IN_ARRAY) {
            if (chr == '{') {
                reading = ID_NUM;
            }
        } else if (reading == ID_NUM) {
            if (chr == ',') {
                trimWhiteSpace(buffer);
                bladeArray.value = buffer == "NO_BLADE" ? 0 : std::stoi(buffer);
                buffer.clear();
                reading = BLADE_ENTRY;
                continue;
            }

            buffer += static_cast<char>(chr);
        } else if (reading == BLADE_ENTRY) {
            if (
                not bladeSects.empty() and
                ((bladeSects.back() == '<' and chr == '>') or
                (bladeSects.back() == '(' and chr == ')'))
            ) {
                bladeSects.pop_back();
            } else if (chr == '<' or chr == '(') {
                bladeSects.push_back(chr);
            } else if (chr == ',' and bladeSects.empty()) {
                trimWhiteSpace(buffer);

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
                buffer.clear();
                if (bladesRead == editor->settings->numBlades) {
                    reading = CONFIG_ARRAY;
                }
                continue;
            }

            buffer += static_cast<char>(chr);
        } else if (reading == CONFIG_ARRAY) {
            if (bladeSects.size() == 1) {
                if (chr == ')') bladeSects.pop_back();
                else buffer += static_cast<char>(chr);
            } else if (bladeSects.size() == 0) {
                if (chr == '(') bladeSects.push_back(chr);
            }

            if (not buffer.empty() and bladeSects.empty()) {
                trimWhiteSpace(buffer);
                bladeArray.name = buffer;

                if (bladeArray.blades.empty()) bladeArray.blades.emplace_back();

                for (BladeArrayDlg::BladeArray& array : editor->bladesPage->bladeArrayDlg->bladeArrays) {
                    if (array.name == bladeArray.name) {
                        array.value = bladeArray.value;
                        array.blades = bladeArray.blades;

                        if (array.value == 0 && array.name != "no_blade") {
                            array.name = "blade_in";
                        }
                    }
                }

                for (const auto& blade : bladeArray.blades) {
                    for (const auto& powerPin : blade.powerPins) {
                        if (editor->bladesPage->powerPins->FindString(powerPin) == wxNOT_FOUND) editor->bladesPage->powerPins->Append(powerPin);
                    }
                }

                reading = IN_ARRAY;
                buffer.clear();
            }
        }
    }
}

const Config::MapPair& Config::findInVMap(const Config::VMap& map, const wxString& search) {
    return *std::find_if(map.begin(), map.end(), [&](const MapPair& pair) { return (pair.second == search || pair.first == search); });
}

wxArrayString Config::orientationStrings() {
    wxArrayString ret;
    for (auto idx{0}; idx < ORIENTATION_MAX; ++idx) {
        ret.emplace_back(orientToStr(static_cast<Orientation>(idx)));
    }
    return ret;
}

wxString Config::orientToStr(Orientation orient) {
    switch (orient) {
        case ORIENTATION_FETS_TOWARDS_BLADE:
            return _("FETs Towards Blade");
        case ORIENTATION_USB_TOWARDS_BLADE:
            return _("USB Towards Blade");
        case ORIENTATION_USB_CCW_FROM_BLADE:
            return _("USB CCW From Blade");
        case ORIENTATION_USB_CW_FROM_BLADE:
            return _("USB CW From Blade");
        case ORIENTATION_TOP_TOWARDS_BLADE:
            return _("Top Towards Blade");
        case ORIENTATION_BOTTOM_TOWARDS_BLADE:
            return _("Bottom Towards Blade");
        default: 
            return {};
    }
}

