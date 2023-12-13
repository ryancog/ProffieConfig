#include "settings.h"

#include <cstring>




Settings::ProffieDefine::ProffieDefine(std::string _name, int32_t _defaultValue, Misc::numEntry* _element, bool loose = false) {
  identifier = _name;
  defaultValue.num = _defaultValue;

  element = _element;
  type = Type::NUMERIC;


  parseDefine = [&](std::string& input) -> bool {
    input = std::strtok(&input[0], " ");
    if (loose ? std::strstr(input.c_str(), identifier.c_str()) != nullptr : input == identifier) {
      static_cast<Misc::numEntry*>(element)->SetValue(std::stoi(std::strtok(nullptr, " \n\r")));
    }

    return false;
  };
}

Settings::ProffieDefine::ProffieDefine(std::string _name, double _defaultValue, Misc::numEntryDouble* _element, bool loose = false) {
  identifier = _name;
  defaultValue.dec = _defaultValue;

  element = _element;
  type = Type::DECIMAL;

  parseDefine = [&](std::string& input) -> bool {
    input = std::strtok(&input[0], " ");
    if (loose ? std::strstr(input.c_str(), identifier.c_str()) != nullptr : input == identifier) {
      static_cast<Misc::numEntryDouble*>(element)->SetValue(std::stod(std::strtok(nullptr, " \n\r")));
    }

    return false;
  };
}

Settings::ProffieDefine::ProffieDefine(std::string _name, bool _defaultState, wxCheckBox* _element, bool loose = false) {
  identifier = _name;
  defaultValue.state = _defaultState;

  element = _element;
  type = Type::STATE;

  parseDefine = [&](std::string& input) -> bool {
    input = std::strtok(&input[0], " ");
    if (loose ? std::strstr(input.c_str(), identifier.c_str()) != nullptr : input == identifier) {
      static_cast<wxCheckBox*>(element)->SetValue(true);
      return true;
    }

    return false;
  };
}

Settings::ProffieDefine::ProffieDefine(std::string _name, wxString _defaultSelection, wxComboBox* _element, bool loose = false) {
  identifier = _name;
  strncpy(defaultValue.str, _defaultSelection.ToStdString().data(), _defaultSelection.length());

  element = _element;
  type = Type::COMBO;

  parseDefine = [&](std::string& input) -> bool {
    input = std::strtok(&input[0], " ");
    if (loose ? std::strstr(input.c_str(), identifier.c_str()) != nullptr : input == identifier) {
      static_cast<wxComboBox*>(element)->SetValue(std::strtok(nullptr, " \n\r"));
      return true;
    }

    return false;
  };
}

inline void Settings::ProffieDefine::overrideParser(std::function<bool(std::string&)> _parser) { parseDefine = _parser; }
inline void* Settings::ProffieDefine::getElementReference() { return element; }
