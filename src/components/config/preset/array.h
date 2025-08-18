#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/preset/array.h
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


#include "ui/controls/choice.h"
#include "ui/controls/text.h"
#include "ui/notifier.h"

#include "preset.h"
#include "config_export.h"

namespace Config {

struct Config;

struct CONFIG_EXPORT Injection {
    Injection(const string& name) : filename{name} {}
    string filename;
};

struct CONFIG_EXPORT PresetArray {
    PresetArray(Config&);

    PCUI::TextData name;
    PCUI::ChoiceData selection;

    // Notifies on duplicate update
    PCUI::Notifier notifyData;

    [[nodiscard]] const vector<std::unique_ptr<Preset>>& presets() const { return mPresets; }
    [[nodiscard]] Preset& preset(uint32 idx) const {
        return **std::next(mPresets.begin(), idx);
    };

    Preset& addPreset();
    void removePreset(uint32);

    void movePresetUp(uint32);
    void movePresetDown(uint32);

private:
    Config& mConfig;
    vector<std::unique_ptr<Preset>> mPresets;
};

struct CONFIG_EXPORT PresetArrays {
    PresetArrays(Config&);

    PCUI::ChoiceData selection;

    enum {
        /**
         * New/removed array selection
         */
        NOTIFY_SELECTION,
        /**
         * New/removed preset selection or choices changed
         */
        NOTIFY_PRESETS,
        /**
         * Injections modified
         */
        NOTIFY_INJECTIONS,
        /**
         * Track modified
         */
        NOTIFY_TRACK_INPUT,
        /**
         * Window should focus to comments entry
         */
        NOTIFY_FOCUS_COMMENTS,
        /**
         * Selected array name status change
         */
        NOTIFY_ARRAY_NAME,
    };
    PCUI::Notifier notifyData;

    [[nodiscard]] const vector<std::unique_ptr<PresetArray>>& arrays() const { return mArrays; }
    [[nodiscard]] PresetArray& array(uint32 idx) { 
        assert(idx < mArrays.size());
        return **std::next(mArrays.begin(), idx);
    }

    PresetArray& addArray(string name);
    void removeArray(uint32 idx);

    [[nodiscard]] const vector<std::unique_ptr<Injection>>& injections() const { return mInjections; }
    [[nodiscard]] Injection& injection(uint32 idx) {
        return **std::next(mInjections.begin(), idx);
    }

    void addInjection(const string&);
    void removeInjection(const Injection&);

    PCUI::ChoiceDataProxy presetProxy;

    PCUI::TextDataProxy nameProxy;
    PCUI::TextDataProxy dirProxy;
    PCUI::TextDataProxy trackProxy;

    PCUI::ChoiceData styleDisplay;
    PCUI::ChoiceDataProxy styleSelectProxy;

    PCUI::TextDataProxy commentProxy;
    PCUI::TextDataProxy styleProxy;
    static PCUI::TextData dummyCommentData;
    static PCUI::TextData dummyStyleData;

    /**
     * Increase number of styles to match number of blades
     * Also syncs display
     */
    void syncStyles();

    /**
     * Sync all display choice and preset style displays.
     */
    void syncStyleDisplay(int32 clearIdx = -1);

private:
    Config& mParent;
    vector<std::unique_ptr<PresetArray>> mArrays;
    vector<std::unique_ptr<Injection>> mInjections;
};

} // namespace Config
