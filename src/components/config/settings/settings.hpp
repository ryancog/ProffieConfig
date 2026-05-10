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

#include "config/settings/bladeawareness.hpp"
#include "data/hierarchic/model.hpp"
#include "data/hierarchic/models/bool.hpp"
#include "data/hierarchic/models/choice.hpp"
#include "data/hierarchic/models/number.hpp"
#include "data/hierarchic/models/vector.hpp"

#include "config_export.h"

namespace config {

struct Config;

struct CONFIG_EXPORT Settings : data::hier::Model, data::Receiver {
    struct ProcessDefinesAction;

    Settings(Config&);
    ~Settings() override;

    void onActivate() override;
    std::vector<Model *> children() override;

    data::hier::Bool massStorage_;
    data::hier::Bool webUsb_;

    // pcui::ChoiceData rfidSerial;

    settings::BladeAwareness bladeAwareness_;

    data::hier::Integer volume_;

    struct {
        data::hier::Bool enable_;
        data::hier::Integer value_;
    } bootVolume_;

    // Does not affect I2C or S/PDIF
    // Cutoff in Hz
    struct {
        data::hier::Bool enable_;
        data::hier::Integer cutoff_;
        data::hier::Integer order_;
    } filter_;

    data::hier::Decimal clashThreshold_;

    // In seconds 
    data::hier::Decimal pliOffTime_;
    // In Minutes
    data::hier::Decimal idleOffTime_;
    data::hier::Decimal motionTimeout_;

    data::hier::Bool disableColorChange_;
    data::hier::Bool disableBasicParserStyles_;
    data::hier::Bool disableTalkie_;
    data::hier::Bool disableDiagnosticCommands_;
    // pcui::ToggleData enableDeveloperCommands;

    // SAVE_STATE Sets:
    // - saveVolume
    // - savePreset
    // - saveColorChange
    // - saveBladeDimming (only if dynamicBladeDimming is set)
    data::hier::Bool saveState_;

    // ENABLE_ALL_EDIT_OPTIONS Sets:
    // - dynamicBladeLength
    // - dynamicBladeDimming
    // - dynamicClashThreshold
    // - saveVolume
    // - saveColorChange
    // - saveBladeDimming
    // - saveClashThreshold
    data::hier::Bool enableAllEditOptions_;

    data::hier::Bool saveVolume_;
    data::hier::Bool savePreset_;
    data::hier::Bool saveColorChange_;

    data::hier::Bool enableOled_;

    data::hier::Choice orientation_;

    struct OrientRotation {
        data::hier::Integer x_;
        data::hier::Integer y_;
        data::hier::Integer z_;
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

    data::hier::Bool dynamicBladeDimming_;
    data::hier::Bool dynamicBladeLength_;
    data::hier::Bool dynamicClashThreshold_;

    // only should be settable if dynamicBladeDimming is true
    data::hier::Bool saveBladeDimming_;
    data::hier::Bool saveClashThreshold_;

    // Useful range is 1~50
    data::hier::Integer audioClashSuppressionLevel_;
    data::hier::Bool dontUseGyroForClash_;

    data::hier::Bool noRepeatRandom_;
    data::hier::Bool femaleTalkie_;
    data::hier::Bool killOldPlayers_;

    // POV Data?

    void processDefines();

    data::hier::Vector defines_;

protected:
    void onSaveOptSet();
    void onVolume();
    void onBootVolumeEnable();
    void onFilterEnableSet();
    void onDisableTalkieSet();
};

/**
 * Look through the defines and see if any should be translated into
 * ProffieConfig-managed settings.
 */
struct CONFIG_EXPORT Settings::ProcessDefinesAction : data::hier::Action {
    ProcessDefinesAction();

    bool setup() override;
    void perform() override;
    void retract() override;
};

} // namespace config

