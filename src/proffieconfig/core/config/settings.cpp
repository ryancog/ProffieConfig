#include "settings.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek


#include "../../core/config/configuration.h"
#include "../../editor/editorwindow.h"
#include "../../editor/pages/generalpage.h"
#include "../../editor/dialogs/bladearraydlg.h"

#include <cstring>

Settings::Settings(EditorWindow* _parent) : parent(_parent) {
  linkDefines();
  setCustomInputParsers();
  setCustomOutputParsers();
}
Settings::~Settings() {
  for (const auto& define : generalDefines) delete define.second;
}

void Settings::linkDefines() {
#   define ENTRY(name, ...) { name, new ProffieDefine(name, __VA_ARGS__) }
#   define CHECKER(name) [&](const ProffieDefine* name) -> bool
#   define IDSETTING(setting) parent->bladesPage->bladeArrayDlg->setting->GetValue()

    generalDefines = {
        // General
        ENTRY("NUM_BLADES", -1, static_cast<PCUI::Numeric *>(nullptr), CHECKER(){ return true; }),
        ENTRY("NUM_BUTTONS", 2, parent->generalPage->buttons, CHECKER(){ return true; }),
        ENTRY("VOLUME", 1500, parent->generalPage->volume, CHECKER(){ return true; }),
        ENTRY("CLASH_THRESHOLD_G", 3.0, parent->generalPage->clash, CHECKER(){ return true; }),
        ENTRY("SAVE_COLOR_CHANGE", true, parent->generalPage->colorSave),
        ENTRY("SAVE_PRESET", true, parent->generalPage->presetSave),
        ENTRY("SAVE_VOLUME", true, parent->generalPage->volumeSave),
        ENTRY("SAVE_STATE", false, static_cast<wxCheckBox*>(nullptr), CHECKER(){ return false; }),

        ENTRY("ENABLE_SSD1306", false, parent->generalPage->enableOLED),

        ENTRY("DISABLE_COLOR_CHANGE", false, parent->generalPage->disableColor),
        ENTRY("DISABLE_TALKIE", false, parent->generalPage->noTalkie),
        ENTRY("DISABLE_BASIC_PARSER_STYLES", true, parent->generalPage->noBasicParsers),
        ENTRY("DISABLE_DIAGNOSTIC_COMMANDS", true, parent->generalPage->disableDiagnosticCommands),

        ENTRY("ORIENTATION", Configuration::Orientation.begin()->first, parent->generalPage->orientation, CHECKER(){ return true; }),
        ENTRY("PLI_OFF_TIME", 2, parent->generalPage->pliTime, CHECKER(){ return true; }),
        ENTRY("IDLE_OFF_TIME", 15, parent->generalPage->idleTime, CHECKER(){ return true; }),
        ENTRY("MOTION_TIMEOUT", 10, parent->generalPage->motionTime, CHECKER(){ return true; }),

        ENTRY("BLADE_DETECT_PIN", "", parent->bladesPage->bladeArrayDlg->detectPin, CHECKER(){ return IDSETTING(enableDetect); }),
        ENTRY("BLADE_ID_CLASS", "", parent->bladesPage->bladeArrayDlg->mode, CHECKER(){ return IDSETTING(enableID); }),
        ENTRY("ENABLE_POWER_FOR_ID", false, parent->bladesPage->bladeArrayDlg->enablePowerForID, CHECKER(def){ return IDSETTING(enableID) && def->getState(); }),
        ENTRY("BLADE_ID_SCAN_MILLIS", 1000, parent->bladesPage->bladeArrayDlg->scanIDMillis, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
        ENTRY("BLADE_ID_TIMES", 10, parent->bladesPage->bladeArrayDlg->numIDTimes, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
    };

#   undef ENTRY
#   undef CHECKER
#   undef IDSETTING
}

void Settings::setCustomInputParsers() {
  generalDefines["NUM_BLADES"]->overrideParser ([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    numBlades = std::stoi(key.second);
    return true;
  });
  generalDefines["SAVE_STATE"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    parent->generalPage->colorSave->SetValue(true);
    parent->generalPage->presetSave->SetValue(true);
    parent->generalPage->volumeSave->SetValue(true);
    return true;
  });
  generalDefines["ORIENTATION"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    parent->generalPage->orientation->entry()->SetStringSelection(Configuration::findInVMap(Configuration::Orientation, key.second).first);
    return true;
  });
  generalDefines["ORIENTATION"]->overrideOutput([&](const ProffieDefine* def) -> std::string {
    return {def->getName() + " " + Configuration::findInVMap(Configuration::Orientation, def->getString()).second};
  });
  generalDefines["BLADE_DETECT_PIN"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladesPage->bladeArrayDlg->enableDetect->SetValue(true);
    parent->bladesPage->bladeArrayDlg->detectPin->entry()->SetValue(key.second);
    return true;
  });
  generalDefines["BLADE_ID_CLASS"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladesPage->bladeArrayDlg->enableID->SetValue(true);
    const auto typeEnd{key.second.find('<')};
    const auto type{key.second.substr(0, typeEnd)};
    if (type == "SnapshotBladeID") {
      parent->bladesPage->bladeArrayDlg->mode->entry()->SetStringSelection(BLADE_ID_MODE_SNAPSHOT);
      const auto pinBegin{key.second.find_first_not_of("< ", typeEnd)};
      const auto pinEnd{key.second.find_first_of(" >", pinBegin)};
      parent->bladesPage->bladeArrayDlg->IDPin->entry()->SetValue(key.second.substr(pinBegin, pinEnd));
    } else if (type == "ExternalPullupBladeID") {
      parent->bladesPage->bladeArrayDlg->mode->entry()->SetStringSelection(BLADE_ID_MODE_EXTERNAL);
      const auto pinBegin{key.second.find_first_not_of("< ", typeEnd)};
      const auto pinEnd{key.second.find_first_of(" >", pinBegin)};
      parent->bladesPage->bladeArrayDlg->IDPin->entry()->SetValue(key.second.substr(pinBegin, pinEnd));
      const auto resBegin{key.second.find_first_not_of(", ", pinEnd)};
      const auto resEnd{key.second.find_first_of(" >", resBegin)};
      parent->bladesPage->bladeArrayDlg->pullupResistance->entry()->SetValue(std::stod(key.second.substr(resBegin, resEnd)));
    } else if (type == "BridgedPullupBladeID") {
      parent->bladesPage->bladeArrayDlg->mode->entry()->SetStringSelection(BLADE_ID_MODE_BRIDGED);
      const auto pinBegin{key.second.find_first_not_of("< ", typeEnd)};
      const auto pinEnd{key.second.find_first_of(" >", pinBegin)};
      parent->bladesPage->bladeArrayDlg->IDPin->entry()->SetValue(key.second.substr(pinBegin, pinEnd));
      const auto pullupPinBegin{key.second.find_first_not_of(", ", pinEnd)};
      const auto pullupPinEnd{key.second.find_first_of(" >", pullupPinBegin)};
      parent->bladesPage->bladeArrayDlg->pullupPin->entry()->SetValue(key.second.substr(pullupPinBegin, pullupPinEnd));
    }
    return true;
  });
  generalDefines["BLADE_ID_SCAN_MILLIS"]->overrideParser([&](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladesPage->bladeArrayDlg->scanIDMillis->entry()->SetValue(std::stoi(key.second));
    parent->bladesPage->bladeArrayDlg->continuousScans->SetValue(true);
    return true;
  });
  generalDefines["BLADE_ID_TIMES"]->overrideParser([&](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladesPage->bladeArrayDlg->numIDTimes->entry()->SetValue(std::stoi(key.second));
    parent->bladesPage->bladeArrayDlg->continuousScans->SetValue(true);
    return true;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladesPage->bladeArrayDlg->enablePowerForID->SetValue(true);
    std::strtok(key.second.data(), "<");
    char* pwrPinTest = std::strtok(nullptr, "<>, ");
    while (pwrPinTest != nullptr) {
      key.second = pwrPinTest;
      if (key.second == "bladePowerPin1") parent->bladesPage->bladeArrayDlg->powerPin1->SetValue(true);
      if (key.second == "bladePowerPin2") parent->bladesPage->bladeArrayDlg->powerPin2->SetValue(true);
      if (key.second == "bladePowerPin3") parent->bladesPage->bladeArrayDlg->powerPin3->SetValue(true);
      if (key.second == "bladePowerPin4") parent->bladesPage->bladeArrayDlg->powerPin4->SetValue(true);
      if (key.second == "bladePowerPin5") parent->bladesPage->bladeArrayDlg->powerPin5->SetValue(true);
      if (key.second == "bladePowerPin6") parent->bladesPage->bladeArrayDlg->powerPin6->SetValue(true);

      pwrPinTest = std::strtok(nullptr, "<>, ");
    }
    return true;
  });
}
void Settings::setCustomOutputParsers() {
  generalDefines["NUM_BLADES"]->overrideOutput([&](const ProffieDefine* def) -> std::string {
    int32_t numBlades = 0;
    for (const BladesPage::BladeConfig& blade : parent->bladesPage->bladeArrayDlg->bladeArrays[parent->bladesPage->bladeArray->entry()->GetSelection()].blades) numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
    return def->getName() + " " + std::to_string(numBlades);
  });
  generalDefines["PLI_OFF_TIME"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    return def->getName() + " " + std::to_string(def->getNum()) + " * 60 * 1000";
  });
  generalDefines["IDLE_OFF_TIME"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    return def->getName() + " " + std::to_string(def->getNum()) + " * 60 * 1000";
  });
  generalDefines["MOTION_TIMEOUT"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    return def->getName() + " " + std::to_string(def->getNum()) + " * 60 * 1000";
  });
  generalDefines["BLADE_ID_CLASS"]->overrideOutput([&](const ProffieDefine* def) -> std::string {
    auto mode = parent->bladesPage->bladeArrayDlg->mode->entry()->GetStringSelection();
    std::string returnVal = def->getName() + " ";
    if (mode == BLADE_ID_MODE_SNAPSHOT) returnVal + "SnapshotBladeID<" + parent->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue() + ">";
    else if (mode == BLADE_ID_MODE_BRIDGED) returnVal + "ExternalPullupBladeID<" + parent->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue() + ", " + parent->bladesPage->bladeArrayDlg->pullupResistance->entry()->GetTextValue() + ">";
    else if (mode == BLADE_ID_MODE_EXTERNAL) returnVal + "BridgedPullupBladeID<" + parent->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue() + ", " + parent->bladesPage->bladeArrayDlg->pullupPin->entry()->GetValue();

    return returnVal;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideOutput([&](const ProffieDefine* def) -> std::string {
    std::string returnVal = def->getName() + " PowerPINS<";
    std::vector<std::string> powerPins;
    if (parent->bladesPage->bladeArrayDlg->powerPin1->GetValue()) powerPins.push_back("bladePowerPin1");
    if (parent->bladesPage->bladeArrayDlg->powerPin2->GetValue()) powerPins.push_back("bladePowerPin2");
    if (parent->bladesPage->bladeArrayDlg->powerPin3->GetValue()) powerPins.push_back("bladePowerPin3");
    if (parent->bladesPage->bladeArrayDlg->powerPin4->GetValue()) powerPins.push_back("bladePowerPin4");
    if (parent->bladesPage->bladeArrayDlg->powerPin5->GetValue()) powerPins.push_back("bladePowerPin5");
    if (parent->bladesPage->bladeArrayDlg->powerPin6->GetValue()) powerPins.push_back("bladePowerPin6");

    for (int32_t pin = 0; pin < static_cast<int32_t>(powerPins.size()); pin++) {
      returnVal += powerPins.at(pin);
      if (pin < static_cast<int32_t>(powerPins.size()) -1) returnVal += ",";
    }

    returnVal += ">";
    return returnVal;
  });
}

void Settings::parseDefines(std::vector<std::string>& _defList) {
  for (const auto& [key, defObj] : generalDefines) {
    for (auto entry = _defList.begin(); entry < _defList.end();) {
      if (defObj->parseDefine(*entry)) {
        _defList.erase(entry);
        break;
      }
      auto key = ProffieDefine::parseKey(*entry);
      if (
        key.first == "ENABLE_AUDIO" ||
        key.first == "ENABLE_WS2811" ||
        key.first == "ENABLE_SD" ||
        key.first == "ENABLE_MOTION" ||
        key.first == "SHARED_POWER_PINS"
        ) {
        entry = _defList.erase(entry);
        continue;
      }
      entry++;
    }
  }
}

int32_t Settings::ProffieDefine::getNum() const {
  if (type != Type::NUMERIC) return 0;
  return const_cast<PCUI::Numeric *>(static_cast<const PCUI::Numeric*>(element))->entry()->GetValue();
}
double Settings::ProffieDefine::getDec() const {
  if (type != Type::DECIMAL) return 0;
  return const_cast<PCUI::NumericDec*>(static_cast<const PCUI::NumericDec*>(element))->entry()->GetValue();
}
bool Settings::ProffieDefine::getState() const {
  if (type == Type::STATE) return const_cast<wxCheckBox*>(static_cast<const wxCheckBox*>(element))->GetValue();
  if (type == Type::RADIO) return const_cast<wxRadioButton*>(static_cast<const wxRadioButton*>(element))->GetValue();

  return false;
}
std::string Settings::ProffieDefine::getString() const {
  if (type == Type::TEXT) return const_cast<PCUI::Text *>(static_cast<const PCUI::Text *>(element))->entry()->GetValue().ToStdString();
  if (type == Type::COMBO) return const_cast<PCUI::Choice *>(static_cast<const PCUI::Choice *>(element))->entry()->GetStringSelection().ToStdString();

  return "";
}

Settings::ProffieDefine::ProffieDefine(std::string _name, int32_t _defaultValue, PCUI::Numeric* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
                                                                                                                                                                        type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .num = _defaultValue }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, double _defaultValue, PCUI::NumericDec* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
                                                                                                                                                                             type(Type::DECIMAL), looseChecking(_loose), defaultValue({ .dec = _defaultValue }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxCheckBox* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
                                                                                                                                                                     type(Type::STATE), looseChecking(_loose), defaultValue({ .state = _defaultState }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxRadioButton* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
                                                                                                                                                                        type(Type::RADIO), looseChecking(_loose), defaultValue({ .state = _defaultState }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultSelection, PCUI::Choice* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
                                                                                                                                                                             type(Type::COMBO), looseChecking(_loose), defaultValue({ .str = _defaultSelection.ToStdString().data() }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultEntry, PCUI::Text* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
                                                                                                                                                                         type(Type::TEXT), looseChecking(_loose), defaultValue({ .str = _defaultEntry.ToStdString().data() }), identifier(_name), element(_element), checkOutput(_check) {}


std::pair<std::string, std::string> Settings::ProffieDefine::parseKey(const std::string& _input) {
    std::pair<std::string, std::string> key;
    auto parseVal{_input};

    // Handle trying to construct from nullptr
    char* val;
    val = std::strtok(parseVal.data(), " \n\r");
    if (val != nullptr) key.first = val;
    val = std::strtok(nullptr, "\n\r");
    if (val != nullptr) key.second = val;

    return key;
}
