#include "propfile.h"
#include "core/defines.h"
#include "core/fileparse.h"

#include <fstream>
#include <iostream>

#include "pages/proppage.h"

#include <wx/tooltip.h>

PropFile::PropFile(const std::string& name) {
  std::cout << "Reading prop config: \"" << name << "\"..." << std::endl;
  readPropConfig(PROPCONFIG_DIR + name + ".pconf");
  page->Show(false);
  std::cout << "Finished reading prop config." << std::endl;
}

void PropFile::show(bool shouldShow) const { page->Show(shouldShow); }
std::string PropFile::getName() const { return name; }
std::string PropFile::getFileName() const { return fileName; }
std::string PropFile::Setting::getOutput() const {
  switch (type) {
    case SettingType::TOGGLE:
      return (toggle->GetValue()) ? define : "";
    case SettingType::OPTION:
      return (option->GetValue()) ? define : "";
    case SettingType::NUMERIC:
      return define + " " + std::to_string(numeric->num->GetValue());
    case SettingType::DECIMAL:
      return define + " " + std::to_string(decimal->num->GetValue());
  }

  return {};
}
std::unordered_map<std::string, PropFile::Setting>& PropFile::getSettings() { return settings; }
bool PropFile::Setting::checkRequiredSatisfied(const std::unordered_map<std::string, Setting>& settings) const {
  for (const auto& require : required) {
    auto key = settings.find(require);
    if (key == settings.end()) return false;
    if (key->second.getOutput().empty()) return false;
  }

  return true;
}


bool PropFile::readPropConfig(const std::string& pathname) {
  std::ifstream configFile(pathname);
  if (!configFile.is_open()) {
    std::cerr << "Could not open prop config file \"" << pathname << "\", skipping..." << std::endl;
    return false;
  }

  std::vector<std::string> config;
  std::string temp;
  while (!configFile.eof()) {
    getline(configFile, temp);
    config.push_back(temp);
  }
  configFile.close();

  if (!readName(config)) {
    std::cerr << "Prop config file \"" << pathname << "\" does not have required field \"NAME\", aborting..." << std::endl;
    return false;
  }
  if (!readFileName(config)) {
    std::cerr << "Prop config file \"" << pathname << "\" does not have required field \"FILENAME\", aborting..." << std::endl;
    return false;
  }
  if (!readSettings(config)) {
    std::cerr << "Prop config file \"" << pathname << "\" does not have optional field \"SETTINGS\", skipping..." << std::endl;
  }
  if (!readLayout(config)) {
    std::cerr << "Prop config file \"" << pathname << "\" does not have optional field \"LAYOUT\", skipping..." << std::endl;
  }
  if (!readButtons(config)) {
    std::cerr << "Prop config file \"" << pathname << "\" does not have optaional field \"BUTTONS\", skipping..." << std::endl;
  }

  return true;
}

bool PropFile::readName(std::vector<std::string>& config) {
  name = FileParse::parseEntry("NAME", config);
  if (name.empty()) return false;
  return true;
}
bool PropFile::readFileName(std::vector<std::string>& config) {
  fileName = FileParse::parseEntry("FILENAME", config);
  if (fileName.empty()) return false;
  return true;
}
bool PropFile::readSettings(std::vector<std::string>& config) {
  std::vector<std::string> settingsSection = FileParse::extractSection("SETTINGS", config);
  std::vector<std::string> section;
  std::vector<std::pair<std::string, Setting>> tempSettings;

  bool read{true};
  while (read) {
    read = false;
    for (auto it = settingsSection.begin(); it < settingsSection.end(); it++) {
      if (!(section = FileParse::extractSection("TOGGLE", settingsSection)).empty()) {
        read = true;
        Setting setting;
        if (!parseSettingCommon(setting, section)) continue;
        setting.type = Setting::SettingType::TOGGLE;
        setting.disables = FileParse::parseListEntry("DISABLE", section);

        tempSettings.push_back({ setting.define, setting });
      }
      if (!(section = FileParse::extractSection("OPTION", settingsSection)).empty()) {
        read = true;
        std::vector<std::string> selection;
        bool isFirst{true};
        while (!(selection = FileParse::extractSection("SELECTION", section)).empty()) {
          Setting setting;
          if (!parseSettingCommon(setting, selection)) continue;
          setting.type = Setting::SettingType::OPTION;
          if (isFirst) {
            isFirst = false;
            setting.isDefault = true;
          }

          setting.disables = FileParse::parseListEntry("DISABLE", selection);

          tempSettings.push_back({setting.define, setting});
        }
      }
      if (!(section = FileParse::extractSection("NUMERIC", settingsSection)).empty()) {
        read = true;
        Setting setting;
        if (!parseSettingCommon(setting, section)) continue;
        setting.type = Setting::SettingType::NUMERIC;

        double entry;
        if ((entry = FileParse::parseNumEntry("MIN", section)) > 0) setting.min = entry;
        if ((entry = FileParse::parseNumEntry("MAX", section)) > 0) setting.max = entry;
        if ((entry = FileParse::parseNumEntry("INCREMENT", section)) > 0) setting.increment = entry;
        if ((entry = FileParse::parseNumEntry("DEFAULT", section)) > 0) setting.defaultVal = entry;

        tempSettings.push_back({setting.define, setting});
      }
      if (!(section = FileParse::extractSection("DECIMAL", settingsSection)).empty()) {
        read = true;
        Setting setting;
        if (!parseSettingCommon(setting, section)) continue;
        setting.type = Setting::SettingType::DECIMAL;

        double entry;
        if ((entry = FileParse::parseNumEntry("MIN", section)) > 0) setting.min = entry;
        if ((entry = FileParse::parseNumEntry("MAX", section)) > 0) setting.max = entry;
        if ((entry = FileParse::parseNumEntry("INCREMENT", section)) > 0) setting.increment = entry;
        if ((entry = FileParse::parseNumEntry("DEFAULT", section)) > 0) setting.defaultVal = entry;

        tempSettings.push_back({setting.define, setting});
      }
    }
  }

  settings.clear();
  settings.insert(tempSettings.begin(), tempSettings.end());

  return true;
}
bool PropFile::parseSettingCommon(Setting& setting, std::vector<std::string>& search) {
  setting.name = FileParse::parseEntry("NAME", search);
  if (setting.name.empty()) {
    std::cerr << "Skipping entry with no name..." << std::endl;
    return false;
  }
  setting.define = FileParse::parseLabel(search.at(0));
  std::string toRemove = " ";
  setting.define.erase(std::remove_if(setting.define.begin(), setting.define.end(), [&toRemove](char c) { return toRemove.find(c) != std::string::npos; }), setting.define.end());
  if (setting.define.empty()) {
    std::cerr << "Entry \"" << setting.name << "\" has empty define, skipping..." << std::endl;
    return false;
  }
  setting.description = FileParse::parseEntry("DESCRIPTION", search);
  setting.required = FileParse::parseListEntry("REQUIRES", search);

  return true;
}
bool PropFile::readLayout(std::vector<std::string>& config) {
  page = new wxBoxSizer(wxVERTICAL);
  PropPage::instance->sizer->Add(page);

  auto layoutSection = FileParse::extractSection("LAYOUT", config);
  parseLayoutSection(layoutSection, page,  PropPage::instance->sizer->GetStaticBox());
  return true;
}

bool PropFile::parseLayoutSection(std::vector<std::string>& section, wxSizer* sizer, wxWindow* parent) {
# define ITEMBORDER wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 5)
  auto createToggle = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.toggle = new wxCheckBox(parent, wxID_ANY, setting.name);
    setting.toggle->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.toggle, ITEMBORDER);
  };
  auto createNumeric = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.numeric = Misc::createNumEntry(parent, setting.name, wxID_ANY, setting.min, setting.max, setting.defaultVal);
    setting.numeric->num->SetIncrement(setting.increment);
    setting.numeric->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.numeric->box, ITEMBORDER);
  };
  auto createDecimal = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.decimal = Misc::createNumEntryDouble(parent, setting.name, wxID_ANY, setting.min, setting.max, setting.defaultVal);
    setting.decimal->num->SetIncrement(setting.increment);
    setting.decimal->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.decimal->box, ITEMBORDER);
  };
  auto createOption = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.option = new wxRadioButton(parent, wxID_ANY, setting.name);
    setting.option->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.option, ITEMBORDER);
  };
# undef ITEMBORDER

  while (!section.empty()) {
    if (section.begin()->find("HORIZONTAL") != std::string::npos || section.begin()->find("VERTICAL") != std::string::npos) {
      auto isHorizontal = section.begin()->find("HORIZONTAL") != std::string::npos;
      auto label = FileParse::parseLabel(*section.begin());
      auto newSection = FileParse::extractSection(isHorizontal ? "HORIZONTAL" : "VERTICAL", section);
      newSection.erase(newSection.begin()); // Erase HORIZONTAL or VERTICAL header to prevent infinite recursion.
#     define BOXBORDER wxSizerFlags(0).Border(wxALL, 5)
      if (label.empty()) { // If no label
        auto newSizer = new wxBoxSizer(isHorizontal ? wxHORIZONTAL : wxVERTICAL);
        parseLayoutSection(newSection, newSizer, parent);
        sizer->Add(newSizer, BOXBORDER);
      } else { // Has label
        auto newSizer = new wxStaticBoxSizer(isHorizontal ? wxHORIZONTAL : wxVERTICAL, parent, label);
        parseLayoutSection(newSection, newSizer, newSizer->GetStaticBox());
        sizer->Add(newSizer, BOXBORDER);
      }
#     undef BOXBORDER
    }
    else if (section.begin()->find("OPTION") != std::string::npos) {
      auto key = settings.find(FileParse::parseLabel(*section.begin()));
      if (key == settings.end()) {
        std::cerr << R"(OPTION: ")" << FileParse::parseLabel(*section.begin()) << R"(" not found in settings, skipping...)" << std::endl;
        section.erase(section.begin());
        continue;
      }
      switch (key->second.type) {
        case Setting::SettingType::TOGGLE: createToggle(key->second, parent, sizer); break;
        case Setting::SettingType::NUMERIC: createNumeric(key->second, parent, sizer); break;
        case Setting::SettingType::DECIMAL: createDecimal(key->second, parent, sizer); break;
        case Setting::SettingType::OPTION: createOption(key->second, parent, sizer); break;
      }
      section.erase(section.begin());
    }
    else {
      section.erase(section.begin());
    }
  }

  return true;
}


bool PropFile::readButtons(std::vector<std::string>& config) { return true; }

