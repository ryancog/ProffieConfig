#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/config/settings/settings.hpp
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

#include "data/bool.hpp"
#include "data/choice.hpp"
#include "data/number.hpp"
#include "data/vector.hpp"
#include "config/settings/bladeawareness.hpp"

#include "config_export.h"

namespace config {

struct Config;

struct CONFIG_EXPORT Settings : data::Node {
    struct ProcessDefinesAction;

    Settings(Config&);
    ~Settings() override;

    bool enumerate(const EnumFunc&) override;
    [[nodiscard]] Model *find(uint64) override;

    data::Bool massStorage_;
    data::Bool webUsb_;

    // pcui::ChoiceData rfidSerial;

    settings::BladeAwareness bladeAwareness_;

    data::Integer volume_;
    data::Bool enableBootVolume_;
    data::Integer bootVolume_;

    // Does not affect I2C or S/PDIF
    // Cutoff in Hz
    data::Bool enableFiltering_;
    data::Integer filterCutoff_;
    data::Integer filterOrder_;

    data::Decimal clashThreshold_;

    // In seconds 
    data::Decimal pliOffTime_;
    // In Minutes
    data::Decimal idleOffTime_;
    data::Decimal motionTimeout_;

    data::Bool disableColorChange_;
    data::Bool disableBasicParserStyles_;
    data::Bool disableTalkie_;
    data::Bool disableDiagnosticCommands_;
    // pcui::ToggleData enableDeveloperCommands;

    // SAVE_STATE Sets:
    // - saveVolume
    // - savePreset
    // - saveColorChange
    // - saveBladeDimming (only if dynamicBladeDimming is set)
    data::Bool saveState_;

    // ENABLE_ALL_EDIT_OPTIONS Sets:
    // - dynamicBladeLength
    // - dynamicBladeDimming
    // - dynamicClashThreshold
    // - saveVolume
    // - saveColorChange
    // - saveBladeDimming
    // - saveClashThreshold
    data::Bool enableAllEditOptions_;

    data::Bool saveVolume_;
    data::Bool savePreset_;
    data::Bool saveColorChange_;

    data::Bool enableOled_;

    data::Choice orientation_;

    struct OrientRotation {
        data::Integer x_;
        data::Integer y_;
        data::Integer z_;
    } orientationRotation_;

    // For debugging touch buttons:
    // pcui::ToggleData speakTouchValues;
    // constexpr static cstring SPEAK_TOUCH_VALUES_STR{"SPEAK_TOUCH_VALUES"};
    
    // Make these be enabled by adding such a device to the
    // wiring. Adjust volume there too? That I don't know...
    /*
    bool enableI2SOut{false};
    bool enableSPDIFOut{false};
    int32 lineOutVolume{2000}; // This can be set to `dynamic_mixer.get_volume()` to follow master
    */

    data::Bool dynamicBladeDimming_;
    data::Bool dynamicBladeLength_;
    data::Bool dynamicClashThreshold_;

    // only should be settable if dynamicBladeDimming is true
    data::Bool saveBladeDimming_;
    data::Bool saveClashThreshold_;

    // Useful range is 1~50
    data::Integer audioClashSuppressionLevel_;
    data::Bool dontUseGyroForClash_;

    data::Bool noRepeatRandom_;
    data::Bool femaleTalkie_;
    data::Bool killOldPlayers_;

    // POV Data?

    void processDefines();

    data::Vector defines_;
};

/**
 * Look through the defines and see if any should be translated into
 * ProffieConfig-managed settings.
 */
struct CONFIG_EXPORT Settings::ProcessDefinesAction : data::Action {
    ProcessDefinesAction();

    bool shouldPerform(data::Model&) override;
    void perform(data::Model&) override;
    void retract(data::Model&) override;
};

} // namespace config

