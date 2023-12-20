#pragma once

#include "wx/sizer.h"
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include "elements/misc.h"
#include <string>
#include <vector>
#include <unordered_map>

class PropFile {
public:
  PropFile(const std::string&);

  bool readPropConfig(const std::string& pathname);

private:
  enum class SettingType {
    TOGGLE,
    NUMERIC,
    DECIMAL,
    OPTION
  };


  struct Setting {
    std::string name{""};
    std::string define{""};
    std::string description{""};

    std::vector<std::string> requires{};
    std::vector<std::string> disables{};

    double min{0};
    double max{0};
    double increment{0};
    double defaultval{0};

    std::vector<std::string> others{};
    bool isDefault{false};

    SettingType type;

    union {
      wxCheckBox* toggle;
      wxRadioButton* option;
      Misc::numEntry* numeric;
      Misc::numEntryDouble* decimal;
    };

    void generateElement();
  };

  std::string name{""};
  std::string fileName{""};
  std::unordered_map<std::string, Setting> settings;
  wxBoxSizer* page;

  bool readName(std::vector<std::string>&);
  bool readFileName(std::vector<std::string>&);
  bool readSettings(std::vector<std::string>&);
  bool readLayout(std::vector<std::string>&);
  [[nodiscard]] wxSizer* parseLayoutSection(std::vector<std::string>&, wxWindow*);
  bool readButtons(std::vector<std::string>&);

  [[nodiscard]] static bool parseSettingCommon(Setting&, std::vector<std::string>&);
};

