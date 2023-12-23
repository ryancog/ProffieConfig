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
        try { setting.defaultVal = stoi(FileParse::parseEntry("DEFAULT", section)); } catch (const std::invalid_argument&) { setting.defaultVal = 0; }
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
  page = new wxBoxSizer(wxVERTICAL);
  PropPage::instance->sizer->Add(page);

  auto layoutSection = FileParse::extractSection("LAYOUT", config);
  parseLayoutSection(layoutSection, page,  PropPage::instance->sizer->GetStaticBox());

  //page->Show(false);

  return true;
}

bool PropFile::parseLayoutSection(std::vector<std::string>& section, wxSizer* sizer, wxWindow* parent) {
  auto createToggle = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.toggle = new wxCheckBox(parent, wxID_ANY, setting.name);
    setting.toggle->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.toggle);
  };
  auto createNumeric = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.numeric = Misc::createNumEntry(parent, setting.name, wxID_ANY, setting.min, setting.max, setting.defaultVal);
    setting.numeric->num->SetIncrement(setting.increment);
    setting.numeric->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.numeric->box);
  };
  auto createDecimal = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.decimal = Misc::createNumEntryDouble(parent, setting.name, wxID_ANY, setting.min, setting.max, setting.defaultVal);
    setting.decimal->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.decimal->box);
  };
  auto createOption = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
    setting.option = new wxRadioButton(parent, wxID_ANY, setting.name);
    setting.option->SetToolTip(new wxToolTip(setting.description));
    sizer->Add(setting.option);
  };

  while (!section.empty()) {
    if (section.begin()->find("HORIZONTAL") != std::string::npos || section.begin()->find("VERTICAL") != std::string::npos) {
      auto isHorizontal = section.begin()->find("HORIZONTAL") != std::string::npos;
      auto label = FileParse::parseLabel(*section.begin());
      auto newSection = FileParse::extractSection(isHorizontal ? "HORIZONTAL" : "VERTICAL", section);
      newSection.erase(newSection.begin()); // Erase HORIZONTAL or VERTICAL header to prevent infinite recursion.
      if (label.empty()) { // If no label
        auto newSizer = new wxBoxSizer(isHorizontal ? wxHORIZONTAL : wxVERTICAL);
        parseLayoutSection(newSection, newSizer, parent);
        sizer->Add(newSizer);
      } else { // Has label
        auto newSizer = new wxStaticBoxSizer(isHorizontal ? wxHORIZONTAL : wxVERTICAL, parent, label);
        parseLayoutSection(newSection, newSizer, newSizer->GetStaticBox());
        sizer->Add(newSizer);
      }
    }
    else if (section.begin()->find("OPTION") != std::string::npos) {
      auto key = settings.find(FileParse::parseLabel(*section.begin()));
      if (key == settings.end()) {
        std::cerr << R"(OPTION: ")" << FileParse::parseLabel(*section.begin()) << R"(" not found in settings, skipping...)" << std::endl;
        section.erase(section.begin());
        continue;
      }
      switch (key->second.type) {
        case SettingType::TOGGLE: createToggle(key->second, parent, sizer); break;
        case SettingType::NUMERIC: createNumeric(key->second, parent, sizer); break;
        case SettingType::DECIMAL: createDecimal(key->second, parent, sizer); break;
        case SettingType::OPTION: createOption(key->second, parent, sizer); break;
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

