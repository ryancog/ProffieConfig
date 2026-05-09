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
#include "data/primitive/models/selector.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/types.hpp"

#include "../dialogs/presetarray.hpp"

struct PresetsPage : data::Receiver {
    PresetsPage(config::Config&);

    void onActivate() override;
    void onDeactivate() override;

    pcui::DescriptorPtr ui();

    [[nodiscard]] const data::prim::Selector& styleSel() const {
        return mStyleSel;
    }

private:
    pcui::DescriptorPtr selection();
    pcui::DescriptorPtr fields();
    pcui::DescriptorPtr displayAndBlade();
    pcui::DescriptorPtr style();

    void onArrayChoice();
    void onPresetChoice();
    void onDisplayChoice();

    void updateBladeStrings();

    config::Config& mConfig;

    data::prim::Selector mArraySel;
    data::prim::Selector mPresetSel;
    data::prim::Selector mDisplaySel;
    data::prim::Selector mStyleSel;
    
    std::vector<std::unique_ptr<data::prim::String>> mBladeStrings;

    PresetArrayDlg *mDlg{nullptr};
};

