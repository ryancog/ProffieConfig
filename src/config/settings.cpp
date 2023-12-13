// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "config/settings.h"

#include "config/configuration.h"
#include "pages/generalpage.h"

#include <cstring>

Settings::Settings() {
  instance = this;
}

void Settings::linkDefines() {
# define ENTRY ProffieDefine::createProffieDefine

  defines = {
      ENTRY("NUM_BLADES", 1, nullptr),
      ENTRY("NUM_BUTTONS", 2, GeneralPage::instance->buttons),
      ENTRY("VOLUME", 2000, GeneralPage::instance->volume),
      ENTRY("CLASH_THRESHOLD_G", 3.0, GeneralPage::instance->clash),
      ENTRY("SAVE_STATE", false, nullptr),
  };

# undef ENTRY
}

void Settings::setCustomDefineParsers() {
  defines["NUM_BLADES"]->overrideParser ([](const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first == "NUM_BLADES") {
      Configuration::instance->numBlades = std::stoi(key.second);
      return true;
    }

    return false;
  });

  defines["SAVE_STATE"]->overrideParser([](const std::string& input) -> bool {
    auto key = ProffieDefine::parseKey(input);
    if (key.first == "SAVE_STATE") {
      GeneralPage::instance->colorSave->SetValue(true);
      GeneralPage::instance->presetSave->SetValue(true);
      GeneralPage::instance->volumeSave->SetValue(true);
      return true;
    }

    return false;
  });
}


std::pair<std::string, Settings::ProffieDefine*> Settings::ProffieDefine::createProffieDefine(std::string _name, int32_t _defaultValue, Misc::numEntry* _element, bool _loose) {
  auto define = new ProffieDefine();
  define->identifier = _name;
  define->defaultValue.num = _defaultValue;

  define->element = _element;
  define->type = Type::NUMERIC;


  define->parseDefine = [&](const std::string& input) -> bool {
    auto key = parseKey(input);
    if (_loose ? std::strstr(key.first.c_str(), define->identifier.c_str()) != nullptr : key.first == define->identifier) {
      static_cast<Misc::numEntry*>(define->element)->SetValue(std::stoi(key.second));
      return true;
    }

    return false;
  };

  return { _name, define };
}

std::pair<std::string, Settings::ProffieDefine*> Settings::ProffieDefine::createProffieDefine(std::string _name, double _defaultValue, Misc::numEntryDouble* _element, bool _loose) {
  auto define = new ProffieDefine();
  define->identifier = _name;
  define->defaultValue.dec = _defaultValue;

  define->element = _element;
  define->type = Type::DECIMAL;

  define->parseDefine = [&](const std::string& input) -> bool {
    auto key = parseKey(input);
    if (_loose ? std::strstr(key.first.c_str(), define->identifier.c_str()) != nullptr : key.first == define->identifier) {
      static_cast<Misc::numEntryDouble*>(define->element)->SetValue(std::stod(key.second));
      return true;
    }

    return false;
  };

  return { _name, define };
}

std::pair<std::string, Settings::ProffieDefine*> Settings::ProffieDefine::createProffieDefine(std::string _name, bool _defaultState, wxCheckBox* _element, bool _loose) {
  auto define = new ProffieDefine();
  define->identifier = _name;
  define->defaultValue.state = _defaultState;

  define->element = _element;
  define->type = Type::STATE;

  define->parseDefine = [&](const std::string& input) -> bool {
    auto key = parseKey(input);
    if (_loose ? std::strstr(key.first.c_str(), define->identifier.c_str()) != nullptr : key.first == define->identifier) {
      static_cast<wxCheckBox*>(define->element)->SetValue(true);
      return true;
    }

    return false;
  };

  return { _name, define };
}

std::pair<std::string, Settings::ProffieDefine*> Settings::ProffieDefine::createProffieDefine(std::string _name, wxString _defaultSelection, wxComboBox* _element, bool _loose) {
  auto define = new ProffieDefine();
  define->identifier = _name;
  strncpy(define->defaultValue.str, _defaultSelection.ToStdString().data(), _defaultSelection.length());

  define->element = _element;
  define->type = Type::COMBO;

  define->parseDefine = [&](const std::string& input) -> bool {
    auto key = parseKey(input);

    if (_loose ? std::strstr(key.first.c_str(), define->identifier.c_str()) != nullptr : key.first == define->identifier) {
      static_cast<wxComboBox*>(define->element)->SetValue(key.second);
      return true;
    }

    return false;
  };

  return { _name, define };
}

inline void Settings::ProffieDefine::overrideParser(std::function<bool(const std::string&)> _parser) { parseDefine = _parser; }

void Settings::ProffieDefine::loadDefault() {
  if (element == nullptr) return;

  switch (type) {
    case Type::STATE:
      static_cast<wxCheckBox*>(element)->SetValue(defaultValue.state);
      break;
    case Type::NUMERIC:
      static_cast<Misc::numEntry*>(element)->SetValue(defaultValue.num);
      break;
    case Type::DECIMAL:
      static_cast<Misc::numEntryDouble*>(element)->SetValue(defaultValue.dec);
      break;
    case Type::COMBO:
      static_cast<wxComboBox*>(element)->SetValue(defaultValue.str);
      break;
  }
}

std::pair<std::string, std::string> Settings::ProffieDefine::parseKey(const std::string& _input) {
  std::pair<std::string, std::string> key;
  std::string parseVal = _input;

  key.first = std::strtok(&parseVal[0], " ");
  try { // Handle trying to construct from nullptr
    key.second = std::strtok(nullptr, " \n\r");
  } catch (std::exception&) {
    key.second = "";
  }

  return key;
}
