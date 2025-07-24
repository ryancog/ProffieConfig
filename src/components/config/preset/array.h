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

#include "../private/export.h"
#include "preset.h"

namespace Config {

struct CONFIG_EXPORT Injection {
    string filename;
};

struct CONFIG_EXPORT PresetArray {
    PCUI::TextData name;
    PCUI::ChoiceData selection;

    [[nodiscard]] Preset& preset(uint32 idx) {
        return *std::next(mPresets.begin(), idx);
    };

    void addPreset();
    void removePreset(uint32);

    void movePresetUp(uint32);
    void movePresetDown(uint32);

private:
    list<Preset> mPresets;
};

struct CONFIG_EXPORT PresetArrays {
    PresetArrays();

    PCUI::ChoiceData selection;

    enum {
        NOTIFY_SELECTION,
        NOTIFY_INJECTIONS,
    };
    PCUI::NotifierData notifyData;

    [[nodiscard]] PresetArray& array(uint32 idx) { 
        return *std::next(mArrays.begin(), idx);
    }

    void addArray();
    void removeArray(uint32);

    [[nodiscard]] const list<Injection>& injections() const { return mInjections; }
    [[nodiscard]] Injection& injection(uint32 idx) {
        return *std::next(mInjections.begin(), idx);
    }

    void addInjection(const string&);
    void removeInjection(const Injection&);

    PCUI::ChoiceDataProxy presetProxy;

    PCUI::TextDataProxy nameProxy;
    PCUI::TextDataProxy dirProxy;
    PCUI::TextDataProxy trackProxy;

    PCUI::ChoiceDataProxy bladeProxy;

    PCUI::TextDataProxy commentProxy;
    PCUI::TextDataProxy styleProxy;

private:
    list<PresetArray> mArrays;
    list<Injection> mInjections;
};

} // namespace Config
