#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/presets.hpp
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

#include <wx/window.h>

#include "config/config.hpp"
#include "data/selector.hpp"
#include "ui/types.hpp"

#include "../dialogs/presetarray.hpp"

struct PresetsPage {
    PresetsPage(config::Config&);

    pcui::DescriptorPtr ui();

    [[nodiscard]] const data::Selector& styleSel() const {
        return mStyleSel;
    }

private:
    pcui::DescriptorPtr selection();
    pcui::DescriptorPtr fields();
    pcui::DescriptorPtr displayAndBlade();
    pcui::DescriptorPtr style();

    void updateBladeStrings();

    config::Config& mConfig;

    data::Selector mArraySel;
    data::Selector mPresetSel;
    data::Selector mDisplaySel;
    data::Selector mStyleSel;
    
    std::vector<data::String> mBladeStrings;

    PresetArrayDlg *mDlg{nullptr};
};

