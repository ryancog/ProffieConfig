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
#include "data/selection.hpp"
#include "data/string.hpp"
#include "data/vector.hpp"
#include "logging/branch.hpp"
#include "utils/types.hpp"
#include "utils/version.hpp"

#include "config_export.h"

namespace config {

struct Config;

struct CONFIG_EXPORT Settings {
    /**
     * Set up defaults and update handlers
     */
    Settings(Config&);

    data::Choice board_;

    data::Choice osVersion_;

    data::Bool massStorage_;
    data::Bool webUsb_;

    data::Vector buttons_;

    // pcui::ChoiceData rfidSerial;

    struct BladeDetect {
        data::Bool enable_;
        data::String pin_;
    } bladeDetect_;

    struct BladeID {
        data::Bool enable_;
        data::String pin_;

        data::Choice mode_;
        data::String bridgePin_;
        data::Integer pullup_;

        data::Bool powerForId_;
        data::Selection powerPins_;

        data::Bool continuousScanning_;
        data::Bool continuousInterval_;
        data::Bool continuousTimes_;
    } bladeId_;

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

    struct CustomOption {
        data::String define_;
        data::String value_;
    };
    data::Vector customOpts_;
};

} // namespace config

