// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "ui/pcspinctrl.h"
#include "ui/pcspinctrldouble.h"
#include "ui/pctextctrl.h"

#include <cstdint>
#include <cstring>
#include <wx/checkbox.h>
#include <wx/radiobut.h>

#define PDEF_DEFAULT_CHECK [](const ProffieDefine* def) -> bool { return def->getState(); }

  class Settings {
public:
  Settings(EditorWindow*);

  void parseDefines(std::vector<std::string>&);

  class ProffieDefine;
  std::unordered_map<std::string, ProffieDefine*> generalDefines{};
  std::vector<std::string> readDefines{};
  int32_t numBlades{0};

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
  } const type{Type::STATE};
  const bool looseChecking{false};

  const union {
    int32_t num;
    double dec;
    bool state;
    char* str;
  } defaultValue{0};

  const std::string identifier{};
  const void* element{nullptr};

public:

  ProffieDefine(std::string name, int32_t defaultValue, pcSpinCtrl* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
  ProffieDefine(std::string name, double defaultValue, pcSpinCtrlDouble* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
  ProffieDefine(std::string name, bool defaultState, wxCheckBox* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
  ProffieDefine(std::string name, bool defaultState, wxRadioButton* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
  ProffieDefine(std::string name, wxString defaultSelection, pcComboBox* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
  ProffieDefine(std::string name, wxString defaultEntry, pcTextCtrl* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);

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
      const_cast<pcSpinCtrl*>(static_cast<const pcSpinCtrl*>(def->element))->entry()->SetValue(stoi(key.second));
      break;
    case Type::DECIMAL:
      const_cast<pcSpinCtrlDouble*>(static_cast<const pcSpinCtrlDouble*>(def->element))->entry()->SetValue(stod(key.second));
      break;
    case Type::COMBO:
      const_cast<pcComboBox*>(static_cast<const pcComboBox*>(def->element))->entry()->SetValue(key.second);
      break;
    case Type::TEXT:
      const_cast<pcTextCtrl*>(static_cast<const pcTextCtrl*>(def->element))->entry()->SetValue(key.second);
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
};
