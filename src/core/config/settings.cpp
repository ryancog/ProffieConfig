// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/config/settings.h"

#include "editor/editorwindow.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/bladeidpage.h"

#include <cstring>

Settings::Settings() {
  linkDefines();
  setCustomInputParsers();
  setCustomOutputParsers();
}

void Settings::linkDefines() {
# define ENTRY(name, ...) { name, new ProffieDefine(name, __VA_ARGS__) }
# define CHECKER(name) [](const ProffieDefine* name) -> bool
# define IDSETTING(setting) EditorWindow::instance->idPage->setting->GetValue()

  generalDefines = {
      // General
      ENTRY("NUM_BLADES", -1, (Misc::numEntry*)nullptr, CHECKER(){ return true; }),
      ENTRY("NUM_BUTTONS", 2, EditorWindow::instance->generalPage->buttons, CHECKER(){ return true; }),
      ENTRY("VOLUME", 2000, EditorWindow::instance->generalPage->volume, CHECKER(){ return true; }),
      ENTRY("CLASH_THRESHOLD_G", 3.0, EditorWindow::instance->generalPage->clash, CHECKER(){ return true; }),
      ENTRY("SAVE_COLOR_CHANGE", true, EditorWindow::instance->generalPage->colorSave),
      ENTRY("SAVE_PRESET", true, EditorWindow::instance->generalPage->presetSave),
      ENTRY("SAVE_VOLUME", true, EditorWindow::instance->generalPage->volumeSave),
      ENTRY("SAVE_STATE", false, (wxCheckBox*)nullptr),

      ENTRY("ENABLE_SSD1306", false, EditorWindow::instance->generalPage->enableOLED),

      ENTRY("DISABLE_COLOR_CHANGE", false, EditorWindow::instance->generalPage->disableColor),
      ENTRY("DISABLE_TALKIE", false, EditorWindow::instance->generalPage->noTalkie),
      ENTRY("DISABLE_BASIC_PARSER_STYLES", true, EditorWindow::instance->generalPage->noBasicParsers),
      ENTRY("DISABLE_DIAGNOSTIC_COMMANDS", true, EditorWindow::instance->generalPage->disableDiagnosticCommands),
      ENTRY("ENABLE_DEVELOPER_COMMANDS", false, EditorWindow::instance->generalPage->enableDeveloperCommands),

      ENTRY("PLI_OFF_TIME", 2, EditorWindow::instance->generalPage->pliTime, CHECKER(){ return true; }),
      ENTRY("IDLE_OFF_TIME", 15, EditorWindow::instance->generalPage->idleTime, CHECKER(){ return true; }),
      ENTRY("MOTION_TIMEOUT", 10, EditorWindow::instance->generalPage->motionTime, CHECKER(){ return true; }),

      ENTRY("BLADE_DETECT_PIN", "", EditorWindow::instance->idPage->detectPin, CHECKER(){ return IDSETTING(enableDetect); }),
      ENTRY("BLADE_ID_CLASS", "", EditorWindow::instance->idPage->mode, CHECKER(){ return IDSETTING(enableID); }),
      ENTRY("ENABLE_POWER_FOR_ID", false, EditorWindow::instance->idPage->enablePowerForID, CHECKER(def){ return IDSETTING(enableID) && def->getState(); }),
      ENTRY("BLADE_ID_SCAN_MILLIS", 1000, EditorWindow::instance->idPage->scanIDMillis, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
      ENTRY("BLADE_ID_TIMES", 10, EditorWindow::instance->idPage->numIDTimes, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
  };

# undef ENTRY
# undef CHECKER
# undef IDSETTING
}

void Settings::setCustomInputParsers() {
  generalDefines["NUM_BLADES"]->overrideParser ([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    numBlades = std::stoi(key.second);
    return true;
  });
  generalDefines["SAVE_STATE"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    EditorWindow::instance->generalPage->colorSave->SetValue(true);
    EditorWindow::instance->generalPage->presetSave->SetValue(true);
    EditorWindow::instance->generalPage->volumeSave->SetValue(true);
    return true;
  });
  generalDefines["BLADE_DETECT_PIN"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    EditorWindow::instance->idPage->enableDetect->SetValue(true);
    EditorWindow::instance->idPage->detectPin->SetValue(key.second);
    return false;
  });
  generalDefines["BLADE_ID_CLASS"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    EditorWindow::instance->idPage->enableID->SetValue(true);
    key.second = std::strtok(key.second.data(), "< ");
    if (key.second == "SnapshotBladeID") {
      EditorWindow::instance->idPage->mode->SetValue(BLADE_ID_MODE_SNAPSHOT);
      EditorWindow::instance->idPage->IDPin->entry->SetValue(std::strtok(nullptr, "<> "));
    } else if (key.second == "ExternalPullupBladeID") {
      EditorWindow::instance->idPage->mode->SetValue(BLADE_ID_MODE_EXTERNAL);
      EditorWindow::instance->idPage->IDPin->entry->SetValue(std::strtok(nullptr, "<, "));
      EditorWindow::instance->idPage->pullupResistance->num->SetValue(std::stod(std::strtok(nullptr, ",> ")));
    } else if (key.second == "BridgedPullupBladeID") {
      EditorWindow::instance->idPage->mode->SetValue(BLADE_ID_MODE_BRIDGED);
      EditorWindow::instance->idPage->IDPin->entry->SetValue(std::strtok(nullptr, "<, "));
      EditorWindow::instance->idPage->pullupPin->entry->SetValue(std::strtok(nullptr, ",> "));
    }
    return true;
  });
  generalDefines["BLADE_ID_SCAN_MILLIS"]->overrideParser([](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    EditorWindow::instance->idPage->scanIDMillis->SetValue(std::stoi(key.second));
    EditorWindow::instance->idPage->continuousScans->SetValue(true);
    return true;
  });
  generalDefines["BLADE_ID_TIMES"]->overrideParser([](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    EditorWindow::instance->idPage->numIDTimes->SetValue(std::stoi(key.second));
    EditorWindow::instance->idPage->continuousScans->SetValue(true);
    return true;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    EditorWindow::instance->idPage->enablePowerForID->SetValue(true);
    std::strtok(key.second.data(), "<");
    char* pwrPinTest = std::strtok(nullptr, "<>, ");
    while (pwrPinTest != nullptr) {
      key.second = pwrPinTest;
      if (key.second == "bladePowerPin1") EditorWindow::instance->idPage->powerPin1->SetValue(true);
      if (key.second == "bladePowerPin2") EditorWindow::instance->idPage->powerPin2->SetValue(true);
      if (key.second == "bladePowerPin3") EditorWindow::instance->idPage->powerPin3->SetValue(true);
      if (key.second == "bladePowerPin4") EditorWindow::instance->idPage->powerPin4->SetValue(true);
      if (key.second == "bladePowerPin5") EditorWindow::instance->idPage->powerPin5->SetValue(true);
      if (key.second == "bladePowerPin6") EditorWindow::instance->idPage->powerPin6->SetValue(true);

      pwrPinTest = std::strtok(nullptr, "<>, ");
    }
    return true;
  });
}
void Settings::setCustomOutputParsers() {
  generalDefines["NUM_BLADES"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    int32_t numBlades = 0;
    for (const BladesPage::BladeConfig& blade : EditorWindow::instance->idPage->bladeArrays[EditorWindow::instance->bladesPage->bladeArray->GetSelection()].blades) numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
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
  generalDefines["BLADE_ID_CLASS"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    auto mode = EditorWindow::instance->idPage->mode->GetValue();
    std::string returnVal = def->getName() + " ";
    if (mode == BLADE_ID_MODE_SNAPSHOT) returnVal + "SnapshotBladeID<" + EditorWindow::instance->idPage->IDPin->entry->GetValue() + ">";
    else if (mode == BLADE_ID_MODE_BRIDGED) returnVal + "ExternalPullupBladeID<" + EditorWindow::instance->idPage->IDPin->entry->GetValue() + ", " + EditorWindow::instance->idPage->pullupResistance->num->GetTextValue() + ">";
    else if (mode == BLADE_ID_MODE_EXTERNAL) returnVal + "BridgedPullupBladeID<" + EditorWindow::instance->idPage->IDPin->entry->GetValue() + ", " + EditorWindow::instance->idPage->pullupPin->entry->GetValue();

    return returnVal;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    std::string returnVal = def->getName() + " PowerPINS<";
    std::vector<std::string> powerPins;
    if (EditorWindow::instance->idPage->powerPin1->GetValue()) powerPins.push_back("bladePowerPin1");
    if (EditorWindow::instance->idPage->powerPin2->GetValue()) powerPins.push_back("bladePowerPin2");
    if (EditorWindow::instance->idPage->powerPin3->GetValue()) powerPins.push_back("bladePowerPin3");
    if (EditorWindow::instance->idPage->powerPin4->GetValue()) powerPins.push_back("bladePowerPin4");
    if (EditorWindow::instance->idPage->powerPin5->GetValue()) powerPins.push_back("bladePowerPin5");
    if (EditorWindow::instance->idPage->powerPin6->GetValue()) powerPins.push_back("bladePowerPin6");

    for (int32_t pin = 0; pin < static_cast<int32_t>(powerPins.size()); pin++) {
      returnVal += powerPins.at(pin);
      if (pin < static_cast<int32_t>(powerPins.size()) -1) returnVal += ",";
    }

    returnVal += ">";
    return returnVal;
  });
}

void Settings::ProffieDefine::loadDefault() {
  if (element == nullptr) return;

  if (typeid(element) == typeid(wxCheckBox*)) {
    ((wxCheckBox*)element)->SetValue(true);
    return;
  }
  if (typeid(element) == typeid(wxRadioButton*)) return;
  if (typeid(element) == typeid(Misc::numEntry*)) return;
  if (typeid(element) == typeid(Misc::numEntryDouble*)) return;
  if (typeid(element) == typeid(Misc::comboBoxEntry*)) return;
  if (typeid(element) == typeid(Misc::textEntry*)) return;
}

void Settings::parseDefines(const std::vector<std::string>& _defList) {
  for (const auto& [key, defObj] : generalDefines) {
    for (const std::string& entry : _defList) {
      if (defObj->parseDefine(entry)) break;
    }
  }
}

void Settings::loadDefaults() {
  for (const auto& [key, defObj] : generalDefines) {
    defObj->loadDefault();
  }
}
int32_t Settings::ProffieDefine::getNum() const {
  if (type != Type::NUMERIC) return 0;
  return const_cast<Misc::numEntry*>(static_cast<const Misc::numEntry*>(element))->num->GetValue();
}
double Settings::ProffieDefine::getDec() const {
  if (type != Type::DECIMAL) return 0;
  return const_cast<Misc::numEntryDouble*>(static_cast<const Misc::numEntryDouble*>(element))->num->GetValue();
}
bool Settings::ProffieDefine::getState() const {
  if (type == Type::STATE) return const_cast<wxCheckBox*>(static_cast<const wxCheckBox*>(element))->GetValue();
  if (type == Type::RADIO) return const_cast<wxRadioButton*>(static_cast<const wxRadioButton*>(element))->GetValue();

  return false;
}
std::string Settings::ProffieDefine::getString() const {
  if (type == Type::TEXT) return const_cast<Misc::textEntry*>(static_cast<const Misc::textEntry*>(element))->entry->GetValue().ToStdString();
  if (type == Type::COMBO) return const_cast<Misc::comboBoxEntry*>(static_cast<const Misc::comboBoxEntry*>(element))->entry->GetValue().ToStdString();

  return "";
}

Settings::ProffieDefine::ProffieDefine(std::string _name, int32_t _defaultValue, Misc::numEntry* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .num = _defaultValue }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, double _defaultValue, Misc::numEntryDouble* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .dec = _defaultValue }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxCheckBox* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .state = _defaultState }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxRadioButton* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .state = _defaultState }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultSelection, wxComboBox* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .str = _defaultSelection.ToStdString().data() }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultEntry, Misc::textEntry* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .str = _defaultEntry.ToStdString().data() }), identifier(_name), element(_element), checkOutput(_check) {}


std::pair<std::string, std::string> Settings::ProffieDefine::parseKey(const std::string& _input) {
  std::pair<std::string, std::string> key;
  std::string parseVal = _input;

  key.first = std::strtok(&parseVal[0], " ");
  // Handle trying to construct from nullptr
  char* val = std::strtok(nullptr, " \n\r");
  if (val != nullptr) key.second = val;

  return key;
}
