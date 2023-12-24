// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#include "core/config/propfile.h"

#include "core/defines.h"
#include "core/utilities/fileparse.h"
#include "editor/editorwindow.h"
#include "editor/pages/proppage.h"

#include <fstream>
#include <iostream>
#include <wx/tooltip.h>

PropFile::PropFile() {}
PropFile::~PropFile() {
  EditorWindow::instance->propPage->sizer->Detach(page);
  delete page;
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
const std::array<std::vector<std::pair<std::string, std::vector<PropFile::Button>>>, 4>& PropFile::getButtons() { return buttons; }
bool PropFile::Setting::checkRequiredSatisfied(const std::unordered_map<std::string, Setting>& settings) const {
  for (const auto& require : required) {
    auto key = settings.find(require);
    if (key == settings.end()) return false;
    if (key->second.getOutput().empty()) return false;
  }

  return true;
}


PropFile* PropFile::createPropConfig(const std::string& name) {
  std::cout << "Reading prop config: \"" << name << "\"..." << std::endl;
  std::string pathname = PROPCONFIG_DIR + name + ".pconf";

  std::ifstream configFile(pathname);
  if (!configFile.is_open()) {
    error("Could not open prop config file \"" + pathname + "\", aborting...");
    return nullptr;
  }

  std::vector<std::string> config;
  std::string temp;
  while (!configFile.eof()) {
    getline(configFile, temp);
    config.push_back(temp.substr(0, temp.find("//")));
  }
  configFile.close();

  auto prop = new PropFile;

  if (!prop->readName(config)) {
    error("Prop config file \"" + name + "\" does not have section \"NAME\", aborting...");
    return nullptr;
  }
  if (!prop->readFileName(config)) {
    error("Prop config file \"" + name + "\" does not have section \"FILENAME\", aborting...");
    return nullptr;
  }
  if (!prop->readSettings(config)) {
    warning("Prop config file \"" + name + "\" does not have optional section \"SETTINGS\", skipping...");
  }
  if (!prop->readLayout(config)) {
    warning("Prop config file \"" + name + "\" does not have optional section \"LAYOUT\", skipping...");
  }
  if (!prop->readButtons(config)) {
    warning("Prop config file \"" + name + "\" does not have optional section \"BUTTONS\", skipping...");
  }

  prop->pruneUnused();

  prop->page->Show(false);
  std::cout << "Finished reading prop config." << std::endl;

  return prop;
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
    warning("Skipping entry with no name...");
    return false;
  }
  setting.define = FileParse::parseLabel(search.at(0));
  std::string toRemove = " ";
  setting.define.erase(std::remove_if(setting.define.begin(), setting.define.end(), [&toRemove](char c) { return toRemove.find(c) != std::string::npos; }), setting.define.end());
  if (setting.define.empty()) {
    warning("Entry \"" + setting.name + "\" has empty define, skipping...");
    return false;
  }
  setting.description = FileParse::parseEntry("DESCRIPTION", search);
  setting.required = FileParse::parseListEntry("REQUIRES", search);

  return true;
}
bool PropFile::readLayout(std::vector<std::string>& config) {
  page = new wxBoxSizer(wxVERTICAL);
  EditorWindow::instance->propPage->sizer->Add(page, wxSizerFlags(0).Expand());

  auto layoutSection = FileParse::extractSection("LAYOUT", config);
  parseLayoutSection(layoutSection, page,  EditorWindow::instance->propPage->sizer->GetStaticBox());
  return true;
}
bool PropFile::readButtons(std::vector<std::string>& config) {
  bool hasButtons{false};
  for (const auto& line : config) {
    if (line.find("BUTTONS") != std::string::npos) {
      hasButtons = true;
      break;
    }
  }
  if (!hasButtons) return false;

  parseButtons(config);

  return true;
}
void PropFile::parseButtons(std::vector<std::string>& config) {
  while (!false) {
    auto buttonSection = FileParse::extractSection("BUTTONS", config);
    if (buttonSection.empty()) break;

    auto numStart = buttonSection.at(0).find("{");
    auto numEnd = buttonSection.at(0).find("}");

    if (numStart == std::string::npos || numEnd == std::string::npos) {
      error("Button section missing number indicator, skipping...");
      continue;
    }

    if (!std::isdigit(buttonSection.at(0).at(numStart + 1))) {
      error("Button section number indicator malformed, skipping...");
      continue;
    }

    int32_t numButtons = std::stoi(buttonSection.at(0).substr(numStart + 1));
    if (numButtons < 0 || numButtons > 3) {
      error("Button section number indicator \"" + std::to_string(numButtons) + "\" out of range, skipping...");
      continue;
    }

    buttonSection.erase(buttonSection.begin()); // Prevent BUTTONS from being read as a BUTTON entry

    while (!false) {
      auto stateSection = FileParse::extractSection("STATE", buttonSection);
      if (stateSection.empty()) break;
      auto label = FileParse::parseLabel(stateSection.at(0));
      if (label.empty()) {
        error("Button array #" + std::to_string(numButtons) + " has unnamed/malformed state, skipping...");
        continue;
      }
      buttons.at(numButtons).push_back({label, {}});

      parseButtonSection(stateSection, numButtons, buttons.at(numButtons).size() - 1);
    }
  }
}
void PropFile::parseButtonSection(std::vector<std::string>& buttonSection, const int32_t& numButtons, const int32_t& state) {
  while (!buttonSection.empty()) {
    if (buttonSection.at(0).find("BUTTON") == std::string::npos) {
      buttonSection.erase(buttonSection.begin());
      continue;
    }

    auto section = FileParse::extractSection("BUTTON", buttonSection);
    if (section.empty()) continue;

    Button newButton;
    newButton.name =  FileParse::parseLabel(section.at(0));
    if (newButton.name.empty()) {
      error("Button entry has missing name, skipping...");
      continue;
    }

    parseButtonDescriptions(newButton, section);

    if (newButton.descriptions.find({}) == newButton.descriptions.end()) {
      error("Button entry \"" + newButton.name + "\" missing default, skipping...");
      continue;
    }

    parseButtonRelevantSettings(newButton);

    buttons.at(numButtons).at(state).second.push_back(newButton);
  }
}
void PropFile::parseButtonDescriptions(PropFile::Button& newButton, std::vector<std::string>& section) {
  while (!false) {
    std::string label;
    auto description = FileParse::parseEntry("DESCRIPTION", section, label);
    if (description.empty()) break;

    if (label.empty()) {
      if (newButton.descriptions.find({}) != newButton.descriptions.end()) {
        warning("Overriding duplicate default description for button \"" + newButton.name + "\", there should be only one default per button...");
      }
      newButton.descriptions.insert({{}, description});
    } else {
      std::vector<std::string> predicates;

      // Put back parsed-out quotes
      label.insert(label.begin(), '"');
      label.insert(label.end(), '"');

      while (label.find("\"") != std::string::npos) {
        auto predicateBegin = label.find_first_of("\"");
        auto predicateEnd = label.find_first_of("\"", predicateBegin + 1);
        predicates.push_back(label.substr(predicateBegin + 1, predicateEnd - predicateBegin - 1));
        label.erase(predicateBegin, predicateEnd + 1);
      }
      newButton.descriptions.insert({predicates, description});
    }
  }
}
void PropFile::parseButtonRelevantSettings(PropFile::Button& newButton) {
  for (const auto& [ predicates, description ] : newButton.descriptions) {
    for (const auto& predicate : predicates) {
      if ([&predicate, &newButton]() { for (const auto& setting : newButton.relevantSettings) if (setting == predicate) return false; return true; }()) {
        newButton.relevantSettings.push_back(predicate);
      }
    }
  }
}

void PropFile::pruneUnused() {
  for (auto setting = settings.begin(); setting != settings.end();) {
    // It doesn't matter which union member we access, we're effectively reading it as a void*
    if (setting->second.toggle != nullptr) {
      setting++;
      continue;
    }
    warning("Removing unused setting \"" + setting->second.name + "\"...");
    setting = settings.erase(setting);
  }
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
      if (label.empty()) { // If no label
        auto newSizer = new wxBoxSizer(isHorizontal ? wxHORIZONTAL : wxVERTICAL);
        parseLayoutSection(newSection, newSizer, parent);
        sizer->Add(newSizer, wxSizerFlags(0).Expand());
      } else { // Has label
        auto newSizer = new wxStaticBoxSizer(isHorizontal ? wxHORIZONTAL : wxVERTICAL, parent, label);
        parseLayoutSection(newSection, newSizer, newSizer->GetStaticBox());
        sizer->Add(newSizer, wxSizerFlags(0).Border(wxALL, 5).Expand());
      }
    }
    else if (section.begin()->find("OPTION") != std::string::npos) {
      auto key = settings.find(FileParse::parseLabel(*section.begin()));
      if (key == settings.end()) {
        warning(R"(Option ")" + FileParse::parseLabel(*section.begin()) + R"(" not found in settings, skipping...)");
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

void PropFile::warning(const std::string& warning) {
  std::cerr << "WARNING: " << warning << std::endl;
}

void PropFile::error(const std::string& error) {
  std::cerr << "ERROR: " << error << std::endl;
}

