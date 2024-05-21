#include "stylemanager.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * stylemanager/stylemanager.cpp
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

#include "log/logger.h"
#include "pconf/pconf.h"
#include "styles/parse.h"
#include "ui/frame.h"
#include "appcore/interfaces.h"

#define STYLE_FILENAME "styles.pconf"

static PCUI::Frame* interface{nullptr};

void StyleManager::launch(wxWindow* parent) {
    if (interface) {
        interface->Raise();
        return;
    }

    interface = new PCUI::Frame(parent, AppCore::Interface::STYLEMAN, "ProffieConfig Style Manager");


    interface->Show();
}

StyleManager::StyleMap* StyleManager::loadStyles() {
    std::ifstream styleFile{RESOURCEPATH STYLE_FILENAME};
    if (!styleFile.is_open()) {
        Logger::info("Could not open styles config file.");
        return nullptr;
    }

    auto styles{new StyleMap};

    PConf::Section* section;
    while (!styleFile.eof()) {
        section = PConf::readSection(styleFile);
        if (!section) continue;
        if (section->name != "STYLEPRESET") continue;
        if (!section->label) {
            Logger::warn("Style Preset missing name, ignoring!");
            continue;
        }
        if (styles->find(section->label.value()) != styles->end()) {
            Logger::warn("Ignoring style with duplicate name: \"" + section->label.value() + "\"");
            continue;
        }

        std::string* styleStr{nullptr};
        for (const auto& entry : section->entries) {
            if (entry->value && entry->name == "STYLE") styleStr = &entry->value.value();
        }
        if (!styleStr) {
            Logger::warn("Style entry missing style!");
            continue;
        }

        auto style{BladeStyles::parseString(*styleStr)};
        if (!style) {
            Logger::warn("Error parsing style for preset \"" + section->name + "\"");
            continue;
        }

        Preset stylePreset{
            .name = section->label.value(),
            .style = style,
        };
        styles->insert({ stylePreset.name, stylePreset });
    }

    styleFile.close();

    return styles;
}

void StyleManager::saveStyles(const StyleManager::StyleMap& styles) {
    std::ofstream styleFile{RESOURCEPATH STYLE_FILENAME};
    if (!styleFile.is_open()) {
        Logger::info("Could not open styles config file.");
        return;
    }

    for (const auto& [ styleName, style ] : styles) {
        auto section{new PConf::Section};
        section->name = "STYLEPRESET";
        section->label = style.name;
        auto styleEntry{new PConf::Entry};
        styleEntry->name = "STYLE";
        styleEntry->value = BladeStyles::asString(*style.style);
        section->entries.push_back(styleEntry);
        PConf::writeSection(styleFile, *section);
    }

    styleFile.close();
}
