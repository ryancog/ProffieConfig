#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/preset/preset.h
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
#include "utils/types.h"

#include "../private/export.h"

namespace Config {

struct Config;
struct PresetArray;

struct CONFIG_EXPORT Preset {
    Preset(Config&, PresetArray&);

    PCUI::TextData name;
    PCUI::TextData fontDir;
    // vector<string> fontDirs;
    PCUI::TextData track;

    struct Style {
        Style();
        PCUI::TextData comment;
        PCUI::TextData style;
    };

    // No set choice manual
    PCUI::ChoiceData styleSelection;
    PCUI::ChoiceData styleDisplay;

    [[nodiscard]] Style& style(uint32 idx) {
        assert(idx < mStyles.size());
        return *std::next(mStyles.begin(), idx);
    }

    void syncStyles();

private:
    Config& mConfig;
    PresetArray& mParent;

    // Never shrinks, so data isn't lost.
    // At worst it's jumbled by moving blades
    list<Style> mStyles;
};

} // namespace Config

