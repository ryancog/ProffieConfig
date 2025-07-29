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
        ID_SUBBLADE_SELECTION,
        ID_BLADE_TYPE_SELECTION,
    };
    PCUI::NotifierData notifyData;

    // Issues for the current array, for UI
    uint32 arrayIssues{BladeConfig::ISSUE_NONE};

    PCUI::ChoiceDataProxy bladeSelectionProxy;
    PCUI::ChoiceDataProxy subBladeSelectionProxy;

    PCUI::ChoiceDataProxy bladeTypeProxy;

    /**
     * @param clearIdx Index of preset that was deleted, and should be cleared if
     * a blade array is holding on to it.
     */
    void refreshPresetArrays(int32 clearIdx = -1);

    [[nodiscard]] const vector<std::unique_ptr<BladeConfig>>& arrays() const { return mBladeArrays; }
    [[nodiscard]] BladeConfig& array(uint32 idx) { 
        assert(idx < mBladeArrays.size());
        return **std::next(mBladeArrays.begin(), idx);
    }

    BladeConfig& addArray(string&& name = {}, uint32 id = 0, string presetArray = {});
    void removeArray(uint32 idx);
    
    PCUI::CheckListDataProxy powerPinProxy;
    PCUI::TextData powerPinNameEntry;
    void addPowerPinFromEntry();

    PCUI::ChoiceDataProxy colorOrder3Proxy;
    PCUI::ChoiceDataProxy colorOrder4Proxy;
    PCUI::ToggleDataProxy hasWhiteProxy;
    PCUI::ToggleDataProxy useRGBWithWhiteProxy;

    PCUI::ComboBoxDataProxy dataPinProxy;
    PCUI::NumericDataProxy lengthProxy;

    PCUI::RadiosDataProxy subBladeTypeProxy;
    PCUI::NumericDataProxy subBladeLengthProxy;
    PCUI::NumericDataProxy subBladeSegmentsProxy;

    struct StarProxy {
        PCUI::ChoiceDataProxy ledProxy;
        PCUI::NumericDataProxy resistanceProxy;
        PCUI::ComboBoxDataProxy powerPinProxy;
    };

    StarProxy star1Proxy;
    StarProxy star2Proxy;
    StarProxy star3Proxy;
    StarProxy star4Proxy;

    /**
     * @return number of blades across all arrays
     */
    uint32 numBLades();

private:
    Config& mParent;
    vector<std::unique_ptr<BladeConfig>> mBladeArrays;
};

} // namespace Config

