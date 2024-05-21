#include "state.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * appcore/state.cpp
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

#include "stylemanager/stylemanager.h"
#include "prop/propfile.h"
#include "pconf/pconf.h"
#include "log/logger.h"

static StyleManager::StyleMap* styles;
static std::vector<std::string>* propFiles;
static std::vector<PropFile::Data*>* propData;
static std::vector<std::string>* configFiles;
static bool isInitialized{false};
static bool firstRun{true};

void AppCore::init() {
    if (isInitialized) return;

    Logger::init();
    styles = StyleManager::loadStyles();
    propFiles = new std::vector<std::string>;
    configFiles = new std::vector<std::string>;
    propData = new std::vector<PropFile::Data*>;

    std::ifstream stateFile(RESOURCEPATH ".state.pconf");
    if (!stateFile.is_open()) {
        Logger::info("Could not load state file, continuing without!");
        propFiles->assign({ "fett263.pconf", "sa22c.pconf", "shtok.pconf", "BC.pconf", "caiwyn.pconf" });
        (*propData) = PropFile::getPropData(*propFiles);
        return;
    }

    PConf::Section* section;
    while (!stateFile.eof()) {
        section = PConf::readSection(stateFile);
        if (!section) continue;

        if (section->name == "FIRSTRUN") {
            firstRun = section->value.value_or("TRUE") == "TRUE";
        } else if (section->name == "PROPS") {
            for (const auto& entry : section->entries) if (entry->label) propFiles->push_back(entry->label.value());
        } else if (section->name == "CONFIGS") {
            for (const auto& entry : section->entries) if (entry->label) configFiles->push_back(entry->label.value());
        }
    }

    (*propData) = PropFile::getPropData(*propFiles);

    stateFile.close();
}

decltype(styles) AppCore::getStyles() { return styles; }
decltype(propData) AppCore::getPropData() { return propData; }
decltype(propFiles) AppCore::getPropFiles() { return propFiles; }
decltype(configFiles) AppCore::getConfigFiles() { return configFiles; }

