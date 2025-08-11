#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * components/config/config.h
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

#include <wx/datetime.h>

#include "versions/prop.h"
#include "preset/array.h"
#include "bladeconfig/arrays.h"
#include "private/export.h"
#include "settings/settings.h"

namespace Config {

static constexpr cstring RAW_FILE_EXTENSION{".h"};

static constexpr auto MAX_NAME_LENGTH{24};

struct CONFIG_EXPORT Config {
    PCUI::TextData name;
    Settings settings;

    PCUI::ChoiceData propSelection;
    enum {
        ID_PROPSELECTION,
    };
    PCUI::NotifierData propNotifyData;
    [[nodiscard]] const vector<std::unique_ptr<Versions::Prop>>& props() const { return mProps; }
    [[nodiscard]] Versions::Prop& prop(uint32 idx) const {
        return *mProps[idx];
    }

    PresetArrays presetArrays;

    BladeArrays bladeArrays;

    /**
     * Refresh OS and Prop Version(s) and update accordingly
     */
    void refreshVersions();
    void refreshPropVersions();

    void rename(const string& newName);

    bool isSaved();
    /**
     * @return error or nullopt on success
     */
    optional<string> save(const filepath& = {});

    /**
     * Remove from internal storage and let it die once last memory
     * is forgotten...
     */
    void close();

private:
    friend variant<Config *, string> open(const string&);
    friend optional<string> import(const string&, const filepath&);
    Config();

    vector<std::unique_ptr<Versions::Prop>> mProps;
};

/**
 * Search disk and retrieve list of all config names
 */
CONFIG_EXPORT vector<string> fetchListFromDisk();

/**
 * @return List of configs currently open
 */
CONFIG_EXPORT const vector<std::unique_ptr<Config>>& getOpen();

CONFIG_EXPORT bool remove(const string& name);

/**
 * Parse the config and return a fresh ptr, or return the ptr
 * of an already-open config.
 *
 * @param name Config name
 *
 * @return Config or error message
 */
CONFIG_EXPORT variant<Config *, string> open(const string& name);

/**
 * Similar to open, but opens from path instead of in save folder by name.
 *
 * @return err or nullopt
 */
CONFIG_EXPORT optional<string> import(const string& name, const filepath& path);

/**
 * @return the config with name only if config is open
 * nullptr otherwise
 */
CONFIG_EXPORT Config *getIfOpen(const string& name);

} // namespace Config

