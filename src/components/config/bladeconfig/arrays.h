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

#include "config/bladeconfig/bladeconfig.h"

namespace Config {

struct BladeArrays {
    PCUI::ChoiceData bladeArraySelection;
    PCUI::ChoiceDataProxy bladeSelection;
    PCUI::ChoiceDataProxy subBladeSelection;
    PCUI::ChoiceDataProxy bladeTypeSelection;

    void addArray();
    void removeArray(uint32 idx);
    
    PCUI::CheckListDataProxy poewrPinSelection;
    PCUI::TextData powerPinNameEntry;
    void addPowerPinFromEntry();

    PCUI::ChoiceDataProxy colorOrder3;
    PCUI::ChoiceDataProxy colorOrder4;
    PCUI::ComboBoxDataProxy hasWhite;

    struct StarProxy {
        PCUI::ChoiceDataProxy led;
        PCUI::NumericDataProxy resistance;
        PCUI::ComboBoxDataProxy powerPin;
    };

    StarProxy star1;
    StarProxy star2;
    StarProxy star3;
    StarProxy star4;

private:
    vector<BladeConfig> mBladeArrays;

};

} // namespace Config

