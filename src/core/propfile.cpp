#include "propfile.h"
#include "core/fileparse.h"

#include <fstream>
#include <iostream>

#include "pages/proppage.h"

#include <wx/tooltip.h>

PropFile::PropFile(const std::string& pathname) {
  readPropConfig(pathname);
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
        setting.type = SettingType::TOGGLE;
        tempSettings.push_back({ setting.define, setting });
      }
      if (!(section = FileParse::extractSection("NUMERIC", settingsSection)).empty()) {
        read = true;
        Setting setting;
        if (!parseSettingCommon(setting, section)) continue;
        setting.type = SettingType::NUMERIC;
        try { setting.min = stoi(FileParse::parseEntry("MIN", section)); } catch (const std::invalid_argument&) { setting.min = 0; }
        try { setting.max = stoi(FileParse::parseEntry("MAX", section)); } catch (const std::invalid_argument&) { setting.max = 100; }
        try { setting.increment = stoi(FileParse::parseEntry("INCREMENT", section)); } catch (const std::invalid_argument&) { setting.increment = 1; }
        //try { setting.defaultVal = stoi(FileParse:)
        tempSettings.push_back({setting.define, setting});
      }
      if (!(section = FileParse::extractSection("DECIMAL", settingsSection)).empty()) {
        read = true;
        Setting setting;
        if (!parseSettingCommon(setting, section)) continue;
        setting.type = SettingType::DECIMAL;
        try { setting.min = stod(FileParse::parseEntry("MIN", section)); } catch (const std::invalid_argument&) { setting.min = 0; }
        try { setting.max = stod(FileParse::parseEntry("MAX", section)); } catch (const std::invalid_argument&) { setting.max = 100; }
        try { setting.increment = stod(FileParse::parseEntry("INCREMENT", section)); } catch (const std::invalid_argument&) { setting.increment = 1; }
        tempSettings.push_back({setting.define, setting});
      }
      if (!(section = FileParse::extractSection("OPTION", settingsSection)).empty()) {
        read = true;
        std::vector<std::string> selection;
        bool isFirst{true};
        while (!(selection = FileParse::extractSection("SELECTION", section)).empty()) {
          Setting setting;
          if (!parseSettingCommon(setting, selection)) continue;
          setting.type = SettingType::OPTION;
          if (isFirst) {
            isFirst = false;
            setting.isDefault = true;
          }
          tempSettings.push_back({setting.define, setting});
        }
      }
    }
  }

  settings.clear();
  settings.insert(tempSettings.begin(), tempSettings.end());

  return true;
}
bool PropFile::parseSettingCommon(Setting& setting, std::vector<std::string>& search) {
  setting.name = FileParse::parseEntry("NAME", search);
  if (setting.name.empty()) return false;
  setting.define = FileParse::parseLabel(search.at(0));
  setting.description = FileParse::parseEntry("DESCRIPTION", search);
  setting.required = FileParse::parseListEntry("REQUIRES", search);
  setting.disables = FileParse::parseListEntry("DISABLE", search);

  return true;
}
bool PropFile::readLayout(std::vector<std::string>& config) {
  page = new wxStaticBoxSizer(wxVERTICAL, PropPage::instance, name + " Settings");
  page->Show(false);

  bool read{true};
  for (const std::string& line : config) {

  }

  //PropPage::instance->
  return true;
}
wxSizer* PropFile::parseLayoutSection(std::vector<std::string>& section, wxWindow* parent) {
  for (const std::string& line : section) {
    if (line.find("OPTION") != std::string::npos) {
      auto setting = settings.at(FileParse::parseLabel(line));
      switch (setting.type) {
        case SettingType::TOGGLE:
          auto element = new wxCheckBox(parent, wxID_ANY, setting.name);
          element->SetToolTip(new wxToolTip(setting.description));
        case SettingType::NUMERIC:
        case SettingType::DECIMAL:
        case SettingType::OPTION:
          break;
      }
    }
    if (FileParse::extractSection());
  }
}


bool PropFile::readButtons(std::vector<std::string>& config) { return true; }


void PropFile::Setting::generateElement() {
  switch (type) {
    case SettingType::TOGGLE:
      toggle = new wxCheckBox(nullptr, wxID_ANY, name);
      toggle->SetToolTip(new wxToolTip(description));
      break;
    case SettingType::NUMERIC:
    //numeric = Misc::createNumEntry(nullptr, name, wxID_ANY, min, max, )
    case SettingType::DECIMAL:
    case SettingType::OPTION:
      break;
  }
}
