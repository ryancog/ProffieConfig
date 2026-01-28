#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/bladeconfig/arrays.h
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

#include "ui/controls/checklist.h"
#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/numeric.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"
#include "ui/notifier.h"

#include "config/bladeconfig/bladeconfig.h"

namespace Config {

struct Config;

struct CONFIG_EXPORT BladeArrays {
    BladeArrays(Config&);

    // Do not set choices manually
    pcui::ChoiceData arraySelection;
    enum {
        ID_ARRAY_SELECTION,
        ID_ARRAY_ISSUES,
        ID_BLADE_SELECTION,
        ID_SPLIT_SELECTION,
        ID_BLADE_TYPE_SELECTION,
        ID_VISUAL_UPDATE
    };
    pcui::Notifier notifyData;

    // Issues for the current array, for UI
    uint32 arrayIssues{BladeConfig::ISSUE_NONE};

    pcui::ChoiceDataProxy bladeSelectionProxy;

    /**
     * @param clearIdx Index of preset that was deleted, and should be cleared if
     * a blade array is holding on to it.
     */
    void refreshPresetArrays(int32 clearIdx = -1);

    [[nodiscard]] const vector<std::unique_ptr<BladeConfig>>& arrays() const { return mBladeArrays; }
    [[nodiscard]] BladeConfig& array(uint32 idx) const {
        assert(idx < mBladeArrays.size());
        return **std::next(mBladeArrays.begin(), idx);
    }

    BladeConfig& addArray(string&& name = {}, int32 id = 0, const string& presetArray = {});
    void removeArray(uint32 idx);

    pcui::ChoiceDataProxy bladeTypeProxy;
    
    pcui::CheckListDataProxy powerPinProxy;
    pcui::TextData powerPinNameEntry;
    void addPowerPinFromEntry();

    pcui::ChoiceDataProxy colorOrder3Proxy;
    pcui::ChoiceDataProxy colorOrder4Proxy;
    pcui::ToggleDataProxy hasWhiteProxy;
    pcui::ToggleDataProxy useRGBWithWhiteProxy;

    pcui::NumericDataProxy pixelBrightnessProxy;

    pcui::ComboBoxDataProxy dataPinProxy;
    pcui::NumericDataProxy lengthProxy;

    pcui::Notifier visualizerData;

    pcui::ChoiceDataProxy splitSelectionProxy;
    pcui::RadiosDataProxy splitTypeProxy;
    pcui::NumericDataProxy splitStartProxy;
    pcui::NumericDataProxy splitEndProxy;
    pcui::NumericDataProxy splitLengthProxy;
    pcui::NumericDataProxy splitSegmentsProxy;
    pcui::TextDataProxy splitListProxy;
    pcui::NumericDataProxy splitBrightnessProxy;

    struct StarProxy {
        pcui::ChoiceDataProxy ledProxy;
        pcui::NumericDataProxy resistanceProxy;
        pcui::ComboBoxDataProxy powerPinProxy;
    };

    StarProxy star1Proxy;
    StarProxy star2Proxy;
    StarProxy star3Proxy;
    StarProxy star4Proxy;
    pcui::NumericDataProxy simpleBrightnessProxy;

    /**
     * @return number of blades across all arrays
     */
    [[nodiscard]] uint32 numBlades() const;

    void unbindBlade();

private:
    Config& mParent;
    vector<std::unique_ptr<BladeConfig>> mBladeArrays;
};

} // namespace Config

