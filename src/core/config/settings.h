// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "core/utilities/misc.h"
#include "editor/editorwindow.h"

#include <cstdint>
#include <cstring>
#include <wx/wx.h>

#define PDEF_DEFAULT_CHECK [](const ProffieDefine* def) -> bool { return def->getState(); }

class Settings {
public:
  Settings(EditorWindow*);

  void loadDefaults();
  void parseDefines(const std::vector<std::string>&);

  class ProffieDefine;
  std::unordered_map<std::string, ProffieDefine*> generalDefines;
  std::vector<std::string> readDefines;
  int32_t numBlades;

private:
  EditorWindow* parent{nullptr};

  void linkDefines();
  void setCustomInputParsers();
  void setCustomOutputParsers();
};

class Settings::ProffieDefine {
private:
  enum class Type {
    STATE,
    RADIO,
    NUMERIC,
    DECIMAL,
    COMBO,
    TEXT
  };
  const Type type;
  const bool looseChecking;

  const union {
    int32_t num;
    double dec;
    bool state;
    char* str;
  } defaultValue;

  const std::string identifier;
  const void* element;

public:

  ProffieDefine(std::string name, int32_t defaultValue, Misc::numEntry* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
  ProffieDefine(std::string name, double defaultValue, Misc::numEntryDouble* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
  ProffieDefine(std::string name, bool defaultState, wxCheckBox* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
  ProffieDefine(std::string name, bool defaultState, wxRadioButton* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
  ProffieDefine(std::string name, wxString defaultSelection, wxComboBox* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
  ProffieDefine(std::string name, wxString defaultEntry, Misc::textEntry* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);

  static std::pair<std::string, std::string> parseKey(const std::string&);

  std::function<bool(const ProffieDefine*, const std::string&)> parse = [](const ProffieDefine* def, const std::string& input) -> bool {
    auto key = parseKey(input);

    if (def->looseChecking ? std::strstr(key.first.c_str(), def->identifier.c_str()) == nullptr : key.first != def->identifier) return false;

    switch (def->type) {
      case Type::STATE:
        const_cast<wxCheckBox*>(static_cast<const wxCheckBox*>(def->element))->SetValue(true);
        break;
      case Type::RADIO:
        const_cast<wxRadioButton*>(static_cast<const wxRadioButton*>(def->element))->SetValue(true);
        break;
      case Type::NUMERIC:
        const_cast<Misc::numEntry*>(static_cast<const Misc::numEntry*>(def->element))->SetValue(stoi(key.second));
        break;
      case Type::DECIMAL:
        const_cast<Misc::numEntryDouble*>(static_cast<const Misc::numEntryDouble*>(def->element))->SetValue(stod(key.second));
        break;
      case Type::COMBO:
        const_cast<Misc::comboBoxEntry*>(static_cast<const Misc::comboBoxEntry*>(def->element))->SetValue(key.second);
        break;
      case Type::TEXT:
        const_cast<Misc::textEntry*>(static_cast<const Misc::textEntry*>(def->element))->SetValue(key.second);
        break;
    }

    return true;
  };
  std::function<std::string(const ProffieDefine*)> output = [](const ProffieDefine* def) -> std::string {
    switch (def->type) {
      case Type::NUMERIC:
        return def->identifier + " " + std::to_string(def->getNum());
      case Type::DECIMAL:
        return def->identifier + " " + std::to_string(def->getDec());
      case Type::COMBO:
      case Type::TEXT:
        return def->identifier + " " + def->getString();
      case Type::STATE:
      case Type::RADIO:
      default:
        return def->identifier;
    }
  };
  std::function<bool(const ProffieDefine*)> checkOutput;

  std::string getOutput() const { return output(this); }
  bool parseDefine(const std::string& input) const { return parse(this, input); }

  std::string getName() const { return identifier; }
  bool shouldOutput() const { return checkOutput(this); }
  int32_t getNum() const;
  double getDec() const;
  bool getState() const;
  std::string getString() const;

  inline void overrideParser(std::function<bool(const ProffieDefine*, const std::string&)> _newParser) { parse = _newParser; }
  inline void overrideOutput(std::function<std::string(const ProffieDefine*)> _newOutput) { output = _newOutput; }

  void loadDefault();
};
