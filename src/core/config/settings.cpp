// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/config/settings.h"

#include "editor/editorwindow.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/bladearraypage.h"

#include <cstring>

Settings::Settings(EditorWindow* _parent) : parent(_parent) {
  linkDefines();
  setCustomInputParsers();
  setCustomOutputParsers();
}

void Settings::linkDefines() {
# define ENTRY(name, ...) { name, new ProffieDefine(name, __VA_ARGS__) }
# define CHECKER(name) [&](const ProffieDefine* name) -> bool
# define IDSETTING(setting) parent->bladeArrayPage->setting->GetValue()

  generalDefines = {
      // General
      ENTRY("NUM_BLADES", -1,( wxSpinCtrl*)nullptr, CHECKER(){ return true; }),
      ENTRY("NUM_BUTTONS", 2, parent->generalPage->buttons.num, CHECKER(){ return true; }),
      ENTRY("VOLUME", 2000, parent->generalPage->volume.num, CHECKER(){ return true; }),
      ENTRY("CLASH_THRESHOLD_G", 3.0, parent->generalPage->clash.num, CHECKER(){ return true; }),
      ENTRY("SAVE_COLOR_CHANGE", true, parent->generalPage->colorSave),
      ENTRY("SAVE_PRESET", true, parent->generalPage->presetSave),
      ENTRY("SAVE_VOLUME", true, parent->generalPage->volumeSave),
      ENTRY("SAVE_STATE", false, (wxCheckBox*)nullptr, CHECKER() { return false; }),

      ENTRY("ENABLE_SSD1306", false, parent->generalPage->enableOLED),

      ENTRY("DISABLE_COLOR_CHANGE", false, parent->generalPage->disableColor),
      ENTRY("DISABLE_TALKIE", false, parent->generalPage->noTalkie),
      ENTRY("DISABLE_BASIC_PARSER_STYLES", true, parent->generalPage->noBasicParsers),
      ENTRY("DISABLE_DIAGNOSTIC_COMMANDS", true, parent->generalPage->disableDiagnosticCommands),
      ENTRY("ENABLE_DEVELOPER_COMMANDS", false, parent->generalPage->enableDeveloperCommands),

      ENTRY("PLI_OFF_TIME", 2, parent->generalPage->pliTime.num, CHECKER(){ return true; }),
      ENTRY("IDLE_OFF_TIME", 15, parent->generalPage->idleTime.num, CHECKER(){ return true; }),
      ENTRY("MOTION_TIMEOUT", 10, parent->generalPage->motionTime.num, CHECKER(){ return true; }),
      
      ENTRY("BLADE_DETECT_PIN", "", parent->bladeArrayPage->detectPin.entry, CHECKER(){ return IDSETTING(enableDetect); }),
      ENTRY("BLADE_ID_CLASS", "", parent->bladeArrayPage->mode, CHECKER(){ return IDSETTING(enableID); }),
      ENTRY("ENABLE_POWER_FOR_ID", false, parent->bladeArrayPage->enablePowerForID, CHECKER(def){ return IDSETTING(enableID) && def->getState(); }),
      ENTRY("BLADE_ID_SCAN_MILLIS", 1000, parent->bladeArrayPage->scanIDMillis.num, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
      ENTRY("BLADE_ID_TIMES", 10, parent->bladeArrayPage->numIDTimes.num, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
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
  generalDefines["SAVE_STATE"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    parent->generalPage->colorSave->SetValue(true);
    parent->generalPage->presetSave->SetValue(true);
    parent->generalPage->volumeSave->SetValue(true);
    return true;
  });
  generalDefines["BLADE_DETECT_PIN"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladeArrayPage->enableDetect->SetValue(true);
    parent->bladeArrayPage->detectPin.entry->SetValue(key.second);
    return false;
  });
  generalDefines["BLADE_ID_CLASS"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladeArrayPage->enableID->SetValue(true);
    key.second = std::strtok(key.second.data(), "< ");
    if (key.second == "SnapshotBladeID") {
      parent->bladeArrayPage->mode->SetValue(BLADE_ID_MODE_SNAPSHOT);
      parent->bladeArrayPage->IDPin.entry->SetValue(std::strtok(nullptr, "<> "));
    } else if (key.second == "ExternalPullupBladeID") {
      parent->bladeArrayPage->mode->SetValue(BLADE_ID_MODE_EXTERNAL);
      parent->bladeArrayPage->IDPin.entry->SetValue(std::strtok(nullptr, "<, "));
      parent->bladeArrayPage->pullupResistance.num->SetValue(std::stod(std::strtok(nullptr, ",> ")));
    } else if (key.second == "BridgedPullupBladeID") {
      parent->bladeArrayPage->mode->SetValue(BLADE_ID_MODE_BRIDGED);
      parent->bladeArrayPage->IDPin.entry->SetValue(std::strtok(nullptr, "<, "));
      parent->bladeArrayPage->pullupPin.entry->SetValue(std::strtok(nullptr, ",> "));
    }
    return true;
  });
  generalDefines["BLADE_ID_SCAN_MILLIS"]->overrideParser([&](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladeArrayPage->scanIDMillis.num->SetValue(std::stoi(key.second));
    parent->bladeArrayPage->continuousScans->SetValue(true);
    return true;
  });
  generalDefines["BLADE_ID_TIMES"]->overrideParser([&](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladeArrayPage->numIDTimes.num->SetValue(std::stoi(key.second));
    parent->bladeArrayPage->continuousScans->SetValue(true);
    return true;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideParser([&](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    parent->bladeArrayPage->enablePowerForID->SetValue(true);
    std::strtok(key.second.data(), "<");
    char* pwrPinTest = std::strtok(nullptr, "<>, ");
    while (pwrPinTest != nullptr) {
      key.second = pwrPinTest;
      if (key.second == "bladePowerPin1") parent->bladeArrayPage->powerPin1->SetValue(true);
      if (key.second == "bladePowerPin2") parent->bladeArrayPage->powerPin2->SetValue(true);
      if (key.second == "bladePowerPin3") parent->bladeArrayPage->powerPin3->SetValue(true);
      if (key.second == "bladePowerPin4") parent->bladeArrayPage->powerPin4->SetValue(true);
      if (key.second == "bladePowerPin5") parent->bladeArrayPage->powerPin5->SetValue(true);
      if (key.second == "bladePowerPin6") parent->bladeArrayPage->powerPin6->SetValue(true);

      pwrPinTest = std::strtok(nullptr, "<>, ");
    }
    return true;
  });
}
void Settings::setCustomOutputParsers() {
  generalDefines["NUM_BLADES"]->overrideOutput([&](const ProffieDefine* def) -> std::string {
    int32_t numBlades = 0;
    for (const BladesPage::BladeConfig& blade : parent->bladeArrayPage->bladeArrays[parent->bladesPage->bladeArray->GetSelection()].blades) numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
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
    auto mode = parent->bladeArrayPage->mode->GetValue();
    std::string returnVal = def->getName() + " ";
    if (mode == BLADE_ID_MODE_SNAPSHOT) returnVal + "SnapshotBladeID<" + parent->bladeArrayPage->IDPin.entry->GetValue() + ">";
    else if (mode == BLADE_ID_MODE_BRIDGED) returnVal + "ExternalPullupBladeID<" + parent->bladeArrayPage->IDPin.entry->GetValue() + ", " + parent->bladeArrayPage->pullupResistance.num->GetTextValue() + ">";
    else if (mode == BLADE_ID_MODE_EXTERNAL) returnVal + "BridgedPullupBladeID<" + parent->bladeArrayPage->IDPin.entry->GetValue() + ", " + parent->bladeArrayPage->pullupPin.entry->GetValue();

    return returnVal;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideOutput([&](const ProffieDefine* def) -> std::string {
    std::string returnVal = def->getName() + " PowerPINS<";
    std::vector<std::string> powerPins;
    if (parent->bladeArrayPage->powerPin1->GetValue()) powerPins.push_back("bladePowerPin1");
    if (parent->bladeArrayPage->powerPin2->GetValue()) powerPins.push_back("bladePowerPin2");
    if (parent->bladeArrayPage->powerPin3->GetValue()) powerPins.push_back("bladePowerPin3");
    if (parent->bladeArrayPage->powerPin4->GetValue()) powerPins.push_back("bladePowerPin4");
    if (parent->bladeArrayPage->powerPin5->GetValue()) powerPins.push_back("bladePowerPin5");
    if (parent->bladeArrayPage->powerPin6->GetValue()) powerPins.push_back("bladePowerPin6");

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
  if (typeid(element) == typeid(Misc::numEntry)) return;
  if (typeid(element) == typeid(Misc::numEntryDouble)) return;
  if (typeid(element) == typeid(Misc::comboBoxEntry)) return;
  if (typeid(element) == typeid(Misc::textEntry)) return;
}

void Settings::parseDefines(std::vector<std::string>& _defList) {
  for (const auto& [key, defObj] : generalDefines) {
    for (auto entry = _defList.begin(); entry < _defList.end();) {
      if (defObj->parseDefine(*entry)) {
        _defList.erase(entry);
        break;
      }
      entry++;
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
  return const_cast<wxSpinCtrl*>(static_cast<const wxSpinCtrl*>(element))->GetValue();
}
double Settings::ProffieDefine::getDec() const {
  if (type != Type::DECIMAL) return 0;
  return const_cast<wxSpinCtrlDouble*>(static_cast<const wxSpinCtrlDouble*>(element))->GetValue();
}
bool Settings::ProffieDefine::getState() const {
  if (type == Type::STATE) return const_cast<wxCheckBox*>(static_cast<const wxCheckBox*>(element))->GetValue();
  if (type == Type::RADIO) return const_cast<wxRadioButton*>(static_cast<const wxRadioButton*>(element))->GetValue();

  return false;
}
std::string Settings::ProffieDefine::getString() const {
  if (type == Type::TEXT) return const_cast<wxTextCtrl*>(static_cast<const wxTextCtrl*>(element))->GetValue().ToStdString();
  if (type == Type::COMBO) return const_cast<wxTextCtrl*>(static_cast<const wxTextCtrl*>(element))->GetValue().ToStdString();

  return "";
}

Settings::ProffieDefine::ProffieDefine(std::string _name, int32_t _defaultValue, wxSpinCtrl* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::NUMERIC), looseChecking(_loose), defaultValue({ .num = _defaultValue }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, double _defaultValue, wxSpinCtrlDouble* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::DECIMAL), looseChecking(_loose), defaultValue({ .dec = _defaultValue }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxCheckBox* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::STATE), looseChecking(_loose), defaultValue({ .state = _defaultState }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxRadioButton* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::RADIO), looseChecking(_loose), defaultValue({ .state = _defaultState }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultSelection, wxComboBox* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::COMBO), looseChecking(_loose), defaultValue({ .str = _defaultSelection.ToStdString().data() }), identifier(_name), element(_element), checkOutput(_check) {}
Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultEntry, wxTextCtrl* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    type(Type::TEXT), looseChecking(_loose), defaultValue({ .str = _defaultEntry.ToStdString().data() }), identifier(_name), element(_element), checkOutput(_check) {}


std::pair<std::string, std::string> Settings::ProffieDefine::parseKey(const std::string& _input) {
  std::pair<std::string, std::string> key;
  std::string parseVal = _input;

  key.first = std::strtok(&parseVal[0], " \n\r");
  // Handle trying to construct from nullptr
  char* val = std::strtok(nullptr, " \n\r");
  if (val != nullptr) key.second = val;

  return key;
}
