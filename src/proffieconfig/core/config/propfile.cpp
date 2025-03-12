#include "propfile.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

#include <wx/tooltip.h>
#include <wx/statbox.h>

#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "utils/paths.h"
#include "ui/controls.h"

#include "../../core/utilities/misc.h"

PropFile::PropFile(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
  settings = new SettingMap;
  buttons = new std::array<ButtonArray, 4>;
}
PropFile::~PropFile() {
  delete settings;
  delete buttons;
}

std::string PropFile::getName() const { return name; }
std::string PropFile::getFileName() const { return fileName; }
std::string PropFile::getInfo() const { return info; }

std::string PropFile::Setting::getOutput() const {
  switch (type) {
    case SettingType::TOGGLE:
      return static_cast<wxCheckBox*>(control)->GetValue() ? define : "";
    case SettingType::OPTION:
      return static_cast<wxRadioButton*>(control)->GetValue() ? define : "";
    case SettingType::NUMERIC:
      return define + " " + std::to_string(static_cast<PCUI::Numeric*>(control)->entry()->GetValue());
    case SettingType::DECIMAL:
      return define + " " + std::to_string(static_cast<PCUI::NumericDec*>(control)->entry()->GetValue());
  }

  return {};
}

void PropFile::Setting::enable(bool enable) const {
  switch(type) {
    case PropFile::Setting::SettingType::TOGGLE:
      static_cast<wxCheckBox*>(control)->Enable(enable);
      break;
    case PropFile::Setting::SettingType::OPTION:
      static_cast<wxRadioButton*>(control)->Enable(enable);
      break;
    case PropFile::Setting::SettingType::NUMERIC:
      static_cast<PCUI::Numeric*>(control)->entry()->Enable(enable);
      break;
    case PropFile::Setting::SettingType::DECIMAL:
      static_cast<PCUI::NumericDec*>(control)->entry()->Enable(enable);
      break;

  }
}

void PropFile::Setting::setValue(double value) const {
  switch (type) {
    case SettingType::TOGGLE:
      static_cast<wxCheckBox*>(control)->SetValue(value);
      break;
    case SettingType::OPTION:
      static_cast<wxRadioButton*>(control)->SetValue(value);
      break;
    case SettingType::NUMERIC:
      static_cast<PCUI::Numeric*>(control)->entry()->SetValue(value);
      break;
    case SettingType::DECIMAL:
      static_cast<PCUI::NumericDec*>(control)->entry()->SetValue(value);
      break;
  }
}

PropFile::SettingMap* PropFile::getSettings() { return settings; }

const std::array<PropFile::ButtonArray, 4>* PropFile::getButtons() { return buttons; }

bool PropFile::Setting::checkRequiredSatisfied(const std::unordered_map<std::string, Setting>& settings) const {
  if (!requiredAny.empty()) {
    for (const auto& require : requiredAny) {
      auto key = settings.find(require);
      if (key == settings.end()) continue;
      if (!key->second.getOutput().empty()) return true;
    }

    return false;
  } else {
    for (const auto& require : required) {
      auto key = settings.find(require);
      if (key == settings.end()) return false;
      if (key->second.getOutput().empty()) return false;
    }

    return true;
  }
}

PropFile* PropFile::createPropConfig(const string& propName, wxWindow* _parent, bool builtin) {
    auto& logger{Log::Context::getGlobal().createLogger("PropFile::createPropConfig()")};
    logger.info("Loading prop " + propName);
    filepath propPath;
    if (builtin) {
        propPath = Paths::resources() / "props" / (propName + ".pconf");
    } else {
        propPath = Paths::props() / (propName + ".pconf");
    }

    std::ifstream configFile(propPath);
    if (!configFile.is_open()) {
        logger.error("Could not open prop config file \"" + propPath.string() + "\", aborting...");
        return nullptr;
    }

    PConf::Data data;
    PConf::read(configFile, data, logger.binfo("Reading pconf..."));
    configFile.close();

    auto prop = new PropFile(_parent);
    const auto hashedData{PConf::hash(data)};

    const auto name{hashedData.find("NAME")};
    if (name == hashedData.end() or not name->second->value) {
        logger.error("Prop config file \"" + propName + "\" does not have section \"NAME\", aborting...");
        return nullptr;
    }
    prop->name = *name->second->value;

    const auto fileName{hashedData.find("FILENAME")};
    if (fileName == hashedData.end() or not fileName->second->value) {
        logger.error("Prop config file \"" + propName + "\" does not have section \"FILENAME\", aborting...");
        return nullptr;
    }
    prop->fileName = *fileName->second->value;

    const auto info{hashedData.find("INFO")};
    if (info == hashedData.end() or not info->second->value) {
        prop->info = "Prop has no additional info.";
        logger.info("Prop config file \"" + propName + "\" does not have optional section \"INFO\", skipping...");
    } else {
        prop->info = *info->second->value;
    }

    const auto settings {hashedData.find("SETTINGS")};
    if (settings == hashedData.end() or settings->second->getType() != PConf::Type::SECTION) {
        logger.info("Prop config file \"" + propName + "\" does not have optional section \"SETTINGS\", skipping...");
    } else {
        prop->readSettings(PConf::hash(std::static_pointer_cast<PConf::Section>(settings->second)->entries), *logger.binfo("Reading settings..."));
    }

    const auto layout{hashedData.find("LAYOUT")};
    if (layout == hashedData.end() or layout->second->getType() != PConf::Type::SECTION) {
        logger.info("Prop config file \"" + propName + "\" does not have optional section \"LAYOUT\", skipping...");
    } else {
        prop->readLayout(std::static_pointer_cast<PConf::Section>(layout->second)->entries, *logger.binfo("Reading layout..."));
    }

    const auto buttonsRange{hashedData.equal_range("BUTTONS")};
    if (buttonsRange.first == buttonsRange.second) {
        logger.info("Prop config file \"" + propName + "\" does not have optional section \"BUTTONS\", skipping...");
    } else {
        for (auto it{buttonsRange.first}; it != buttonsRange.second; ++it) {
            if (it->second->getType() == PConf::Type::SECTION) {
                prop->readButtons(std::static_pointer_cast<PConf::Section>(it->second), *logger.binfo("Reading buttons..."));
            }
        }
    }

    for (auto setting = prop->settings->begin(); setting != prop->settings->end();) {
        if (setting->second.control != nullptr) {
            ++setting;
            continue;
        }
        logger.warn("Removing unused setting \"" + setting->second.name + "\"...");
        setting = prop->settings->erase(setting);
    }

    prop->Show(false);

    return prop;
}

void PropFile::readSettings(const PConf::HashedData& data, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PropFile::readSettings()")};
    vector<std::pair<string, Setting>> tempSettings;

    const auto parseDisables([](const PConf::HashedData& data) -> vector<string> {
        vector<string> ret;

        const auto disables{data.find("DISABLE")};
        if (disables != data.end() and disables->second->value) {
            auto disableValue{*disables->second->value};
            while (true) {
                const auto disableEnd{disableValue.find('\n')};
                ret.push_back(disableValue.substr(0, disableEnd));

                if (disableEnd == std::string::npos) break;
                disableValue = disableValue.substr(disableEnd + 1);
            }
        }

        return ret;
    });

    const auto toggleRange{data.equal_range("TOGGLE")};
    for (auto it{toggleRange.first}; it != toggleRange.second; ++it) {
        Setting setting;
        const auto toggleData{parseSettingCommon(setting, it->second, logger)};
        if (not toggleData) continue;

        setting.type = Setting::SettingType::TOGGLE;
        setting.disables = parseDisables(*toggleData);

        tempSettings.push_back({setting.define, setting});
    }

    const auto optionRange{data.equal_range("OPTION")};
    for (auto it{optionRange.first}; it != optionRange.second; ++it) {
        if (it->second->getType() != PConf::Type::SECTION) continue;

        const auto optionData{PConf::hash(std::static_pointer_cast<PConf::Section>(it->second)->entries)};
        const auto selectionRange{optionData.equal_range("SELECTION")};
        for (auto selectionIt{selectionRange.first}; selectionIt != selectionRange.second; ++selectionIt) {
            Setting setting;
            const auto selectionData{parseSettingCommon(setting, selectionIt->second, logger)};
            if (not selectionData)  continue;

            setting.type = Setting::SettingType::OPTION;
            setting.isDefault = selectionIt == selectionRange.first;
            setting.disables = parseDisables(*selectionData);
            setting.shouldOutput = selectionData->find("NO_OUTPUT") != selectionData->end();

            tempSettings.push_back({setting.define, setting});
        }
    }

    const auto numericRange{data.equal_range("NUMERIC")};
    for (auto it{numericRange.first}; it != numericRange.second; ++it) {
        Setting setting;
        const auto numericData{parseSettingCommon(setting, it->second, logger)};
        if (not numericData) continue;

        setting.type = Setting::SettingType::NUMERIC;
        const auto minEntry{numericData->find("MIN")};
        if (minEntry != numericData->end() and minEntry->second->value) setting.min = strtol(minEntry->second->value->c_str(), nullptr, 10);
        const auto maxEntry{numericData->find("MAX")};
        if (maxEntry != numericData->end() and maxEntry->second->value) setting.max = strtol(maxEntry->second->value->c_str(), nullptr, 10);
        const auto defaultEntry{numericData->find("DEFAULT")};
        if (defaultEntry != numericData->end() and defaultEntry->second->value) setting.defaultVal = strtol(defaultEntry->second->value->c_str(), nullptr, 10);
        const auto incrementEntry{numericData->find("INCREMENT")};
        if (incrementEntry != numericData->end() and incrementEntry->second->value) setting.increment = strtol(incrementEntry->second->value->c_str(), nullptr, 10);

        tempSettings.push_back({setting.define, setting});
    }

    const auto decimalRange{data.equal_range("DECIMAL")};
    for (auto it{decimalRange.first}; it != decimalRange.second; ++it) {
        Setting setting;
        const auto decimalData{parseSettingCommon(setting, it->second, logger)};
        if (not decimalData) continue;

        setting.type = Setting::SettingType::DECIMAL;
        const auto minEntry{decimalData->find("MIN")};
        if (minEntry != decimalData->end() and minEntry->second->value) setting.min = strtod(minEntry->second->value->c_str(), nullptr);
        const auto maxEntry{decimalData->find("MAX")};
        if (maxEntry != decimalData->end() and maxEntry->second->value) setting.max = strtod(maxEntry->second->value->c_str(), nullptr);
        const auto defaultEntry{decimalData->find("DEFAULT")};
        if (defaultEntry != decimalData->end() and defaultEntry->second->value) setting.defaultVal = strtod(defaultEntry->second->value->c_str(), nullptr);
        const auto incrementEntry{decimalData->find("INCREMENT")};
        if (incrementEntry != decimalData->end() and incrementEntry->second->value) setting.increment = strtod(incrementEntry->second->value->c_str(), nullptr);

        tempSettings.push_back({setting.define, setting});
    }

    settings->clear();
    settings->insert(tempSettings.begin(), tempSettings.end());
}

optional<PConf::HashedData> PropFile::parseSettingCommon(Setting& setting, const std::shared_ptr<PConf::Entry>& entry, Log::Logger& logger) {
    if (not entry->label) {
        logger.warn(entry->name + " section has no label, ignoring!");
        return nullopt;
    }

    setting.define = *entry->label;
    trimWhiteSpace(setting.define);
    if (setting.define.empty()) {
        logger.warn(entry->name + " section has empty define/label, ignoring!");
        return nullopt;
    }

    if (entry->getType() != PConf::Type::SECTION) {
        logger.warn(entry->name + " entry is not section, ignoring!");
        return nullopt;
    }
    const auto data{PConf::hash(std::static_pointer_cast<PConf::Section>(entry)->entries)};

    const auto name{data.find("NAME")};
    if (name == data.end() or not name->second->value) {
        logger.warn(entry->name + " section does not have the required \"NAME\" entry, ignoring!");
        return nullopt;
    }
    setting.name = *name->second->value;

    const auto description{data.find("DESCRIPTION")};
    if (description != data.end() and description->second->value) setting.description = *description->second->value;

    const auto requireAny{data.find("REQUIREANY")};
    if (requireAny != data.end() and requireAny->second->value) {
        auto requireAnyValue{*requireAny->second->value};
        while (true) {
            const auto requireAnyEnd{requireAnyValue.find('\n')};
            setting.requiredAny.push_back(requireAnyValue.substr(0, requireAnyEnd));

            if (requireAnyEnd == std::string::npos) break;
            requireAnyValue = requireAnyValue.substr(requireAnyEnd + 1);
        }
    }

    const auto required{data.find("REQUIRE")};
    if (required != data.end() and required->second->value) {
        auto requiredValue{*required->second->value};
        while (true) {
            const auto requiredEnd{requiredValue.find('\n')};
            setting.required.push_back(requiredValue.substr(0, requiredEnd));

            if (requiredEnd == std::string::npos) break;
            requiredValue = requiredValue.substr(requiredEnd + 1);
        }
    }

    return data;
}

void PropFile::readLayout(const PConf::Data& data, Log::Branch& lBranch) {
#   define ITEMBORDER wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 5)
    auto& logger{lBranch.createLogger("PropFile::readLayout()")};
    auto createToggle = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
        setting.control = new wxCheckBox(parent, wxID_ANY, setting.name);
        static_cast<wxCheckBox*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
        sizer->Add(static_cast<wxCheckBox*>(setting.control), ITEMBORDER);
    };
    auto createNumeric = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
        auto entry = new PCUI::Numeric(parent, wxID_ANY, setting.min, setting.max, setting.defaultVal, setting.increment, wxSP_ARROW_KEYS, setting.name);
        setting.control = entry;
        static_cast<PCUI::Numeric*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
        sizer->Add(entry, ITEMBORDER);
    };
    auto createDecimal = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
        auto entry = new PCUI::NumericDec(parent, wxID_ANY, setting.min, setting.max, setting.defaultVal, setting.increment, wxSP_ARROW_KEYS, setting.name);
        setting.control = entry;
        static_cast<PCUI::NumericDec*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
        sizer->Add(entry, ITEMBORDER);
    };
    auto createOption = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
        setting.control = new wxRadioButton(parent, wxID_ANY, setting.name);
        static_cast<wxRadioButton*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
        sizer->Add(static_cast<wxRadioButton*>(setting.control), ITEMBORDER);
    };

    vector<wxWindow *> parentStack;
    vector<wxSizer*> sizerStack;
    sizer = new wxBoxSizer(wxVERTICAL);

    // <Current Iterator, End Iterator>
    vector<std::pair<decltype(data.begin()), decltype(data.end())>> entryStack;
    if (not data.empty()) entryStack.emplace_back(data.begin(), data.end());

    while (true) {
        if (entryStack.empty()) break;
        const auto& entry{*entryStack.back().first};

        if (entry->name == "HORIZONTAL" or entry->name == "VERTICAL") {
            if (entry->getType() == PConf::Type::SECTION) {
                wxSizer *nextSizer{nullptr};
                auto *previousSizer{sizerStack.empty() ? sizer : sizerStack.back()};
                if (entry->label) {
                    auto *sectionSizer{new wxStaticBoxSizer(entry->name == "HORIZONTAL" ? wxHORIZONTAL : wxVERTICAL, parentStack.empty() ? this : parentStack.back(), *entry->label)};
                    nextSizer = sectionSizer;
                    parentStack.push_back(sectionSizer->GetStaticBox());
                    previousSizer->Add(nextSizer, ITEMBORDER.Expand());
                } else {
                    auto *sectionSizer{new wxBoxSizer(entry->name == "HORIZONTAL" ? wxHORIZONTAL : wxVERTICAL)};
                    nextSizer = sectionSizer;
                    previousSizer->Add(nextSizer, wxSizerFlags{}.Expand());
                }

                const auto& entries{std::static_pointer_cast<PConf::Section>(entry)->entries};
                if (not entries.empty()) {
                    // Decrement to past-begin since this will be prematurely incremented
                    // (instead of the "intended" current stack, due to the new addition)
                    entryStack.emplace_back(--entries.begin(), entries.end());
                    sizerStack.push_back(nextSizer);
                }
            }
        } else if (entry->name == "SETTING") {
            if (entry->getType() == PConf::Type::ENTRY and entry->label) {
                auto define{settings->find(entry->label.value_or(""))};
                if (define == settings->end()) {
                    logger.warn("Setting \"" + *entry->label + "\" not found in settings, skipping...");
                } else {
                    auto *parent{parentStack.empty() ? this : parentStack.back()};
                    auto *sectionSizer{sizerStack.empty() ? sizer : sizerStack.back()};
                    switch (define->second.type) {
                        case Setting::SettingType::TOGGLE: createToggle(define->second, parent, sectionSizer); break;
                        case Setting::SettingType::NUMERIC: createNumeric(define->second, parent, sectionSizer); break;
                        case Setting::SettingType::DECIMAL: createDecimal(define->second, parent, sectionSizer); break;
                        case Setting::SettingType::OPTION: createOption(define->second, parent, sectionSizer); break;
                    }
                    if (define->second.isDefault) define->second.setValue(true);
                }
            }
        }

        while (true) {
            ++entryStack.back().first;
            if (entryStack.back().first == entryStack.back().second) {
                entryStack.pop_back();
                if (not sizerStack.empty()) {
                    if (auto *staticBoxSizer = dynamic_cast<wxStaticBoxSizer *>(sizerStack.back())) {
                        if (not parentStack.empty() and staticBoxSizer->GetStaticBox() == parentStack.back()) parentStack.pop_back();
                    }
                    sizerStack.pop_back();
                }
                if (entryStack.empty()) break;
                continue;
            }
            break;
        }
    }
    

    SetSizerAndFit(sizer);
#   undef ITEMBORDER
}

void PropFile::readButtons(const std::shared_ptr<PConf::Section>& data, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PropFile::readButtons()")};

    if (not data->labelNum) {
      logger.warn("Button section missing number indicator, skipping...");
      return;
    }

    if (*data->labelNum < 0 || *data->labelNum > 3) {
      logger.warn("Button section number indicator \"" + std::to_string(*data->labelNum) + "\" out of range, skipping...");
      return;
    }

    for (const auto& entry : data->entries) {
        if (entry->getType() != PConf::Type::SECTION) continue;
        if (entry->name != "STATE") continue;
        if (not entry->label) {
            logger.warn("Button array #" + std::to_string(*data->labelNum) + " has unnamed state, skipping...");
            continue;
        }

        for (const auto& buttonEntry : std::static_pointer_cast<PConf::Section>(entry)->entries) {
            if (buttonEntry->getType() != PConf::Type::SECTION) continue;
            if (buttonEntry->name != "BUTTON") continue;
            if (not buttonEntry->label) {
                logger.warn("Button entry has missing name, skipping...");
                continue;
            }

            Button newButton;
            newButton.name = *buttonEntry->label;

            for (const auto descEntry : std::static_pointer_cast<PConf::Section>(buttonEntry)->entries) {
                if (descEntry->getType() != PConf::Type::ENTRY) continue;
                if (descEntry->name != "DESCRIPTION") continue;
                if (not descEntry->value) {
                    logger.warn("Description empty for button \"" + newButton.name + "\", ignoring");
                }

                if (not descEntry->label) {
                    if (newButton.descriptions.find({}) != newButton.descriptions.end()) {
                        logger.warn("Overriding duplicate default description for button \"" + newButton.name + "\", there should be only one default per button...");
                    }
                }

                newButton.descriptions.emplace(descEntry->label.value_or(""), descEntry->value.value_or("EMPTY"));
            }
        }
    }
}

