#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
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

#include "config/bladeconfig/bladeconfig.h"
#include "ui/notifier.h"

namespace Config {

struct Config;

struct CONFIG_EXPORT BladeArrays {
    BladeArrays(Config&);

    // Do not set choices manually
    PCUI::ChoiceData arraySelection;
    enum {
        ID_ARRAY_SELECTION,
        ID_ARRAY_ISSUES,
        ID_BLADE_SELECTION,
        ID_SPLIT_SELECTION,
        ID_BLADE_TYPE_SELECTION,
    };
    PCUI::Notifier notifyData;

    // Issues for the current array, for UI
    uint32 arrayIssues{BladeConfig::ISSUE_NONE};

    PCUI::ChoiceDataProxy bladeSelectionProxy;

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

    BladeConfig& addArray(string&& name = {}, uint32 id = 0, string presetArray = {});
    void removeArray(uint32 idx);

    PCUI::ChoiceDataProxy bladeTypeProxy;
    
    PCUI::CheckListDataProxy powerPinProxy;
    PCUI::TextData powerPinNameEntry;
    void addPowerPinFromEntry();

    PCUI::ChoiceDataProxy colorOrder3Proxy;
    PCUI::ChoiceDataProxy colorOrder4Proxy;
    PCUI::ToggleDataProxy hasWhiteProxy;
    PCUI::ToggleDataProxy useRGBWithWhiteProxy;

    PCUI::NumericDataProxy pixelBrightnessProxy;

    PCUI::ComboBoxDataProxy dataPinProxy;
    PCUI::NumericDataProxy lengthProxy;

    PCUI::Notifier visualizerData;

    PCUI::ChoiceDataProxy splitSelectionProxy;
    PCUI::RadiosDataProxy splitTypeProxy;
    PCUI::NumericDataProxy splitStartProxy;
    PCUI::NumericDataProxy splitEndProxy;
    PCUI::NumericDataProxy splitLengthProxy;
    PCUI::NumericDataProxy splitSegmentsProxy;
    PCUI::TextDataProxy splitListProxy;
    PCUI::NumericDataProxy splitBrightnessProxy;

    struct StarProxy {
        PCUI::ChoiceDataProxy ledProxy;
        PCUI::NumericDataProxy resistanceProxy;
        PCUI::ComboBoxDataProxy powerPinProxy;
    };

    StarProxy star1Proxy;
    StarProxy star2Proxy;
    StarProxy star3Proxy;
    StarProxy star4Proxy;
    PCUI::NumericDataProxy simpleBrightnessProxy;

    /**
     * @return number of blades across all arrays
     */
    uint32 numBlades() const;

    void unbindBlade();

private:
    Config& mParent;
    vector<std::unique_ptr<BladeConfig>> mBladeArrays;
};

} // namespace Config

