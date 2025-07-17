#include "settings.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/settings/settings.h
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

Config::Settings::Settings() {
    // Asign update handlers


    // Set defaults
    volume = 1000;
    enableBootVolume = false;
    clashThreshold = 3.0;

    pliOffTime = 10;
    idleOffTime = 10;
    motionOffTime = 15;

    disableColorChange = false;
    disableBasicParserStyles = false;
    disableDiagnosticCommands = false;
    enableDeveloperCommands = false;

    saveState = false;
    enableAllEditOptions = false;

    saveColorChange = false;    
    saveVolume = false;
    savePreset = false;

    orientation.setChoices({
        "FETs Towards Blade",
        "USB Towards Blade",
        "USB CCW From Blade",
        "USB CW From Blade",
        "Top Towards Blade",
        "Bottom Towards Blade"
    });
    orientation = FETS_TOWARDS_BLADE;

    orientationRotation.x = 0;
    orientationRotation.y = 0;
    orientationRotation.z = 0;

    speakTouchValues = false;

    dynamicBladeDimming = false;
    dynamicBladeLength = false;
    dynamicClashThreshold = false;

    saveBladeDimming = false;
    saveClashThreshold = false;

    filterCutoff = 100;
    filterOrder = 8;

    audioClashSuppressionLevel = 10;
    dontUseGyroForClash = false;

    noRepeatRandom = false;
    femaleTalkie = false;
    disableTalkie = false;
    killOldPlayers = false;
}
