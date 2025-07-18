#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/preset/preset.h
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

#include "ui/controls/text.h"
#include "utils/types.h"

#include "../private/export.h"

namespace Config {

struct CONFIG_EXPORT Preset {
    PCUI::TextData name;
    PCUI::TextData fontDir;
    // vector<string> fontDirs;
    PCUI::TextData track;
    struct Style {
        Style();
        PCUI::TextData comment; // {"ProffieConfig Default Blue AudioFlicker"};
        PCUI::TextData style; // {"StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()"};
    };
    vector<Style> styles;
    // vector<UID> styles;
};

} // namespace Config

