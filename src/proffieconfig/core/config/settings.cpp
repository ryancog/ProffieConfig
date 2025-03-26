#include "settings.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek


#include "../../core/config/configuration.h"
#include "../../editor/editorwindow.h"
#include "../../editor/pages/generalpage.h"
#include "../../editor/dialogs/bladearraydlg.h"

#include <cstring>
#include <utility>

Settings::Settings(EditorWindow* _parent) : mParent(_parent) {
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
#   define IDSETTING(setting) mParent->bladesPage->bladeArrayDlg->setting->GetValue()

    generalDefines = {
        // General
        ENTRY("NUM_BLADES", static_cast<PCUI::Numeric *>(nullptr), CHECKER(){ return true; }),
        ENTRY("NUM_BUTTONS", mParent->generalPage->buttons, CHECKER(){ return true; }),
        ENTRY("VOLUME", mParent->generalPage->volume, CHECKER(){ return true; }),
        ENTRY("CLASH_THRESHOLD_G", mParent->generalPage->clash, CHECKER(){ return true; }),
        ENTRY("SAVE_COLOR_CHANGE", mParent->generalPage->colorSave),
        ENTRY("SAVE_PRESET", mParent->generalPage->presetSave),
        ENTRY("SAVE_VOLUME", mParent->generalPage->volumeSave),
        ENTRY("SAVE_STATE", static_cast<wxCheckBox*>(nullptr), CHECKER(){ return false; }),

        ENTRY("ENABLE_SSD1306", mParent->generalPage->enableOLED),

        ENTRY("DISABLE_COLOR_CHANGE", mParent->generalPage->disableColor),
        ENTRY("DISABLE_TALKIE", mParent->generalPage->noTalkie),
        ENTRY("DISABLE_BASIC_PARSER_STYLES", mParent->generalPage->noBasicParsers),
        ENTRY("DISABLE_DIAGNOSTIC_COMMANDS", mParent->generalPage->disableDiagnosticCommands),

        ENTRY("ORIENTATION", mParent->generalPage->orientation, CHECKER(){ return true; }),
        ENTRY("PLI_OFF_TIME", mParent->generalPage->pliTime, CHECKER(){ return true; }),
        ENTRY("IDLE_OFF_TIME", mParent->generalPage->idleTime, CHECKER(){ return true; }),
        ENTRY("MOTION_TIMEOUT", mParent->generalPage->motionTime, CHECKER(){ return true; }),

        ENTRY("BLADE_DETECT_PIN", mParent->bladesPage->bladeArrayDlg->detectPin, CHECKER(){ return IDSETTING(enableDetect); }),
        ENTRY("BLADE_ID_CLASS", mParent->bladesPage->bladeArrayDlg->mode, CHECKER(){ return IDSETTING(enableID); }),
        ENTRY("ENABLE_POWER_FOR_ID", mParent->bladesPage->bladeArrayDlg->enablePowerForID, CHECKER(def){ return IDSETTING(enableID) && def->getState(); }),
        ENTRY("BLADE_ID_SCAN_MILLIS", mParent->bladesPage->bladeArrayDlg->scanIDMillis, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
        ENTRY("BLADE_ID_TIMES", mParent->bladesPage->bladeArrayDlg->numIDTimes, CHECKER(){ return IDSETTING(enableID) && IDSETTING(continuousScans); }),
    };

#   undef ENTRY
#   undef CHECKER
#   undef IDSETTING
}

void Settings::setCustomInputParsers() {
    auto numBladesParser{[this](const ProffieDefine* def, const wxString& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    long tmpVal{};
    key.second.ToCLong(&tmpVal);
    numBlades = tmpVal;
    return true;
  }};
  generalDefines["NUM_BLADES"]->overrideParser(numBladesParser);

  auto saveStateParser{[this](const ProffieDefine* def, const wxString& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    mParent->generalPage->colorSave->SetValue(true);
    mParent->generalPage->presetSave->SetValue(true);
    mParent->generalPage->volumeSave->SetValue(true);
    return true;
  }};
  generalDefines["SAVE_STATE"]->overrideParser(saveStateParser);

  auto orientParser{[&](const ProffieDefine* def, const wxString& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;

    mParent->generalPage->orientation->entry()->SetStringSelection(Configuration::findInVMap(Configuration::ORIENTATION, key.second).first);
    return true;
  }};
  generalDefines["ORIENTATION"]->overrideParser(orientParser);

  auto bladeDetectPinParser{[&](const ProffieDefine* def, const wxString& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    mParent->bladesPage->bladeArrayDlg->enableDetect->SetValue(true);
    mParent->bladesPage->bladeArrayDlg->detectPin->entry()->SetValue(key.second);
    return true;
  }};
  generalDefines["BLADE_DETECT_PIN"]->overrideParser(bladeDetectPinParser);

  auto bladeIDClassParser{[&](const ProffieDefine* def, const wxString& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    mParent->bladesPage->bladeArrayDlg->enableID->SetValue(true);
    const auto typeEnd{key.second.find('<')};
    const auto type{key.second.substr(0, typeEnd)};
    if (type == "SnapshotBladeID") {
      mParent->bladesPage->bladeArrayDlg->mode->entry()->SetSelection(BLADE_ID_MODE_SNAPSHOT);
      const auto pinBegin{key.second.find_first_not_of("< ", typeEnd)};
      const auto pinEnd{key.second.find_first_of(" >", pinBegin)};
      mParent->bladesPage->bladeArrayDlg->IDPin->entry()->SetValue(key.second.substr(pinBegin, pinEnd - pinBegin));
    } else if (type == "ExternalPullupBladeID") {
      mParent->bladesPage->bladeArrayDlg->mode->entry()->SetSelection(BLADE_ID_MODE_EXTERNAL);
      const auto pinBegin{key.second.find_first_not_of("< ", typeEnd)};
      const auto pinEnd{key.second.find_first_of(", >", pinBegin)};
      mParent->bladesPage->bladeArrayDlg->IDPin->entry()->SetValue(key.second.substr(pinBegin, pinEnd - pinBegin));
      const auto resBegin{key.second.find_first_not_of(", ", pinEnd)};
      const auto resEnd{key.second.find_first_of(" >", resBegin)};
      mParent->bladesPage->bladeArrayDlg->pullupResistance->entry()->SetValue(key.second.substr(resBegin, resEnd - resBegin));
    } else if (type == "BridgedPullupBladeID") {
      mParent->bladesPage->bladeArrayDlg->mode->entry()->SetSelection(BLADE_ID_MODE_BRIDGED);
      const auto pinBegin{key.second.find_first_not_of("< ", typeEnd)};
      const auto pinEnd{key.second.find_first_of(", >", pinBegin)};
      mParent->bladesPage->bladeArrayDlg->IDPin->entry()->SetValue(key.second.substr(pinBegin, pinEnd - pinBegin));
      const auto pullupPinBegin{key.second.find_first_not_of(", ", pinEnd)};
      const auto pullupPinEnd{key.second.find_first_of(" >", pullupPinBegin)};
      mParent->bladesPage->bladeArrayDlg->pullupPin->entry()->SetValue(key.second.substr(pullupPinBegin, pullupPinEnd - pullupPinBegin));
    }
    return true;
  }};
  generalDefines["BLADE_ID_CLASS"]->overrideParser(bladeIDClassParser);

  auto bladeIDScanMillisParser{[&](const ProffieDefine* def, const wxString& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    mParent->bladesPage->bladeArrayDlg->scanIDMillis->entry()->SetValue(key.second);
    mParent->bladesPage->bladeArrayDlg->continuousScans->SetValue(true);
    return true;
  }};
  generalDefines["BLADE_ID_SCAN_MILLIS"]->overrideParser(bladeIDScanMillisParser);

  auto bladeIDTimesParser{[&](const ProffieDefine* def, const wxString& input) ->bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    mParent->bladesPage->bladeArrayDlg->numIDTimes->entry()->SetValue(key.second);
    mParent->bladesPage->bladeArrayDlg->continuousScans->SetValue(true);
    return true;
  }};
  generalDefines["BLADE_ID_TIMES"]->overrideParser(bladeIDTimesParser);

  auto enablePowerForIDParser{[&](const ProffieDefine* def, const wxString& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first != def->getName()) return false;
    
    mParent->bladesPage->bladeArrayDlg->enablePowerForID->SetValue(true);
    if (key.second.find("bladePowerPin1") != string::npos) mParent->bladesPage->bladeArrayDlg->powerPin1->SetValue(true);
    if (key.second.find("bladePowerPin2") != string::npos) mParent->bladesPage->bladeArrayDlg->powerPin2->SetValue(true);
    if (key.second.find("bladePowerPin3") != string::npos) mParent->bladesPage->bladeArrayDlg->powerPin3->SetValue(true);
    if (key.second.find("bladePowerPin4") != string::npos) mParent->bladesPage->bladeArrayDlg->powerPin4->SetValue(true);
    if (key.second.find("bladePowerPin5") != string::npos) mParent->bladesPage->bladeArrayDlg->powerPin5->SetValue(true);
    if (key.second.find("bladePowerPin6") != string::npos) mParent->bladesPage->bladeArrayDlg->powerPin6->SetValue(true);

    return true;
  }};
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideParser(enablePowerForIDParser);

  auto timeoutParser{[&](const ProffieDefine* def, const wxString& input) -> bool {
      auto key = ProffieDefine::parseKey(input);
      if (key.first != def->getName()) return false;

      uint64 searchPos{0};
      int32 timeout{0};
      while (not false) {
          auto valPos{key.second.find_first_not_of("\t *()", searchPos)};
          if (valPos == string::npos) break;

          long tmpVal{};
          if (key.second.substr(valPos).ToCLong(&tmpVal)) {
              if (timeout == 0) timeout = 1;
              timeout *= tmpVal;
          }
          searchPos = key.second.find_first_of("\t *()", valPos);
      }
      timeout /= 60 * 1000;

      if (def->getName() == "IDLE_OFF_TIME") {
          mParent->generalPage->idleTime->entry()->SetValue(timeout);
      } else if (def->getName() == "PLI_OFF_TIME") {
          mParent->generalPage->pliTime->entry()->SetValue(timeout);
      } else if (def->getName() == "MOTION_TIMEOUT") {
          mParent->generalPage->motionTime->entry()->SetValue(timeout);
      }

      return true;
  }};

  generalDefines["IDLE_OFF_TIME"]->overrideParser(timeoutParser);
  generalDefines["PLI_OFF_TIME"]->overrideParser(timeoutParser);
  generalDefines["MOTION_TIMEOUT"]->overrideParser(timeoutParser);
}
void Settings::setCustomOutputParsers() {
  generalDefines["NUM_BLADES"]->overrideOutput([&](const ProffieDefine* def) -> wxString {
    int32 numBlades{0};
    for (const BladesPage::BladeConfig& blade : mParent->bladesPage->bladeArrayDlg->bladeArrays[mParent->bladesPage->bladeArray->entry()->GetSelection()].blades) {
        numBlades += blade.subBlades.size() > 0 ? static_cast<int32>(blade.subBlades.size()) : 1;
    }
    return def->getName() + " " + std::to_string(numBlades);
  });
  generalDefines["PLI_OFF_TIME"]->overrideOutput([](const ProffieDefine* def) -> wxString {
    return def->getName() + " " + std::to_string(def->getNum()) + " * 60 * 1000";
  });
  generalDefines["IDLE_OFF_TIME"]->overrideOutput([](const ProffieDefine* def) -> wxString {
    return def->getName() + " " + std::to_string(def->getNum()) + " * 60 * 1000";
  });
  generalDefines["MOTION_TIMEOUT"]->overrideOutput([](const ProffieDefine* def) -> wxString {
    return def->getName() + " " + std::to_string(def->getNum()) + " * 60 * 1000";
  });

  auto orientOutput{[&](const ProffieDefine* def) -> wxString {
      return {def->getName() + " " + Configuration::findInVMap(Configuration::ORIENTATION, def->getString()).second};
  }};
  generalDefines["ORIENTATION"]->overrideOutput(orientOutput);

  generalDefines["BLADE_ID_CLASS"]->overrideOutput([&](const ProffieDefine* def) -> wxString {
    const auto mode = mParent->bladesPage->bladeArrayDlg->mode->entry()->GetSelection();
    wxString returnVal = def->getName() + " ";
    if (mode == BLADE_ID_MODE_SNAPSHOT) {
        returnVal += "SnapshotBladeID<" + mParent->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue().ToStdString() + ">";
    } else if (mode == BLADE_ID_MODE_BRIDGED) {
        returnVal += 
            "BridgedPullupBladeID<" + 
            mParent->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue().ToStdString() + 
            ", " + 
            mParent->bladesPage->bladeArrayDlg->pullupPin->entry()->GetValue().ToStdString() +
            ">";
    } else if (mode == BLADE_ID_MODE_EXTERNAL) {
        returnVal +=
            "ExternalPullupBladeID<" + 
            mParent->bladesPage->bladeArrayDlg->IDPin->entry()->GetValue().ToStdString() + 
            ", " + 
            std::to_string(mParent->bladesPage->bladeArrayDlg->pullupResistance->entry()->GetValue()) + 
            ">";
    }

    return returnVal;
  });
  generalDefines["ENABLE_POWER_FOR_ID"]->overrideOutput([&](const ProffieDefine* def) -> wxString {
    wxString returnVal = def->getName() + " PowerPINS<";
    std::vector<wxString> powerPins;
    if (mParent->bladesPage->bladeArrayDlg->powerPin1->GetValue()) powerPins.emplace_back("bladePowerPin1");
    if (mParent->bladesPage->bladeArrayDlg->powerPin2->GetValue()) powerPins.emplace_back("bladePowerPin2");
    if (mParent->bladesPage->bladeArrayDlg->powerPin3->GetValue()) powerPins.emplace_back("bladePowerPin3");
    if (mParent->bladesPage->bladeArrayDlg->powerPin4->GetValue()) powerPins.emplace_back("bladePowerPin4");
    if (mParent->bladesPage->bladeArrayDlg->powerPin5->GetValue()) powerPins.emplace_back("bladePowerPin5");
    if (mParent->bladesPage->bladeArrayDlg->powerPin6->GetValue()) powerPins.emplace_back("bladePowerPin6");

    for (int32_t pin = 0; pin < static_cast<int32_t>(powerPins.size()); pin++) {
      returnVal += powerPins.at(pin);
      if (pin < static_cast<int32_t>(powerPins.size()) -1) returnVal += ",";
    }

    returnVal += ">";
    return returnVal;
  });
}

void Settings::parseDefines(std::vector<wxString>& _defList) {
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
  if (mType != Type::NUMERIC) return 0;
  return const_cast<PCUI::Numeric *>(static_cast<const PCUI::Numeric*>(mElement))->entry()->GetValue();
}
double Settings::ProffieDefine::getDec() const {
  if (mType != Type::DECIMAL) return 0;
  return const_cast<PCUI::NumericDec*>(static_cast<const PCUI::NumericDec*>(mElement))->entry()->GetValue();
}
bool Settings::ProffieDefine::getState() const {
  if (mType == Type::STATE) return const_cast<wxCheckBox*>(static_cast<const wxCheckBox*>(mElement))->GetValue();
  if (mType == Type::RADIO) return const_cast<wxRadioButton*>(static_cast<const wxRadioButton*>(mElement))->GetValue();

  return false;
}
wxString Settings::ProffieDefine::getString() const {
  if (mType == Type::TEXT) return const_cast<PCUI::Text *>(static_cast<const PCUI::Text *>(mElement))->entry()->GetValue().ToStdString();
  if (mType == Type::COMBO) return const_cast<PCUI::Choice *>(static_cast<const PCUI::Choice *>(mElement))->entry()->GetStringSelection().ToStdString();

  return "";
}

Settings::ProffieDefine::ProffieDefine(wxString _name, PCUI::Numeric* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) :
    mType(Type::NUMERIC), mLooseChecking(_loose), mIdentifier(std::move(_name)), mElement(_element), checkOutput(std::move(_check)) {}

Settings::ProffieDefine::ProffieDefine(wxString _name, PCUI::NumericDec* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) : 
    mType(Type::DECIMAL), mLooseChecking(_loose), mIdentifier(std::move(_name)), mElement(_element), checkOutput(std::move(_check)) {}

Settings::ProffieDefine::ProffieDefine(wxString _name, wxCheckBox* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) : 
    mLooseChecking(_loose), mIdentifier(std::move(_name)), mElement(_element), checkOutput(std::move(_check)) {}

Settings::ProffieDefine::ProffieDefine(wxString _name, wxRadioButton* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) : 
    mType(Type::RADIO), mLooseChecking(_loose), mIdentifier(std::move(_name)), mElement(_element), checkOutput(std::move(_check)) {}

Settings::ProffieDefine::ProffieDefine(wxString _name, PCUI::Choice* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) : 
    mType(Type::COMBO), mLooseChecking(_loose), mIdentifier(std::move(_name)), mElement(_element), checkOutput(std::move(_check)) {}

Settings::ProffieDefine::ProffieDefine(wxString _name, PCUI::Text* _element, std::function<bool(const ProffieDefine*)> _check, bool _loose) : 
    mType(Type::TEXT), mLooseChecking(_loose), mIdentifier(std::move(_name)), mElement(_element), checkOutput(std::move(_check)) {}

std::pair<wxString, wxString> Settings::ProffieDefine::parseKey(const wxString& _input) {
    std::pair<wxString, wxString> key;

    constexpr cstring DELIMITER{" \n\r"};
    auto keyPos{_input.find_first_not_of(DELIMITER)};
    if (keyPos == string::npos) return key;
    auto keyEnd{_input.find_first_of(DELIMITER, keyPos)};
    key.first = _input.substr(keyPos, keyEnd - keyPos);
    if (keyEnd == string::npos) return key;

    auto valuePos{_input.find_first_not_of(DELIMITER, keyEnd)};
    if (valuePos == string::npos) return key;
    auto valueEnd{_input.find_last_not_of(DELIMITER)};
    key.second = _input.substr(valuePos, valueEnd - valuePos + 1);
    
    return key;
}
