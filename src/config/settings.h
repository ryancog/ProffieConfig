// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include <cstdint>
#include <wx/wx.h>

#include "elements/misc.h"

class Settings {
public:
  Settings();
  static Settings* instance;

  void linkDefines();
  void setCustomDefineParsers();

  class ProffieDefine;
  std::unordered_map<std::string, ProffieDefine*> defines;

private:
};

class Settings::ProffieDefine {
public:
  static std::pair<std::string, ProffieDefine*> createProffieDefine(std::string name, int32_t defaultValue, Misc::numEntry* element, bool loose = false);
  static std::pair<std::string, ProffieDefine*> createProffieDefine(std::string name, double defaultValue, Misc::numEntryDouble* element, bool loose = false);
  static std::pair<std::string, ProffieDefine*> createProffieDefine(std::string name, bool defaultState, wxCheckBox* element, bool loose = false);
  static std::pair<std::string, ProffieDefine*> createProffieDefine(std::string name, wxString defaultSelection, wxComboBox* element, bool loose = false);

  static std::pair<std::string, std::string> parseKey(const std::string&);

  inline void overrideParser(std::function<bool(const std::string&)> newParser);
  void loadDefault();

private:
  ProffieDefine();

  enum class Type {
    STATE,
    NUMERIC,
    DECIMAL,
    COMBO
  };

  union {
    int32_t num;
    double dec;
    bool state;
    char* str;
  } defaultValue;

  Type type;
  std::string identifier;
  void* element;

  std::function<bool(const std::string&)> parseDefine;
};
