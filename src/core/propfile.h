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
  struct Setting;
  PropFile(const std::string&);

  void show(bool = true) const;
  std::string getName() const;
  std::string getFileName() const;
  std::unordered_map<std::string, Setting>& getSettings();

  struct Setting {
    std::string getOutput() const;
    bool checkRequiredSatisfied(const std::unordered_map<std::string, Setting>&) const;

    std::string name{""};
    std::string define{""};
    std::string description{""};

    std::vector<std::string> required{};
    std::vector<std::string> disables{};
    bool disabled{false};

    double min{0};
    double max{100};
    double increment{1};
    double defaultVal{0};

    std::vector<std::string> others{};
    bool isDefault{false};

    enum class SettingType {
      TOGGLE,
      NUMERIC,
      DECIMAL,
      OPTION
    } type;

    union {
      wxCheckBox* toggle;
      wxRadioButton* option;
      Misc::numEntry* numeric;
      Misc::numEntryDouble* decimal;
    };
  };


private:
  std::string name{""};
  std::string fileName{""};
  std::unordered_map<std::string, Setting> settings;
  wxBoxSizer* page;

  bool readPropConfig(const std::string& pathname);
  bool readName(std::vector<std::string>&);
  bool readFileName(std::vector<std::string>&);
  bool readSettings(std::vector<std::string>&);
  bool readLayout(std::vector<std::string>&);
  bool parseLayoutSection(std::vector<std::string>&, wxSizer*, wxWindow*);
  bool readButtons(std::vector<std::string>&);

  [[nodiscard]] static bool parseSettingCommon(Setting&, std::vector<std::string>&);
};

