#include "propfile.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * prop/propfile.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <string>

#include "log/logger.h"
#include "pconf/pconf.h"
#include "config/settings.h"

PropFile::Data* readProp(const std::string& filename);

static Config::Setting::DefineMap* parsePropSettings(const PConf::Section& settingsSection);
static PropFile::Data::LayoutVec* parsePropLayout(const PConf::Section& layoutSection, const Config::Setting::DefineMap& settings);
static PropFile::Data::ButtonMap* parsePropButtons(const PConf::Section& buttonsSection, const Config::Setting::DefineMap* settings);

static void checkSettingCommon(const PConf::Entry& entry, Config::Setting::DefineBase& setting);
static void generateSettingCommon(Config::Setting::DefineBase& setting, const PConf::Section& section, const Config::Setting::DefineMap& settingMap);
static Config::Setting::Toggle<Config::Setting::DefineBase>* generateToggle(const PConf::Section& toggleSection, const Config::Setting::DefineMap& settings);
static std::vector<Config::Setting::Selection<Config::Setting::DefineBase>*> generateOptionSelections(const PConf::Section& optionSection, const Config::Setting::DefineMap& settings);
static Config::Setting::Numeric<Config::Setting::DefineBase>* generateNumeric(const PConf::Section& numericSection, const Config::Setting::DefineMap& settings);
static Config::Setting::Decimal<Config::Setting::DefineBase>* generateDecimal(const PConf::Section& decimalSection, const Config::Setting::DefineMap& settings);

static std::vector<PropFile::Data::Button*> parseButtonState(const std::vector<PConf::Entry*>& buttonEntries, const Config::Setting::DefineMap* settings);

PropFile::Data::~Data() {
    if (settings) {
        for (auto& setting : *settings) if (setting.second) delete setting.second;
        delete settings;
    }
    if (layout) {
        for (auto& item : *layout) if (item) delete item;
        delete layout;
    }
    for (auto& buttonMap : buttonControls) {
        if (buttonMap.second) delete buttonMap.second;
    }
}

std::vector<PropFile::Data*> PropFile::getPropData(const std::vector<std::string>& pConfs) {
    static std::unordered_map<std::string, PropFile::Data*> propData{};
    static std::unordered_set<std::string> propNames{};

    for (const auto& file : pConfs) {
        if (propData.find(file) != propData.end()) continue;

        auto prop{readProp(file)};
        if (!prop) continue;

        if (propNames.find(prop->name) != propNames.end()) {
            Logger::warn("Duplicate prop with name \"" + prop->name + "\" read, skipping!", false);
            continue;
        }

        propNames.insert(prop->name);
        propData.emplace(file, prop);
    }

    std::vector<PropFile::Data*> ret;
    for (const auto& [ filename, prop ] : propData) {
        ret.push_back(prop);
    }

    return ret;
}

PropFile::Data* readProp(const std::string& filename) {
    std::ifstream propFile(wxGetCwd().ToStdString() + PROPPATH + filename);
    if (!propFile.is_open()) {
        Logger::error("Prop file \"" + filename + "\" not found, skipping!", false);
        return nullptr;
    }

    auto prop{new PropFile::Data};
    bool foundSect;

    while (propFile.peek() != EOF) {
        std::unique_ptr<PConf::Entry> entry{PConf::readEntry(propFile, foundSect)};
        if (foundSect) {
            std::unique_ptr<PConf::Section> sect{PConf::readSection(propFile)};
            if (!sect) continue;
            if (sect->name == "SETTINGS") prop->settings = parsePropSettings(*sect);
            else if (sect->name == "LAYOUT") {
                if (!prop->settings) {
                    Logger::warn("Missing/out of order settings section, ignoring layout section!", false);
                    continue;
                }
                prop->layout = parsePropLayout(*sect, *prop->settings);
            } else if (sect->name == "BUTTONS") {
                auto buttonMap{parsePropButtons(*sect, prop->settings)};
                if (buttonMap) prop->buttonControls.emplace(buttonMap->numButton, buttonMap);
            }
        }
        if (!entry) continue;
        if (entry->name == "NAME") prop->name = entry->value.value_or("");
        if (entry->name == "FILENAME") prop->filename = entry->value.value_or("");
        if (entry->name == "INFO") prop->info = entry->value.value_or("");
    }

    return prop;
};

static Config::Setting::DefineMap* parsePropSettings(const PConf::Section& settingsSection) {
    auto settings{new Config::Setting::DefineMap};

    for (const auto& entry : settingsSection.entries) {
        if (entry->getType() != PConf::DataType::SECTION) {
            Logger::warn("Stray entry in settings! (" + entry->name + ")", false);
            continue;
        }


        if (entry->name == "TOGGLE") {
            auto toggle{generateToggle(*static_cast<PConf::Section*>(entry), *settings)};
            if (toggle) settings->emplace(toggle->define, toggle);
        } else if (entry->name == "OPTION") {
            for (const auto& selection : generateOptionSelections(*static_cast<PConf::Section*>(entry), *settings)) {
                settings->emplace(selection->define, selection);
            }
        } else if (entry->name == "NUMERIC") {
            auto numeric{generateNumeric(*static_cast<PConf::Section*>(entry), *settings)};
            if (numeric) settings->emplace(numeric->define, numeric);
        } else if (entry->name == "DECIMAL") {
            auto decimal{generateDecimal(*static_cast<PConf::Section*>(entry), *settings)};
            if (decimal) settings->emplace(decimal->define, decimal);
        }
    }

    return settings;
}

static void checkSettingCommon(const PConf::Entry& entry, Config::Setting::DefineBase& setting) {
    if (entry.name == "NAME") setting.name = entry.value.value_or("");
    else if (entry.name == "DESCRIPTION") setting.description = entry.value.value_or("");
    else if (entry.name == "REQUIRE" || entry.name == "REQUIREANY") {
        setting.require = PConf::setFromValue(entry.value);
        setting.requireAny = entry.name == "REQUIREANY";
    }
}
static void generateSettingCommon(Config::Setting::DefineBase& setting, const PConf::Section& section, const Config::Setting::DefineMap& settingMap) {
    setting.define = section.label.value();
    setting.group = settingMap;
}

static Config::Setting::Toggle<Config::Setting::DefineBase>* generateToggle(const PConf::Section& toggleSection, const Config::Setting::DefineMap& settingMap) {
    if (!toggleSection.label) {
        Logger::warn("Define has no define, skipping!");
        return nullptr;
    }

    auto toggle{new Config::Setting::Toggle<Config::Setting::DefineBase>};
    generateSettingCommon(*toggle, toggleSection, settingMap);
    for (const auto& entry : toggleSection.entries) {
        checkSettingCommon(*entry, *toggle);
        if (entry->name == "DISABLE") toggle->disable = PConf::setFromValue(entry->value);
    }

    return toggle;
}

static std::vector<Config::Setting::Selection<Config::Setting::DefineBase>*> generateOptionSelections(const PConf::Section& optionSection, const Config::Setting::DefineMap& settingMap) {
    std::vector<Config::Setting::Selection<Config::Setting::DefineBase>*> selections;
    for (const auto& entry : optionSection.entries) {
        if (entry->name == "SELECTION" && entry->getType() == PConf::DataType::SECTION) {
            if (!entry->label) {
                Logger::warn("Define has no define, skipping!");
                continue;
            }

            auto selection{new Config::Setting::Selection<Config::Setting::DefineBase>};
            generateSettingCommon(*selection, *static_cast<PConf::Section*>(entry), settingMap);
            for (const auto selectionEntry : static_cast<PConf::Section*>(entry)->entries) {
                checkSettingCommon(*selectionEntry, *selection);
                if (entry->name == "DISABLE") selection->disable = PConf::setFromValue(entry->value);
                if (selectionEntry->name == "OUTPUT") selection->output = selectionEntry->value.value_or("TRUE") == "FALSE" ? false : true;
            }

            selections.push_back(selection);
        }
    }

    for (const auto& selection : selections) {
        for (const auto& peer : selections) {
            if (peer == selection) continue;
            selection->peers.insert(peer);
        }
    }

    return selections;
}

static auto isNum = [](const std::string& str) -> bool {
    if (str.empty()) return false;
    if (str.length() == 1) return std::isdigit(str.at(0));

    return (std::isdigit(str.at(0)) || (str.at(0) == '-' && std::isdigit(str.at(1))));
};


static Config::Setting::Numeric<Config::Setting::DefineBase>* generateNumeric(const PConf::Section& numericSection, const Config::Setting::DefineMap& settingMap) {
    if (!numericSection.label) {
        Logger::warn("Define has no define, skipping!");
        return nullptr;
    }

    auto numeric{new Config::Setting::Numeric<Config::Setting::DefineBase>};
    generateSettingCommon(*numeric, numericSection, settingMap);
    for (const auto entry : numericSection.entries) {
        checkSettingCommon(*entry, *numeric);
        const auto& val = entry->value.value_or(" ");
        if (entry->name == "MIN" && isNum(val)) numeric->min = stoi(val);
        if (entry->name == "MAX" && isNum(val)) numeric->max = stoi(val);
        if (entry->name == "DEFAULT" && isNum(val)) numeric->value = stoi(val);
        if (entry->name == "INCREMENT" && isNum(val)) numeric->increment = stoi(val);
    }

    return numeric;
}

static Config::Setting::Decimal<Config::Setting::DefineBase>* generateDecimal(const PConf::Section& decimalSection, const Config::Setting::DefineMap& settingMap) {
    if (!decimalSection.label) {
        Logger::warn("Define has no define, skipping!");
        return nullptr;
    }

    auto decimal{new Config::Setting::Decimal<Config::Setting::DefineBase>};
    generateSettingCommon(*decimal, decimalSection, settingMap);
    for (const auto entry : decimalSection.entries) {
        checkSettingCommon(*entry, *decimal);
        const auto& val = entry->value.value_or(" ");
        if (entry->name == "MIN" && isNum(val)) decimal->min = stod(val);
        if (entry->name == "MAX" && isNum(val)) decimal->max = stod(val);
        if (entry->name == "DEFAULT" && isNum(val)) decimal->value = stod(val);
        if (entry->name == "INCREMENT" && isNum(val)) decimal->increment = stod(val);
    }

    return decimal;
}

static PropFile::Data::LayoutVec* parsePropLayout(const PConf::Section& layoutSection, const Config::Setting::DefineMap& settings) {
    auto items{new PropFile::Data::LayoutVec};

    for (const auto entry : layoutSection.entries) {
        if (entry->name == "SETTING") {
            if (!entry->label) {
                Logger::warn("Layout entry missing label, skipping!", false);
                continue;
            }
            const auto& setting{settings.find(entry->label.value())};
            if (setting == settings.end()) {
                Logger::warn("Layout entry has unknown setting, skipping! (" + entry->label.value() + ")", false);
                continue;
            }
            auto layoutItem{new PropFile::Data::LayoutItem};
            layoutItem->setting = setting->second;
            items->push_back(layoutItem);
        } else if (entry->name == "HORIZONTAL") {
            if (entry->getType() != PConf::DataType::SECTION) {
                Logger::warn("Horizontal layout section interpreted as entry due to syntax error, skipping!" + (entry->label ? " (" + entry->label.value() + ")" : ""), false);
                continue;
            }
            auto layoutLevel{new PropFile::Data::LayoutLevel};
            layoutLevel->direction = PropFile::Data::LayoutLevel::Direction::HORIZONTAL;
            layoutLevel->label = entry->label.value_or("");
            layoutLevel->items = parsePropLayout(*static_cast<PConf::Section*>(entry), settings);
            items->push_back(layoutLevel);
        } else if (entry->name == "VERTICAL") {
            if (entry->getType() != PConf::DataType::SECTION) {
                Logger::warn("Vertical layout section interpreted as entry due to syntax error, skipping!" + (entry->label ? " (" + entry->label.value() + ")" : ""), false);
                continue;
            }
            auto layoutLevel{new PropFile::Data::LayoutLevel};
            layoutLevel->direction = PropFile::Data::LayoutLevel::Direction::VERTICAL;
            layoutLevel->label = entry->label.value_or("");
            layoutLevel->items = parsePropLayout(*static_cast<PConf::Section*>(entry), settings);
            items->push_back(layoutLevel);
        }
    }

    return items;
}

static PropFile::Data::ButtonMap* parsePropButtons(const PConf::Section& buttonsSection, const Config::Setting::DefineMap* settings) {
    if (buttonsSection.labelNum.value_or(-1) < 0) {
        Logger::warn("Button section missing numeric label, ignoring!", false);
        return nullptr;
    }

    auto buttonMap{new PropFile::Data::ButtonMap};
    buttonMap->numButton = buttonsSection.labelNum.value();

    for (const auto& stateEntry : buttonsSection.entries) {
        if (stateEntry->name == "STATE") {
            if (!stateEntry->label) {
                Logger::warn("Button state section missing label, skipping!", false);
                continue;
            }
            if (stateEntry->getType() != PConf::DataType::SECTION) {
                Logger::warn("Button state \"" + stateEntry->label.value() + "\" interpreted as entry due to syntax error, skipping!", false);
                continue;
            }

            auto state{new PropFile::Data::ButtonState};
            state->label = stateEntry->label.value();
            state->buttons = parseButtonState(static_cast<PConf::Section*>(stateEntry)->entries, settings);

            buttonMap->states.insert(state);
        }
    }


    return buttonMap;
}

static std::vector<PropFile::Data::Button*> parseButtonState(const std::vector<PConf::Entry*>& buttonEntries, const Config::Setting::DefineMap* settings) {
    std::vector<PropFile::Data::Button*> buttons;
    for (const auto& buttonEntry : buttonEntries) {
        if (!buttonEntry->label) {
            Logger::warn("Button section missing label, skipping!", false);
            continue;
        }
        if (buttonEntry->getType() != PConf::DataType::SECTION) {
            Logger::warn("Button section \"" + buttonEntry->label.value() + "\" interpreted as entry due to syntax error, skipping!", false);
            continue;
        }

        auto button{new PropFile::Data::Button};
        button->label = buttonEntry->label.value();

        for (const auto& description : static_cast<PConf::Section*>(buttonEntry)->entries) {
            if (!settings) {
                button->descriptions.emplace(nullptr, description->value.value_or(""));
                break;
            }

            const auto& setting{settings->find(description->label.value_or(""))};
            button->descriptions.emplace((setting == settings->end() ? nullptr : setting->second), description->value.value_or(""));
        }

        buttons.push_back(button);
    }

    return buttons;
}
