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

  class ProffieDefine;

private:
};

class Settings::ProffieDefine {
public:
  ProffieDefine(std::string name, int32_t defaultValue, Misc::numEntry* element, bool loose);
  ProffieDefine(std::string name, double defaultValue, Misc::numEntryDouble* element, bool loose);
  ProffieDefine(std::string name, bool defaultState, wxCheckBox* element, bool loose);
  ProffieDefine(std::string name, wxString defaultSelection, wxComboBox* element, bool loose);

  inline void overrideParser(std::function<bool(std::string&)> newParser);
  inline void* getElementReference();


private:
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

  std::function<bool(std::string&)> parseDefine;
};
