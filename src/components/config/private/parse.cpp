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

namespace Configuration {
    constexpr string_view INJECTION_STR{"injection"};

    template<typename ...ARGS>
    void errorMessage(Log::Logger& logger, EditorWindow *editor, const wxString& msg, ARGS... args) {
        logger.error(wxString::Format(msg, args...).ToStdString());
        auto* msgEvent{new Misc::MessageBoxEvent(
            wxID_ANY,
            wxString::Format(wxGetTranslation(msg), args...) + "\n\n" + _("Configuration not saved."),
            _("Configuration Error"),
            wxOK | wxCENTER | wxICON_ERROR)
        };
        wxQueueEvent(editor, msgEvent);
    }

    bool runPreChecks(EditorWindow *, Log::Branch&);

    void tryAddInjection(const string& buffer, EditorWindow *);

    void outputConfigTop(std::ofstream&, const EditorWindow *);
    void outputConfigTopGeneral(std::ofstream&, const EditorWindow *);
    void outputConfigTopCustom(std::ofstream&, const EditorWindow *);
    void outputConfigTopBladeAwareness(std::ofstream&, const EditorWindow *);
    void outputConfigTopPropSpecific(std::ofstream&, const EditorWindow *);
    void outputConfigTopSA22C(std::ofstream&, const EditorWindow *);
    void outputConfigTopFett263(std::ofstream&, const EditorWindow *);
    void outputConfigTopBC(std::ofstream&, const EditorWindow *);
    void outputConfigTopCaiwyn(std::ofstream&, const EditorWindow *);
    void outputConfigProp(std::ofstream&, const EditorWindow *);
    void outputConfigPresets(std::ofstream&, const EditorWindow *);
    void outputConfigPresetsStyles(std::ofstream&, const EditorWindow *);
    void outputConfigPresetsBlades(std::ofstream&, const EditorWindow *);
    void genWS281X(std::ofstream&, const BladesPage::BladeConfig&);
    void genSubBlades(std::ofstream&, const BladesPage::BladeConfig&);
    void outputConfigButtons(std::ofstream&, const EditorWindow *);

    void readConfigTop(std::ifstream&, EditorWindow*);
    void readConfigProp(std::ifstream&, EditorWindow*);
    void readConfigPresets(std::ifstream&, EditorWindow*);
    void readConfigStyles(std::ifstream&, EditorWindow*);
    void readPresetArray(std::ifstream&, EditorWindow*);
    void readBladeArray(std::ifstream&, EditorWindow*);
    void setCustomDefines(EditorWindow* editor);
} // namespace Configuration


bool Configuration::outputConfig(const filepath& filePath, EditorWindow *editor, Log::Branch *lBranch, bool fullOutput) {
    auto& logger{Log::Branch::optCreateLogger("Configuration::outputConfig()", lBranch)};

    if (not runPreChecks(editor, *logger.binfo("Running prechecks..."))) return false;

    if (fullOutput) {
        std::error_code err;
        const auto injectionDir{Paths::proffieos() / "config" / INJECTION_STR};
        for (const auto& injection : editor->presetsPage->injections) {
            auto injectionPath{injectionDir / filepath{injection}};
            fs::create_directories(injectionPath.parent_path());
            if (not fs::copy_file(
                Paths::injections() / filepath{injection},
                injectionPath,
                fs::copy_options::overwrite_existing,
                err
            )) {
                errorMessage(logger, editor, wxTRANSLATE("Failed setting up injection \"%s\""), injection);
                return false;
            }
        }
    }

    std::ofstream configOutput(filePath);
    if (not configOutput.is_open()) {
        errorMessage(logger, editor, wxTRANSLATE("Could not open config file for output."));
        return false;
    }

    configOutput << "/*\n";
    configOutput << "This configuration file was generated by ProffieConfig " wxSTRINGIZE(BIN_VERSION) ", created by Ryryog25.\n";
    configOutput << "The tool can be found here: https://proffieconfig.kafrenetrading.com/\n";
    configOutput << "ProffieConfig is an All-In-One utility for managing your Proffieboard.\n";
    configOutput << "*/\n\n";


    outputConfigTop(configOutput, editor);
    outputConfigProp(configOutput, editor);
    outputConfigPresets(configOutput, editor);
    outputConfigButtons(configOutput, editor);

    configOutput.close();
    logger.info("Done");
    return true;
}
bool Configuration::outputConfig(EditorWindow *editor) {
    return Configuration::outputConfig(Paths::configs() / (string{editor->getOpenConfig()} + ".h"), editor);
}

bool Configuration::exportConfig(EditorWindow *editor) {
    wxFileDialog configLocation(editor, "Save ProffieOS Config File", "", wxString{editor->getOpenConfig()}, "ProffieOS Configuration (*.h)|*.h", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (configLocation.ShowModal() == wxID_CANCEL) return false; // User Closed

    return Configuration::outputConfig(configLocation.GetPath().ToStdString(), editor);
}

void Configuration::outputConfigTop(std::ofstream& configOutput, const EditorWindow *editor) {
    configOutput << "#ifdef CONFIG_TOP" << std::endl;
    outputConfigTopGeneral(configOutput, editor);
    outputConfigTopPropSpecific(configOutput, editor);
    outputConfigTopCustom(configOutput, editor);
    configOutput << "#endif" << std::endl << std::endl;

}

void Configuration::outputConfigTopGeneral(std::ofstream& configOutput, const EditorWindow *editor) {
    if (editor->generalPage->massStorage->GetValue()) configOutput << "//PROFFIECONFIG ENABLE_MASS_STORAGE" << std::endl;
    if (editor->generalPage->webUSB->GetValue()) configOutput << "//PROFFIECONFIG ENABLE_WEBUSB" << std::endl;

    configOutput << findInVMap(PROFFIEBOARD, editor->generalPage->board->entry()->GetStringSelection()).second << std::endl;

    configOutput << "const unsigned int maxLedsPerStrip = " << editor->generalPage->maxLEDs->entry()->GetValue() << ";" << std::endl;
    configOutput << "#define ENABLE_AUDIO" << std::endl;
    configOutput << "#define ENABLE_WS2811" << std::endl;
    configOutput << "#define ENABLE_SD" << std::endl;
    configOutput << "#define ENABLE_MOTION" << std::endl;
    configOutput << "#define SHARED_POWER_PINS" << std::endl;

    for (const auto& [ name, define ] : editor->settings->generalDefines) {
        if (define->shouldOutput()) configOutput << "#define " << define->getOutput() << std::endl;
    }
}

void Configuration::outputConfigTopPropSpecific(std::ofstream& configOutput, const EditorWindow *editor) {
    auto *selectedProp{editor->propsPage->getSelectedProp()};
    if (selectedProp == nullptr) return;

    for (const auto& [ name, setting ] : *selectedProp->getSettings()) {
        if (
                        !setting.checkRequiredSatisfied(*selectedProp->getSettings()) ||
                        setting.disabled ||
                        !setting.shouldOutput
                        ) continue;

        auto output = setting.getOutput();
        if (!output.empty()) configOutput << "#define " << output << std::endl;
    }
}

void Configuration::outputConfigTopCustom(std::ofstream& configOutput, const EditorWindow *editor) {
    for (const auto& [ name, value ] : editor->generalPage->customOptDlg->getCustomDefines()) {
        if (!name.empty()) configOutput << "#define " << name << " " << value << std::endl;
    }
}

void Configuration::outputConfigProp(std::ofstream& configOutput, const EditorWindow *editor) {
    auto *selectedProp{editor->propsPage->getSelectedProp()};
    if (selectedProp == nullptr) return;

    configOutput << "#ifdef CONFIG_PROP" << std::endl;
    configOutput << "#include \"../props/" << selectedProp->getFileName() << "\"" << std::endl;
    configOutput << "#endif" << std:: endl << std::endl; // CONFIG_PROP
}

void Configuration::outputConfigPresets(std::ofstream& configOutput, const EditorWindow *editor) {
    configOutput << "#ifdef CONFIG_PRESETS\n" << std::flush;
    for (const auto& injection : editor->presetsPage->injections) {
        configOutput << "#include \"" << INJECTION_STR.data() << '/' << injection << '"' << std::endl;
    }
    if (not editor->presetsPage->injections.empty()) configOutput << std::endl;
    outputConfigPresetsStyles(configOutput, editor);
    outputConfigPresetsBlades(configOutput, editor);
    configOutput << "#endif\n\n" << std::flush;
}

void Configuration::outputConfigPresetsStyles(std::ofstream& configOutput, const EditorWindow *editor) {
    for (const BladeArrayDlg::BladeArray& bladeArray : editor->bladesPage->bladeArrayDlg->bladeArrays) {
        configOutput << "Preset " << bladeArray.name << "[] = {\n";
        for (const PresetsPage::PresetConfig& preset : bladeArray.presets) {
            configOutput << "\t{ \"" << preset.dirs << "\", \"" << preset.track << "\",\n";
            if (preset.styles.size() > 0) {
                for (const auto& style : preset.styles) {
                    string line;
                    std::istringstream commentStream(style.comment);

                    if (not style.comment.empty()) {
                        configOutput << "\t\t/*\n";
                    }
                    while (!false) {
                        std::getline(commentStream, line);
                        if (commentStream.eof()) break;
                        configOutput << "\t\t * " << line << '\n';
                    }
                    if (not style.comment.empty()) {
                        configOutput << "\t\t */\n";
                    }

                    std::istringstream styleStream(style.style);
                    while (!false) {
                        std::getline(styleStream, line);
                        configOutput << "\t\t" << line;
                        if (styleStream.eof()) {
                            configOutput << ",\n";
                            break;
                        } 
                        configOutput << '\n';
                    }
                }
            } else configOutput << "\t\t,\n";

            configOutput << "\t\t\"" << preset.name << "\"\n\t}";
            // If not the last one, add comma
            if (&bladeArray.presets[bladeArray.presets.size() - 1] != &preset) configOutput << ",";
            configOutput << '\n';
        }
        configOutput << "};\n";
    }
}

void Configuration::outputConfigPresetsBlades(std::ofstream& configOutput, const EditorWindow *editor) {
    configOutput << "BladeConfig blades[] = {" << std::endl;
    for (const BladeArrayDlg::BladeArray& bladeArray : editor->bladesPage->bladeArrayDlg->bladeArrays) {
        configOutput << "\t{ " << (bladeArray.name == "no_blade" ? "NO_BLADE" : std::to_string(bladeArray.value)) << "," << std::endl;
        for (const BladesPage::BladeConfig& blade : bladeArray.blades) {
            if (blade.type == BD_PIXELRGB || blade.type == BD_PIXELRGBW) {
                if (blade.isSubBlade) genSubBlades(configOutput, blade);
                else {
                    configOutput << "\t\t";
                    genWS281X(configOutput, blade);
                    configOutput << "," << std::endl;
                }
            } else if (blade.type == BD_SIMPLE) {
                std::array<bool, 4> powerPinUsed;
                configOutput << "\t\tSimpleBladePtr<";

                auto outputLED{[&configOutput](BladesPage::LED ledSel, int32_t resistance) {
                        configOutput << BladesPage::ledToStr(ledSel);;
                        auto usesResistance{ledSel & BladesPage::USES_RESISTANCE};
                        if (usesResistance) {
                            configOutput << '<' << resistance << ">, ";
                        } else configOutput << ", ";
                    }};

                outputLED(blade.star1, blade.star1Resistance);
                powerPinUsed[0] = blade.star1 != BladesPage::NONE;
                outputLED(blade.star2, blade.star2Resistance);
                powerPinUsed[1] = blade.star2 != BladesPage::NONE;
                outputLED(blade.star3, blade.star3Resistance);
                powerPinUsed[2] = blade.star3 != BladesPage::NONE;
                outputLED(blade.star4, blade.star4Resistance);
                powerPinUsed[3] = blade.star4 != BladesPage::NONE;

                int8_t usageIndex = 0;
                for (auto& usePowerPin : powerPinUsed) {
                    if (usePowerPin && usageIndex < static_cast<int8_t>(blade.powerPins.size())) {
                        configOutput << blade.powerPins.at(usageIndex++);
                    } else {
                        configOutput << "-1";
                    }

                    if (&usePowerPin != &powerPinUsed[3]) configOutput << ", ";
                }
                configOutput << ">()," << std::endl;
            }
        }
        configOutput << "\t\tCONFIGARRAY(" << bladeArray.name << "), \"" << bladeArray.name << "\"" << std::endl << "\t}";
        if (&bladeArray != &editor->bladesPage->bladeArrayDlg->bladeArrays[editor->bladesPage->bladeArrayDlg->bladeArrays.size() - 1]) configOutput << ",";
        configOutput << std::endl;
    }
    configOutput << "};" << std::endl;
}

void Configuration::genWS281X(std::ofstream& configOutput, const BladesPage::BladeConfig& blade) {
    wxString bladePin = blade.dataPin;
    wxString bladeColor = blade.type == BD_PIXELRGB || blade.useRGBWithWhite ? blade.colorType : [=](wxString colorType) -> wxString { colorType.replace(colorType.find("W"), 1, "w"); return colorType; }(blade.colorType);

    configOutput << "WS281XBladePtr<" << blade.numPixels << ", " << bladePin << ", Color8::" << bladeColor << ", PowerPINS<";
    for (const auto& powerPin : blade.powerPins) {
        configOutput << powerPin << (&powerPin != &blade.powerPins.back() ? ", " : "");
    }
    configOutput << ">>()";
};

void Configuration::genSubBlades(std::ofstream& configOutput, const BladesPage::BladeConfig& blade) {
    int32_t subNum{0};
    for (const auto& subBlade : blade.subBlades) {
        if (blade.useStride) {
            configOutput << "\t\tSubBladeWithStride( ";
            configOutput << subNum << ", ";
            configOutput << blade.numPixels - blade.subBlades.size() + subNum << ", ";
            configOutput << blade.subBlades.size() << ", ";
        } else if (blade.useZigZag) {
            configOutput << "\t\tSubBladeZZ( ";
            configOutput << "0, ";
            configOutput << blade.numPixels - 1 << ", ";
            configOutput << blade.subBlades.size() << ", ";
            configOutput << subNum << ", ";
        } else {
            configOutput << "\t\tSubBlade( ";
            configOutput << subBlade.startPixel << ", " << subBlade.endPixel << ", ";
        }

        if (subNum == 0) {
            genWS281X(configOutput, blade);
            configOutput << ")," << std::endl;
        } else {
            configOutput << "NULL)," << std::endl;
        }

        subNum++;
    }
}

void Configuration::outputConfigButtons(std::ofstream& configOutput, const EditorWindow *editor) {
    configOutput << "#ifdef CONFIG_BUTTONS" << std::endl;
    configOutput << "Button PowerButton(BUTTON_POWER, powerButtonPin, \"pow\");" << std::endl;
    if (editor->generalPage->buttons->entry()->GetValue() >= 2) configOutput << "Button AuxButton(BUTTON_AUX, auxPin, \"aux\");" << std::endl;
    if (editor->generalPage->buttons->entry()->GetValue() == 3) configOutput << "Button Aux2Button(BUTTON_AUX2, aux2Pin, \"aux\");" << std::endl; // figure out aux2 syntax
    configOutput << "#endif" << std::endl << std::endl; // CONFIG_BUTTONS
}

bool Configuration::readConfig(const filepath& filePath, EditorWindow* editor) {
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
                if (buffer == "CONFIG_TOP") Configuration::readConfigTop(file, editor);
                if (buffer == "CONFIG_PROP") Configuration::readConfigProp(file, editor);
                if (buffer == "CONFIG_PRESETS") Configuration::readConfigPresets(file, editor);
                if (buffer == "CONFIG_STYLES") Configuration::readConfigStyles(file, editor);
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

void Configuration::tryAddInjection(const string& buffer, EditorWindow *editor) {
    auto& logger{Log::Context::getGlobal().createLogger("Configuration::tryAddInjection()")};

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

bool Configuration::importConfig(EditorWindow* editor) {
    wxFileDialog configLocation(editor, "Choose ProffieOS Config File", "", "", "C Header Files (*.h)|*.h", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (configLocation.ShowModal() == wxID_CANCEL) return false; // User Closed

    return Configuration::readConfig(configLocation.GetPath().ToStdString(), editor);
}

void Configuration::readConfigTop(std::ifstream& file, EditorWindow* editor) {
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

void Configuration::setCustomDefines(EditorWindow* editor) {
    for (const auto& define : editor->settings->readDefines) {
        auto key = Settings::ProffieDefine::parseKey(define);
        if (!key.first.empty()) editor->generalPage->customOptDlg->addDefine(key.first, key.second);
    }
}

void Configuration::readConfigProp(std::ifstream& file, EditorWindow* editor) {
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

void Configuration::readConfigPresets(std::ifstream& file, EditorWindow* editor) {
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

void Configuration::readConfigStyles(std::ifstream& file, EditorWindow* editor) {
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

void Configuration::readPresetArray(std::ifstream& file, EditorWindow* editor) {
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

void Configuration::readBladeArray(std::ifstream& file, EditorWindow* editor) {
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

bool Configuration::runPreChecks(EditorWindow *editor, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Configuration::runPreChecks()")};

    if (editor->bladesPage->bladeArrayDlg->enableDetect->GetValue() && editor->bladesPage->bladeArrayDlg->detectPin->entry()->GetValue() == "") {
        errorMessage(logger, editor, wxTRANSLATE("Blade Detect Pin cannot be empty."));
        return false;
    }
    if (editor->bladesPage->bladeArrayDlg->enableID->GetValue() && editor->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue() == "") {
        errorMessage(logger, editor, wxTRANSLATE("Blade ID Pin cannot be empty."));
        return false;
    }
    auto arrayNameIsEmpty{[&]() { 
        for (const BladeArrayDlg::BladeArray& array : editor->bladesPage->bladeArrayDlg->bladeArrays) {
            if (array.name == "") return true;
        }
        return false; 
    }};
    if (arrayNameIsEmpty()) {
        errorMessage(logger, editor, wxTRANSLATE("Blade Array Name cannot be empty."));
        return false;
    }
    if (editor->bladesPage->bladeArrayDlg->enableID->GetValue() && editor->bladesPage->bladeArrayDlg->mode->entry()->GetStringSelection() == BLADE_ID_MODE_BRIDGED && editor->bladesPage->bladeArrayDlg->pullupPin->entry()->GetValue() == "") {
        errorMessage(logger, editor, wxTRANSLATE("Pullup Pin cannot be empty."));
        return false;
    }
    if (editor->bladesPage->bladeArrayDlg->enableDetect->GetValue() && editor->bladesPage->bladeArrayDlg->enableID->GetValue() && editor->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue() == editor->bladesPage->bladeArrayDlg->detectPin->entry()->GetValue()) {
        errorMessage(logger, editor, wxTRANSLATE("Blade ID Pin and Blade Detect Pin cannot be the same."));
        return false;
    }

    auto getNumBlades{[](const BladeArrayDlg::BladeArray& array) {
            int32 numBlades{0};
            for (const BladesPage::BladeConfig& blade : array.blades) {
                blade.isSubBlade ? numBlades += static_cast<int32>(blade.subBlades.size()) : numBlades++;
            }
            return numBlades;
    }};
    auto bladeArrayLengthsEqual{[&]() -> bool {
        int32 lastNumBlades{getNumBlades(editor->bladesPage->bladeArrayDlg->bladeArrays.at(0))};
        for (const BladeArrayDlg::BladeArray& array : editor->bladesPage->bladeArrayDlg->bladeArrays) {
            if (getNumBlades(array) != lastNumBlades) return false;
            lastNumBlades = getNumBlades(array);
        }
        return true;
    }};
    if (not bladeArrayLengthsEqual()) {
        errorMessage(logger, editor, wxTRANSLATE("All Blade Arrays must be the same length.\n\nPlease add/remove blades to make them equal"));
        return false;
    }

    for (auto& bladeArray : editor->bladesPage->bladeArrayDlg->bladeArrays) {
        for (auto idx{0}; idx < bladeArray.blades.size(); ++idx) {
            auto& blade{bladeArray.blades[idx]};
            if (blade.type != BD_SIMPLE) continue;

            uint32 numActiveLEDs{0};
            if (blade.star1 != BladesPage::NONE) ++numActiveLEDs;
            if (blade.star2 != BladesPage::NONE) ++numActiveLEDs;
            if (blade.star3 != BladesPage::NONE) ++numActiveLEDs;
            if (blade.star4 != BladesPage::NONE) ++numActiveLEDs;

            if (blade.powerPins.size() != numActiveLEDs) {
                errorMessage(
                    logger,
                    editor,
                    wxTRANSLATE("Simple blade %d in array \"%s\" with %d active LEDs should have %d power pins selected. (Has %d)"),
                    idx,
                    bladeArray.name,
                    numActiveLEDs,
                    numActiveLEDs,
                    blade.powerPins.size()
                );
                return false;
            }
        }
        for (auto& preset : bladeArray.presets) {
            for (auto& [ comment, style ] : preset.styles) {
                if (style.empty()) continue;
                constexpr cstring ERROR_STRING{wxTRANSLATE("Malformed bladestyle in preset %s in blade array %s:\n\n%s")};

                size_t depth{0};
                for (const char chr : style) {
                    if (chr == '<') ++depth;
                    else if (chr == '>') --depth;
                }
                if (depth != 0) {
                    errorMessage(logger, editor, ERROR_STRING, preset.name.empty() ? "[unnamed]" : preset.name, bladeArray.name, "Mismatched \"<>\"");
                    return false;
                }

                depth = 0;
                for (const char chr : style) {
                    if (chr == '(') ++depth;
                    else if (chr == ')') --depth;
                }
                if (depth != 0) {
                    errorMessage(logger, editor, ERROR_STRING, preset.name.empty() ? "[unnamed]" : preset.name, bladeArray.name, "Mismatched \"()\"");
                    return false;
                }
            }
        }
    }

    return true;
}

const Configuration::MapPair& Configuration::findInVMap(const Configuration::VMap& map, const wxString& search) {
    return *std::find_if(map.begin(), map.end(), [&](const MapPair& pair) { return (pair.second == search || pair.first == search); });
}

wxArrayString Configuration::orientationStrings() {
    wxArrayString ret;
    for (auto idx{0}; idx < ORIENTATION_MAX; ++idx) {
        ret.emplace_back(orientToStr(static_cast<Orientation>(idx)));
    }
    return ret;
}

wxString Configuration::orientToStr(Orientation orient) {
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

