// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "config/settings.h"

#include "config/configuration.h"
#include "core/defines.h"
#include "pages/generalpage.h"
#include "pages/bladeidpage.h"
#include "pages/proppage.h"

#include <cstring>

Settings* Settings::instance;
Settings::Settings() {
  instance = this;

  linkDefines();
  setCustomInputParsers();
  setCustomOutputParsers();

  allDefines.insert(generalDefines.begin(), generalDefines.end());
  allDefines.insert(propDefines.begin(), generalDefines.end());
}

void Settings::linkDefines() {
# define ENTRY(name, ...) { name, new ProffieDefine(name, __VA_ARGS__) }
# define CHECKER(name) [](const ProffieDefine* name) -> bool
# define ISFETT PropPage::instance->prop->GetValue() == PR_FETT263
# define ISSA22C PropPage::instance->prop->GetValue() == PR_SA22C
# define ISBC PropPage::instance->prop->GetValue() == PR_BC
# define ISCAIWYN PropPage::instance->prop->GetValue() == PR_CAIWYN
# define PRSETTING(setting) PropPage::instance->setting->GetValue()
# define IDSETTING(setting) BladeIDPage::instance->setting->GetValue()
# define GNSETTING(setting) GeneralPage::instance->setting->GetValue()

  generalDefines = {
      // General
      ENTRY("NUM_BLADES", -1, (Misc::numEntry*)nullptr, CHECKER(){ return true; }),
      ENTRY("NUM_BUTTONS", 2, GeneralPage::instance->buttons, CHECKER(){ return true; }),
      ENTRY("VOLUME", 2000, GeneralPage::instance->volume, CHECKER(){ return true; }),
      ENTRY("CLASH_THRESHOLD_G", 3.0, GeneralPage::instance->clash, CHECKER(){ return true; }),
      ENTRY("SAVE_COLOR_CHANGE", true, GeneralPage::instance->colorSave),
      ENTRY("SAVE_PRESET", true, GeneralPage::instance->presetSave),
      ENTRY("SAVE_VOLUME", true, GeneralPage::instance->volumeSave),
      ENTRY("SAVE_STATE", false, (wxCheckBox*)nullptr),

      ENTRY("ENABLE_SSD1306", false, GeneralPage::instance->enableOLED),

      ENTRY("DISABLE_COLOR_CHANGE", false, GeneralPage::instance->disableColor),
      ENTRY("DISABLE_TALKIE", false, GeneralPage::instance->noTalkie),
      ENTRY("DISABLE_BASIC_PARSER_STYLES", true, GeneralPage::instance->noBasicParsers),
      ENTRY("DISABLE_DIAGNOSTIC_COMMANDS", true, GeneralPage::instance->disableDiagnosticCommands),
      ENTRY("ENABLE_DEVELOPER_COMMANDS", false, GeneralPage::instance->enableDeveloperCommands),

      ENTRY("PLI_OFF_TIME", 2, GeneralPage::instance->pliTime, CHECKER(){ return true; }),
      ENTRY("IDLE_OFF_TIME", 15, GeneralPage::instance->idleTime, CHECKER(){ return true; }),
      ENTRY("MOTION_TIMEOUT", 10, GeneralPage::instance->motionTime, CHECKER(){ return true; }),

      ENTRY("BLADE_DETECT_PIN", "", BladeIDPage::instance->detectPin, CHECKER(){ return IDSETTING(enableDetect); }),
      ENTRY("BLADE_ID_CLASS", "", BladeIDPage::instance->mode, CHECKER(){ return IDSETTING(enableID); }),
      ENTRY("ENABLE_POWER_FOR_ID", false, BladeIDPage::instance->enablePowerForID, CHECKER(def){ return IDSETTING(enableID) && def->getState(); }),
      ENTRY("BLADE_ID_SCAN_MILLIS", 1000, BladeIDPage::instance->scanIDMillis, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
      ENTRY("BLADE_ID_TIMES", 10, BladeIDPage::instance->numIDTimes, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
  };
  propDefines = {
      // Prop-Specific
      ENTRY("STAB_ON",              false,  PropPage::instance->stabOn, CHECKER(def){ return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),
      ENTRY("STAB_ON_PREON",        false,  PropPage::instance->stabOnPreon, CHECKER(def){ return ISFETT && PRSETTING(stabOn) && def->getState(); }, true),
      ENTRY("STAB_ON_NO_BM",        false,  PropPage::instance->stabOnNoBattle, CHECKER(def){ return ISFETT && PRSETTING(stabOn) && def->getState(); }, true),
      ENTRY("SWING_ON",             false,  PropPage::instance->swingOn, CHECKER(def){ return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),
      ENTRY("SWING_ON_PREON",       false,  PropPage::instance->swingOnPreon, CHECKER(def){ return ISFETT && PRSETTING(swingOn) && def->getState(); }, true),
      ENTRY("SWING_ON_NO_BM",       false,  PropPage::instance->swingOnNoBattle, CHECKER(def){ return ISFETT && PRSETTING(swingOn) && def->getState(); }, true),
      ENTRY("SWING_ON_SPEED",       250,    PropPage::instance->swingOnSpeed, CHECKER(def){ return (ISFETT || ISBC || ISSA22C) && PRSETTING(swingOn) && def->getState(); }, true),
      ENTRY("THRUST_ON",            false,  PropPage::instance->thrustOn, CHECKER(def){ return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),
      ENTRY("THRUST_ON_PREON",      false,  PropPage::instance->thrustOnPreon, CHECKER(def){ return ISFETT && PRSETTING(thrustOn) && def->getState(); }, true),
      ENTRY("THRUST_ON_NO_BM",      false,  PropPage::instance->thrustOnNoBattle, CHECKER(def){ return ISFETT && PRSETTING(thrustOn) && def->getState(); }, true),
      ENTRY("TWIST_ON",             false,  PropPage::instance->twistOn, CHECKER(def){ return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),
      ENTRY("TWIST_ON_PREON",       false,  PropPage::instance->twistOnPreon, CHECKER(def){ return ISFETT && PRSETTING(twistOn) && def->getState(); }, true),
      ENTRY("TWIST_ON_NO_BM",       false,  PropPage::instance->twistOnNoBattle, CHECKER(def){ return ISFETT && PRSETTING(twistOn) && def->getState(); }, true),
      ENTRY("TWIST_OFF",            false,  PropPage::instance->twistOff, CHECKER(def){ return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),
      ENTRY("TWIST_OFF_NO_POSTOFF", false,  PropPage::instance->twistOffFast, CHECKER(def){ return ISFETT && PRSETTING(twistOff) && def->getState(); }, true),

      ENTRY("NO_LOCKUP_HOLD",             false,  PropPage::instance->noLockupHold, CHECKER(def){ return (ISSA22C) && def->getState(); }, true),
      ENTRY("ENABLE_AUTO_SWING_BLAST",    false,  PropPage::instance->multiBlastSwing, CHECKER(def){ return (ISBC) && def->getState(); }, true),
      ENTRY("NO_BLADE_NO_GEST_ONOFF",     false,  PropPage::instance->disableGestureNoBlade, CHECKER(def){ return ISBC && def->getState(); }, true),
      ENTRY("BUTTON_CLASH",               false,  PropPage::instance->pwrClash, CHECKER(def) { return ISCAIWYN && def->getState(); }, true),
      ENTRY("BUTTON_LOCKUP",              false,  PropPage::instance->pwrLockup, CHECKER(def){ return ISCAIWYN && def->getState(); }, true),
      ENTRY("HOLD_BUTTON_OFF",            false,  PropPage::instance->pwrHoldOff, CHECKER(def){ return ISFETT && def->getState(); }, true),
      ENTRY("HOLD_BUTTON_LOCKUP",         false,  PropPage::instance->auxHoldLockup, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("USE_BC_MELT_STAB",           false,  PropPage::instance->meltGestureAlways, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("CIRCULAR_VOLUME_MENU",       false,  PropPage::instance->volumeCircular, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("CIRCULAR_DIM_MENU",          false,  PropPage::instance->brightnessCircular, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("MOTION_WAKE_POWER_BUTTON",   false, PropPage::instance->pwrWakeGesture, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("EDIT",                       false,  PropPage::instance->editEnable, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("EDIT_MODE_MENU",             false,  PropPage::instance->editMode, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("EDIT_SETTINGS_MENU",         false,  PropPage::instance->editSettings, CHECKER(def) { return ISFETT && def->getState(); }, true),

      ENTRY("TRACK_PLAYER_NO_PROMPTS",    false,  PropPage::instance->trackPlayerPrompts, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("SAY_COLOR_LIST",             false,  PropPage::instance->spokenColors, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("SAY_BATTERY_VOLTS",          false,  PropPage::instance->spokenBatteryVolts, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("SAY_BATTERY_PERCENT",        false,  PropPage::instance->spokenBatteryPercent, CHECKER(def) { return ISFETT && def->getState(); }, true),

      ENTRY("FORCE_PUSH",                 false,  PropPage::instance->forcePush, CHECKER(def) { return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),
      ENTRY("FORCE_PUSH_ALWAYS",          false,  PropPage::instance->forcePush, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("FORCE_PUSH_LENGTH",          5,  PropPage::instance->forcePushLength, CHECKER(def) { return (ISFETT || ISBC || ISSA22C) && def->getState(); }, true),

      ENTRY("DISABLE_QUOTE_PLAYER",       false,  PropPage::instance->enableQuotePlayer, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("RANDOMIZE_QUOTE_PLAYER",     false,  PropPage::instance->randomizeQuotePlayer, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("QUOTE_PLAYER_START_ON",      false,  PropPage::instance->quotePlayerDefault, CHECKER(def) { return ISFETT && def->getState(); }, true),

      ENTRY("SPECIAL_ABILITIES",          false, PropPage::instance->specialAbilities, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("MULTI_PHASE",                false,  PropPage::instance->multiPhase, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("SPIN_MODE",                  false,  PropPage::instance->spinMode, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("SAVE_CHOREOGRAPHY",          false,  PropPage::instance->saveChoreo, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("SAVE_GESTURE_OFF",           false,  PropPage::instance->saveGesture, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DUAL_MODE_SOUND",            false,  PropPage::instance->dualModeSound, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("QUICK_SELECT_ON_BOOT",       false,  PropPage::instance->quickPresetSelect, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DISABLE_MULTI_BLAST_TOGGLE", false,  PropPage::instance->multiBlastDisableToggle, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DISABLE_MULTI_BLAST",        false,  PropPage::instance->multiBlast, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DISABLE_CHANGE_FONT",        false,  PropPage::instance->noOTFFontChange, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DISABLE_CHANGE_STYLE",       false,  PropPage::instance->noOTFStyleChange, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DISABLE_COPY_PRESET",        false,  PropPage::instance->noOTFPresetCopy, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("CLASH_STRENGTH_SOUND",       false,  PropPage::instance->clashStrengthSound, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("MAX_CLASH",                  10,  PropPage::instance->clashStrengthSoundMaxClash,CHECKER(def) { return ISFETT && PRSETTING(clashStrengthSound) && def->getState(); }, true),

      ENTRY("BATTLE_MODE_START_ON",       false,  PropPage::instance->battleModeOnStart, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("BATTLE_MODE_ALWAYS_ON",      false,  PropPage::instance->battleModeAlways, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("DISABLE_BM_TOGGLE",          false,  PropPage::instance->battleModeNoToggle, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("GESTURE_AUTO_BATTLE_MODE",   false,  PropPage::instance->gestureEnBattle, CHECKER(def) { return ISBC && def->getState(); }, true),
      ENTRY("LOCKUP_DELAY",               200,  PropPage::instance->lockupDelay, CHECKER(){ return ISFETT || ISBC || ISSA22C; }, true),
      ENTRY("BM_CLASH_DETECT",            4,  PropPage::instance->battleModeClash, CHECKER(def) { return ISFETT && def->getState(); }, true),
      ENTRY("BM_DISABLE_OFF_BUTTON",      false,  PropPage::instance->battleModeDisablePWR, CHECKER(def) { return ISFETT && def->getState(); }, true),
  };

# undef ENTRY
# undef CHECKER
# undef ISFETT
# undef ISSA22C
# undef ISBC
# undef ISCAIWYN
# undef PRSETTING
}

void Settings::setCustomInputParsers() {
  generalDefines["NUM_BLADES"]->overrideParser ([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    Configuration::instance->numBlades = std::stoi(key.second);
    return true;
  });
  generalDefines["SAVE_STATE"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    GeneralPage::instance->colorSave->SetValue(true);
    GeneralPage::instance->presetSave->SetValue(true);
    GeneralPage::instance->volumeSave->SetValue(true);
    return true;
  });
  generalDefines["BLADE_DETECT_PIN"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    BladeIDPage::instance->enableDetect->SetValue(true);
    BladeIDPage::instance->detectPin->SetValue(key.second);
    return false;
  });
  generalDefines["BLADE_ID_CLASS"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    BladeIDPage::instance->enableID->SetValue(true);
    key.second = std::strtok(key.second.data(), "< ");
    if (key.second == "SnapshotBladeID") {
      BladeIDPage::instance->mode->SetValue(BLADE_ID_MODE_SNAPSHOT);
      BladeIDPage::instance->IDPin->entry->SetValue(std::strtok(nullptr, "<> "));
    } else if (key.second == "ExternalPullupBladeID") {
      BladeIDPage::instance->mode->SetValue(BLADE_ID_MODE_EXTERNAL);
      BladeIDPage::instance->IDPin->entry->SetValue(std::strtok(nullptr, "<, "));
      BladeIDPage::instance->pullupResistance->num->SetValue(std::stod(std::strtok(nullptr, ",> ")));
    } else if (key.second == "BridgedPullupBladeID") {
      BladeIDPage::instance->mode->SetValue(BLADE_ID_MODE_BRIDGED);
      BladeIDPage::instance->IDPin->entry->SetValue(std::strtok(nullptr, "<, "));
      BladeIDPage::instance->pullupPin->entry->SetValue(std::strtok(nullptr, ",> "));
    }
    return true;
  });
  generalDefines["BLADE_ID_SCAN_MILLIS"]->overrideParser([](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    BladeIDPage::instance->scanIDMillis->SetValue(std::stoi(key.second));
    BladeIDPage::instance->continuousScans->SetValue(true);
  });
  generalDefines["BLADE_ID_TIMES"]->overrideParser([](const ProffieDefine* def, const std::string& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    BladeIDPage::instance->numIDTimes->SetValue(std::stoi(key.second));
    BladeIDPage::instance->continuousScans->SetValue(true);
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    BladeIDPage::instance->enablePowerForID->SetValue(true);
    std::strtok(key.second.data(), "<");
    char* pwrPinTest = std::strtok(nullptr, "<>, ");
    while (pwrPinTest != nullptr) {
      key.second = pwrPinTest;
      if (key.second == "bladePowerPin1") BladeIDPage::instance->powerPin1->SetValue(true);
      if (key.second == "bladePowerPin2") BladeIDPage::instance->powerPin2->SetValue(true);
      if (key.second == "bladePowerPin3") BladeIDPage::instance->powerPin3->SetValue(true);
      if (key.second == "bladePowerPin4") BladeIDPage::instance->powerPin4->SetValue(true);
      if (key.second == "bladePowerPin5") BladeIDPage::instance->powerPin5->SetValue(true);
      if (key.second == "bladePowerPin6") BladeIDPage::instance->powerPin6->SetValue(true);

      pwrPinTest = std::strtok(nullptr, "<>, ");
    }
    return true;
  });
  propDefines["FORCE_PUSH"]->overrideParser([](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first.find(def->getName()) == std::string::npos) return false;

    if (key.first.find("FETT263") != std::string::npos) PropPage::instance->forcePushBM->SetValue(true);
    else PropPage::instance->forcePush->SetValue(true);

    return true;
  });
}
void Settings::setCustomOutputParsers() {
  generalDefines["NUM_BLADES"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    int32_t numBlades = 0;
    for (const BladesPage::BladeConfig& blade : BladeIDPage::instance->bladeArrays[BladesPage::instance->bladeArray->GetSelection()].blades) numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
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
    auto mode = BladeIDPage::instance->mode->GetValue();
    std::string returnVal = def->getName() + " ";
    if (mode == BLADE_ID_MODE_SNAPSHOT) returnVal + "SnapshotBladeID<" + BladeIDPage::instance->IDPin->entry->GetValue() + ">";
    else if (mode == BLADE_ID_MODE_BRIDGED) returnVal + "ExternalPullupBladeID<" + BladeIDPage::instance->IDPin->entry->GetValue() + ", " + BladeIDPage::instance->pullupResistance->num->GetTextValue() + ">";
    else if (mode == BLADE_ID_MODE_EXTERNAL) returnVal + "BridgedPullupBladeID<" + BladeIDPage::instance->IDPin->entry->GetValue() + ", " + BladeIDPage::instance->pullupPin->entry->GetValue();

    return returnVal;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideOutput([](const ProffieDefine* def) -> std::string {
    std::string returnVal = def->getName() + " PowerPINS<";
    std::vector<std::string> powerPins;
    if (BladeIDPage::instance->powerPin1->GetValue()) powerPins.push_back("bladePowerPin1");
    if (BladeIDPage::instance->powerPin2->GetValue()) powerPins.push_back("bladePowerPin2");
    if (BladeIDPage::instance->powerPin3->GetValue()) powerPins.push_back("bladePowerPin3");
    if (BladeIDPage::instance->powerPin4->GetValue()) powerPins.push_back("bladePowerPin4");
    if (BladeIDPage::instance->powerPin5->GetValue()) powerPins.push_back("bladePowerPin5");
    if (BladeIDPage::instance->powerPin6->GetValue()) powerPins.push_back("bladePowerPin6");

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
  for (const auto& [key, defObj] : allDefines) {
    for (const std::string& entry : _defList) {
      if (defObj->parseDefine(entry)) break;
    }
  }
}

void Settings::loadDefaults() {
  for (const auto& [key, defObj] : allDefines) {
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

std::pair<std::string, std::string> Settings::ProffieDefine::parseKey(const std::string& _input) {
  std::pair<std::string, std::string> key;
  std::string parseVal = _input;

  key.first = std::strtok(&parseVal[0], " ");
  // Handle trying to construct from nullptr
  char* val = std::strtok(nullptr, " \n\r");
  if (val != nullptr) key.second = val;

  return key;
}
