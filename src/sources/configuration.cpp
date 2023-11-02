#include "configuration.h"

#include "defines.h"
#include "mainwindow.h"
#include <cstring>
#include <wx/filedlg.h>

Configuration* Configuration::instance;
Configuration::Configuration() {
    instance = this;
}

void Configuration::outputConfig(const std::string& filePath) {
    PresetsPage::instance->update();
    Configuration::updateBladesConfig();
    BladesPage::instance->update();
    HardwarePage::instance->update();

    std::ofstream configOutput(filePath);

    outputConfigTop(configOutput);
    outputConfigProp(configOutput);
    outputConfigPresets(configOutput);
    outputConfigButtons(configOutput);

    configOutput.close();
}
void Configuration::outputConfig() { Configuration::outputConfig(PROFFIEOS_PATH "/config/ProffieConfig_autogen.h"); }
void Configuration::exportConfig() {
    wxFileDialog configLocation(MainWindow::instance, "Save ProffieOS Config File", "", "ProffieConfig_autogen.h", "C Header Files (*.h)|*.h", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (configLocation.ShowModal() == wxID_CANCEL) return; // User Closed

    Configuration::outputConfig(configLocation.GetPath().ToStdString());
}

void Configuration::outputConfigTop(std::ofstream& configOutput) {
    configOutput << "#ifdef CONFIG_TOP" << std::endl;
    outputConfigTopDefaults(configOutput);
    outputConfigTopGeneral(configOutput);
    outputConfigTopPropSpecific(configOutput);
    configOutput << "#endif" << std::endl << std::endl;

}
void Configuration::outputConfigTopDefaults(std::ofstream& configOutput) {
    if (GeneralPage::instance->settings.massStorage->GetValue()) configOutput << "//PROFFIECONFIG ENABLE_MASS_STORAGE" << std::endl;
    if (GeneralPage::instance->settings.webUSB->GetValue()) configOutput << "//PROFFIECONFIG ENABLE_WEBUSB" << std::endl;
    switch (parseBoardType(GeneralPage::instance->settings.board->GetValue().ToStdString())) {
    case Configuration::ProffieBoard::V1:
        configOutput << "#include \"proffieboard_v1_config.h\"" << std::endl;
        break;
    case Configuration::ProffieBoard::V2:
        configOutput << "#include \"proffieboard_v2_config.h\"" << std::endl;
        break;
    case Configuration::ProffieBoard::V3:
        configOutput << "#include \"proffieboard_v3_config.h\"" << std::endl;
    }
    configOutput << "#define NUM_BLADES " << [=]() -> int32_t {
        int32_t numBlades = 0;
        for (const Configuration::bladeConfig& blade : Configuration::instance->blades) numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
        return numBlades;
    }() << std::endl;
    configOutput << "#define NUM_BUTTONS " << GeneralPage::instance->settings.buttons->num->GetValue() << std::endl;
    configOutput << "#define VOLUME " << GeneralPage::instance->settings.volume->num->GetValue() << std::endl;
    configOutput << "const unsigned int maxLedsPerStrip = " << GeneralPage::instance->settings.maxLEDs->num->GetValue() << ";" << std::endl;
    configOutput << "#define CLASH_THRESHOLD_G " << GeneralPage::instance->settings.clash->num->GetValue() << std::endl;
    // Implement Blade Detect Config
    configOutput << "#define ENABLE_AUDIO" << std::endl;
    configOutput << "#define ENABLE_WS2811" << std::endl;
    configOutput << "#define ENABLE_SD" << std::endl;
    configOutput << "#define ENABLE_MOTION" << std::endl;
    configOutput << "#define SHARED_POWER_PINS" << std::endl;
}
void Configuration::outputConfigTopGeneral(std::ofstream& configOutput) {
    if (HardwarePage::instance->settings.OLED->GetValue()) configOutput << "#define ENABLE_SSD1306" << std::endl;
    if (GeneralPage::instance->settings.colorSave->GetValue()) configOutput << "#define SAVE_COLOR_CHANGE" << std::endl;
    if (GeneralPage::instance->settings.presetSave->GetValue()) configOutput << "#define SAVE_PRESET" << std::endl;
    if (GeneralPage::instance->settings.volumeSave->GetValue()) configOutput << "#define SAVE_VOLUME" << std::endl;
    if (GeneralPage::instance->settings.disableColor->GetValue()) configOutput << "#define DISABLE_COLOR_CHANGE" << std::endl;
    if (GeneralPage::instance->settings.noTalkie->GetValue()) configOutput << "#define DISABLE_TALKIE" << std::endl;
    if (GeneralPage::instance->settings.noBasicParsers->GetValue()) configOutput << "#define DISABLE_BASIC_PARSER_STYLES" << std::endl;
    if (GeneralPage::instance->settings.disableDiagnosticCommands->GetValue()) configOutput << "#define DISABLE_DIAGNOSTIC_COMMANDS" << std::endl;
    if (GeneralPage::instance->settings.enableDeveloperCommands->GetValue()) configOutput << "#define ENABLE_DEVELOPER_COMMANDS" << std::endl;
    configOutput << "#define PLI_OFF_TIME " << GeneralPage::instance->settings.pliTime->num->GetValue() << " * 60 * 1000" << std::endl;
    configOutput << "#define IDLE_OFF_TIME " << GeneralPage::instance->settings.idleTime->num->GetValue() << " * 60 * 1000" << std::endl;
    configOutput << "#define MOTION_TIMEOUT " << GeneralPage::instance->settings.motionTime->num->GetValue() << " * 60 * 1000" << std::endl;
}
void Configuration::outputConfigTopPropSpecific(std::ofstream& configOutput) {
    switch (parsePropSel(PropPage::instance->settings.prop->GetValue().ToStdString())) {
    case Configuration::SaberProp::SA22C:
        outputConfigTopSA22C(configOutput);
        break;
    case Configuration::SaberProp::FETT263:
        outputConfigTopFett263(configOutput);
        break;
    case Configuration::SaberProp::BC:
        outputConfigTopBC(configOutput);
        break;
    case Configuration::SaberProp::SHTOK:
        // No Options
        break;
    case Configuration::SaberProp::CAIWYN:
        outputConfigTopCaiwyn(configOutput);
        break;
    default:
        // No Options
        break;
    }
}
void Configuration::outputConfigTopSA22C(std::ofstream& configOutput) {
    if (PropPage::instance->settings.noLockupHold->GetValue()) configOutput << "#define SA22C_NO_LOCKUP_HOLD" << std::endl;
    if (PropPage::instance->settings.stabOn->GetValue()) configOutput << "#define SA22C_STAB_ON" << std::endl;
    if (PropPage::instance->settings.swingOn->GetValue()) {
        configOutput << "#define SA22C_SWING_ON" << std::endl;
        configOutput << "#define SA22C_SWING_ON_SPEED " << PropPage::instance->settings.swingOnSpeed->num->GetValue() << std::endl;
    }
    if (PropPage::instance->settings.twistOn->GetValue()) configOutput << "#define SA22C_TWIST_ON" << std::endl;
    if (PropPage::instance->settings.thrustOn->GetValue()) configOutput << "#define SA22C_THRUST_ON" << std::endl;
    if (PropPage::instance->settings.twistOff->GetValue()) configOutput << "#define SA22C_TWIST_OFF" << std::endl;
    if (PropPage::instance->settings.forcePush->GetValue()) {
        configOutput << "#define SA22C_FORCE_PUSH" << std::endl;
        configOutput << "#define SA22C_FORCE_PUSH_LENGTH " << PropPage::instance->settings.forcePushLength->num->GetValue() << std::endl;
    }
    if (PropPage::instance->settings.gestureEnBattle->GetValue())
        configOutput << "#define GESTURE_AUTO_BATTLE_MODE" << std::endl;
    configOutput << "#define SA22C_LOCKUP_DELAY " << PropPage::instance->settings.lockupDelay->num->GetValue() << std::endl;
}
void Configuration::outputConfigTopFett263(std::ofstream& configOutput) {
    if (PropPage::instance->settings.stabOn->GetValue()) {
        if (PropPage::instance->settings.stabOnFast->GetValue()) configOutput << "#define FETT263_STAB_ON" << std::endl;
        else if (PropPage::instance->settings.stabOnPreon->GetValue()) configOutput << "#define FETT263_STAB_ON_PREON" << std::endl;
        if (PropPage::instance->settings.stabOnNoBattle->GetValue()) configOutput << "#define FETT263_STAB_ON_NO_BM" << std::endl;
    }
    if (PropPage::instance->settings.swingOn->GetValue()) {
        if (PropPage::instance->settings.swingOnFast->GetValue()) configOutput << "#define FETT263_SWING_ON" << std::endl;
        else if (PropPage::instance->settings.swingOnPreon->GetValue()) configOutput << "#define FETT263_SWING_ON_PREON" << std::endl;
        if (PropPage::instance->settings.swingOnNoBattle->GetValue()) configOutput << "#define FETT263_SWING_ON_NO_BM" << std::endl;
        configOutput << "#define FETT263_SWING_ON_SPEED " << PropPage::instance->settings.swingOnSpeed->num->GetValue() << std::endl;
    }
    if (PropPage::instance->settings.thrustOn->GetValue()) {
        if (PropPage::instance->settings.thrustOnFast->GetValue()) configOutput << "#define FETT263_THRUST_ON" << std::endl;
        else if (PropPage::instance->settings.thrustOnPreon->GetValue()) configOutput << "#define FETT263_THRUST_ON_PREON" << std::endl;
        if (PropPage::instance->settings.thrustOnNoBattle->GetValue()) configOutput << "#define FETT263_THRUST_ON_NO_BM" << std::endl;
    }
    if (PropPage::instance->settings.twistOn->GetValue()) {
        if (PropPage::instance->settings.twistOnFast->GetValue()) configOutput << "#define FETT263_TWIST_ON" << std::endl;
        else if (PropPage::instance->settings.twistOnPreon->GetValue()) configOutput << "#define FETT263_TWIST_ON_PREON" << std::endl;
        if (PropPage::instance->settings.twistOnNoBattle->GetValue()) configOutput << "#define FETT263_TWIST_ON_NO_BM" << std::endl;
    }
    if (PropPage::instance->settings.twistOff->GetValue()) {
        if (PropPage::instance->settings.twistOffFast->GetValue()) configOutput << "#define FETT263_TWIST_OFF_NO_POSTOFF" << std::endl;
        else if (PropPage::instance->settings.twistOffPostoff->GetValue()) configOutput << "#define FETT263_TWIST_OFF" << std::endl;
    }

    if (PropPage::instance->settings.pwrHoldOff->GetValue()) configOutput << "#define FETT263_HOLD_BUTTON_OFF" << std::endl;
    if (PropPage::instance->settings.auxHoldLockup->GetValue()) configOutput << "#define FETT263_HOLD_BUTTON_LOCKUP" << std::endl;
    if (PropPage::instance->settings.meltGestureAlways->GetValue()) configOutput << "#define FETT263_USE_BC_MELT_STAB" << std::endl;
    if (PropPage::instance->settings.volumeCircular->GetValue()) configOutput << "#define FETT263_CIRCULAR_VOLUME_MENU" << std::endl;
    if (PropPage::instance->settings.brightnessCircular->GetValue()) configOutput << "#define FETT263_CIRCULAR_DIM_MENU" << std::endl;
    if (PropPage::instance->settings.pwrWakeGesture->GetValue()) configOutput << "#define FETT263_MOTION_WAKE_POWER_BUTTON" << std::endl;

    if (PropPage::instance->settings.editEnable->GetValue()) {
        configOutput << "#define ENABLE_ALL_EDIT_OPTIONS" << std::endl;
        if (PropPage::instance->settings.editMode->GetValue()) configOutput << "#define FETT263_EDIT_MODE_MENU" << std::endl;
        if (PropPage::instance->settings.editSettings->GetValue()) configOutput << "#define FETT263_EDIT_SETTINGS_MENU" << std::endl;
    }

    if (PropPage::instance->settings.beepErrors->GetValue()) configOutput << "#define DISABLE_TALKIE" << std::endl;
    if (!PropPage::instance->settings.trackPlayerPrompts->GetValue()) configOutput << "#define FETT263_TRACK_PLAYER_NO_PROMPTS" << std::endl;
    if (PropPage::instance->settings.spokenColors->GetValue()) {
        configOutput << "#define FETT263_SAY_COLOR_LIST" << std::endl;
        configOutput << "#define FETT263_SAY_COLOR_LIST_CC" << std::endl;
    }
    if (PropPage::instance->settings.spokenBatteryPercent->GetValue()) configOutput << "#define FETT263_SAY_BATTERY_PERCENT" << std::endl;
    if (PropPage::instance->settings.spokenBatteryVolts->GetValue()) configOutput << "#define FETT263_SAY_BATTERY_VOLTS" << std::endl;

    if (PropPage::instance->settings.forcePush->GetValue()) configOutput << "#define FETT263_FORCE_PUSH_ALWAYS_ON" << std::endl;
    else if (PropPage::instance->settings.forcePushBM->GetValue()) configOutput << "#define FETT263_FORCE_PUSH" << std::endl;
    if (PropPage::instance->settings.forcePush->GetValue() || PropPage::instance->settings.forcePushBM->GetValue()) configOutput << "#define FETT263_FORCE_PUSH_LENGTH " << PropPage::instance->settings.forcePushLength->num->GetValue() << std::endl;


    if (!PropPage::instance->settings.enableQuotePlayer->GetValue()) configOutput << "#define FETT263_DISABLE_QUOTE_PLAYER" << std::endl;
    else {
        if (PropPage::instance->settings.randomizeQuotePlayer->GetValue()) configOutput << "#define FETT263_RANDOMIZE_QUOTE_PLAYER" << std::endl;
        if (PropPage::instance->settings.quotePlayerDefault->GetValue()) configOutput << "#define FETT263_QUOTE_PLAYER_START_ON" << std::endl;
        // if forcePlayerDefault is default already, no define needed
    }

    if (!PropPage::instance->settings.noExtraEffects->GetValue()) {
        if (PropPage::instance->settings.specialAbilities->GetValue()) configOutput << "#define FETT263_SPECIAL_ABILITIES" << std::endl;
        if (PropPage::instance->settings.multiPhase->GetValue()) configOutput << "#define FETT263_MULTI_PHASE" << std::endl;
    }
    if (PropPage::instance->settings.saveChoreo->GetValue()) configOutput << "#define FETT263_SAVE_CHOREOGRAPHY" << std::endl;
    else if (PropPage::instance->settings.spinMode->GetValue()) configOutput << "#define FETT263_SPIN_MODE" << std::endl;
    if (PropPage::instance->settings.saveGesture->GetValue()) configOutput << "#define FETT263_SAVE_GESTURE_OFF" << std::endl;
    if (PropPage::instance->settings.dualModeSound->GetValue()) configOutput << "#define FETT263_DUAL_MODE_SOUND" << std::endl;
    if (PropPage::instance->settings.quickPresetSelect->GetValue()) configOutput << "#define FETT263_QUICK_SELECT_ON_BOOT" << std::endl;
    if (!PropPage::instance->settings.multiBlast->GetValue()) configOutput << "#define FETT263_DISABLE_MULTI_BLAST" << std::endl;
    if (PropPage::instance->settings.multiBlastDisableToggle->GetValue()) configOutput << "#define FETT263_DISABLE_MULTI_BLAST_TOGGLE" << std::endl;

    if (!PropPage::instance->settings.fontChangeOTF->GetValue()) configOutput << "#define FETT263_DISABLE_CHANGE_FONT" << std::endl;
    if (!PropPage::instance->settings.styleChangeOTF->GetValue()) configOutput << "#define FETT263_DISABLE_CHANGE_STYLE" << std::endl;
    if (!PropPage::instance->settings.presetCopyOTF->GetValue()) configOutput << "#define FETT263_DISABLE_COPY_PRESET" << std::endl;
    if (PropPage::instance->settings.clashStrengthSound->GetValue()) {
        configOutput << "#define FETT263_CLASH_STRENGTH_SOUND" << std::endl;
        configOutput << "#define FETT263_MAX_CLASH " << PropPage::instance->settings.clashStrengthSoundMaxClash->num->GetValue() << std::endl;
    }

    // if battleModeToggle is default
    if (PropPage::instance->settings.battleModeAlways->GetValue()) configOutput << "#define FETT263_BATTLE_MODE_ALWAYS_ON" << std::endl;
    if (PropPage::instance->settings.battleModeOnStart->GetValue()) configOutput << "#define FETT263_BATTLE_MODE_START_ON" << std::endl;
    if (PropPage::instance->settings.battleModeNoToggle->GetValue()) configOutput << "#define FETT263_DISABLE_BM_TOGGLE" << std::endl;
    configOutput << "#define FETT263_LOCKUP_DELAY " << PropPage::instance->settings.lockupDelay->num->GetValue() << std::endl;
    configOutput << "#define FETT263_BM_CLASH_DETECT " << PropPage::instance->settings.battleModeClash->num->GetValue() << std::endl;

    if (PropPage::instance->settings.battleModeDisablePWR->GetValue()) configOutput << "#define FETT263_BM_DISABLE_OFF_BUTTON" << std::endl;
}
void Configuration::outputConfigTopBC(std::ofstream& configOutput) {
    if (PropPage::instance->settings.stabOn->GetValue()) configOutput << "#define BC_STAB_ON" << std::endl;
    if (PropPage::instance->settings.swingOn->GetValue()) {
        configOutput << "#define BC_SWING_ON" << std::endl;
        configOutput << "#define BC_SWING_ON_SPEED " << PropPage::instance->settings.swingOnSpeed->num->GetValue() << std::endl;
    }
    if (PropPage::instance->settings.twistOn->GetValue()) configOutput << "#define BC_TWIST_ON" << std::endl;
    if (PropPage::instance->settings.thrustOn->GetValue()) configOutput << "#define BC_THRUST_ON" << std::endl;
    if (PropPage::instance->settings.twistOff->GetValue()) configOutput << "#define BC_TWIST_OFF" << std::endl;
    if (PropPage::instance->settings.forcePush->GetValue()) {
        configOutput << "#define BC_FORCE_PUSH" << std::endl;
        configOutput << "#define BC_FORCE_PUSH_LENGTH " << PropPage::instance->settings.forcePushLength->num->GetValue() << std::endl;
    }
    if (PropPage::instance->settings.gestureEnBattle->GetValue()) configOutput << "#define GESTURE_AUTO_BATTLE_MODE" << std::endl;
    if (PropPage::instance->settings.disableGestureNoBlade->GetValue()) configOutput << "#define NO_BLADE_NO_GEST_ONOFF" << std::endl;
    if (PropPage::instance->settings.multiBlastSwing->GetValue()) configOutput << "#define ENABLE_AUTO_SWING_BLAST" << std::endl;
}
void Configuration::outputConfigTopCaiwyn(std::ofstream& configOutput) {
    if (PropPage::instance->settings.pwrClash->GetValue()) configOutput << "#define CAIWYN_BUTTON_CLASH" << std::endl;
    if (PropPage::instance->settings.pwrLockup->GetValue()) configOutput << "#define CAIWYN_BUTTON_LOCKUP" << std::endl;
}

void Configuration::outputConfigProp(std::ofstream& configOutput)
{
    configOutput << "#ifdef CONFIG_PROP" << std::endl;
    switch (Configuration::parsePropSel(PropPage::instance->settings.prop->GetValue().ToStdString())) {
    case Configuration::SaberProp::SA22C:
        configOutput << "#include \"../props/saber_sa22c_buttons.h\"" << std::endl;
        break;
    case Configuration::SaberProp::FETT263:
        configOutput << "#include \"../props/saber_fett263_buttons.h\"" << std::endl;
        break;
    case Configuration::SaberProp::BC:
        configOutput << "#include \"../props/saber_BC_buttons.h\"" << std::endl;
        break;
    case Configuration::SaberProp::SHTOK:
        configOutput << "#include \"../props/saber_shtok_buttons.h\"" << std::endl;
        break;
    case Configuration::SaberProp::CAIWYN:
        configOutput << "#include \"../props/saber_caiwyn_buttons.h\"" << std::endl;
        break;
    default: break;
    }
    configOutput << "#endif" << std:: endl << std::endl; // CONFIG_PROP
}
void Configuration::outputConfigPresets(std::ofstream& configOutput) {
    configOutput << "#ifdef CONFIG_PRESETS" << std::endl;
    outputConfigPresetsStyles(configOutput);
    configOutput << "BladeConfig blades[] = {" << std::endl;
    configOutput << "\t{ 0," << std::endl;
    outputConfigPresetsBlades(configOutput);
    configOutput << "\t\tCONFIGARRAY(blade_in)" << std::endl << "\t}" << std::endl << "};" << std::endl;
    configOutput << "#endif" << std::endl << std::endl;
}
void Configuration::outputConfigPresetsStyles(std::ofstream& configOutput) {
    configOutput << "Preset blade_in[] = {" << std::endl;
    for (const Configuration::presetConfig& preset : Configuration::instance->presets) {
        configOutput << "\t{ \"" << preset.dirs << "\", \"" << preset.track << "\"," << std::endl;
        if (preset.styles.size() > 0) for (const std::string& style : preset.styles) configOutput << "\t\t" << style << "," << std::endl;
        else configOutput << "\t\t," << std::endl;
        configOutput << "\t\t\"" << preset.name << "\"}";
        // If not the last one, add comma
        if (&Configuration::instance->presets[Configuration::instance->presets.size() - 1] != &preset) configOutput << ",";
        configOutput << std::endl;
    }
    configOutput << "};" << std::endl;
}
void Configuration::outputConfigPresetsBlades(std::ofstream& configOutput) {
    for (const Configuration::bladeConfig& blade : Configuration::instance->blades) {
        if (blade.type == "NeoPixel (RGB)" || blade.type == "NeoPixel (RGBW)") {
            bool firstSub = true;
            if (blade.isSubBlade) for (Configuration::bladeConfig::subBladeInfo subBlade : blade.subBlades) {
                    if (blade.subBladeWithStride) configOutput << "\t\tSubBladeWithStride( ";
                    else /* if not with stride*/ configOutput << "\t\tSubBlade( ";
                    configOutput << subBlade.startPixel << ", " << subBlade.endPixel << ", ";
                    if (firstSub) {
                        genWS281X(configOutput, blade);
                        configOutput << ")," << std::endl;
                    } else {
                        configOutput << "NULL)," << std::endl;
                    }
                    firstSub = false;
                } else {
                configOutput << "\t\t";
                genWS281X(configOutput, blade);
                configOutput << "," << std::endl;
            }
        } else if (blade.type == "Tri-Star Cree" || blade.type == "Quad-Star Cree") {
            bool powerPins[4]{true, true, true, true};
            configOutput << "\t\tSimpleBladePtr<";
            if (blade.Cree1 != "<None>") configOutput << "CreeXPE2" << blade.Cree1 << "Template<" << blade.Cree1Resistance << ">, ";
            else {
                configOutput << "NoLED, ";
                powerPins[0] = false;
            }
            if (blade.Cree2 != "<None>") configOutput << "CreeXPE2" << blade.Cree2 << "Template<" << blade.Cree2Resistance << ">, ";
            else {
                configOutput << "NoLED, ";
                powerPins[1] = false;
            }
            if (blade.Cree3 != "<None>") configOutput << "CreeXPE2" << blade.Cree3 << "Template<" << blade.Cree3Resistance << ">, ";
            else {
                configOutput << "NoLED, ";
                powerPins[2] = false;
            }
            if (blade.Cree4 != "<None>" && blade.type != "Tri-Star Cree") configOutput << "CreeXPE2" << blade.Cree4 << "Template<" << blade.Cree4Resistance << ">, ";
            else {
                configOutput << "NoLED, ";
                powerPins[3] = false;
            }

            Configuration::bladeConfig tempBlade = blade;
            for (int32_t powerPin = 0; powerPin < 4; powerPin++) {
                if (powerPins[powerPin]) {
                    if (tempBlade.usePowerPin1) {
                        configOutput << "bladePowerPin1";
                        tempBlade.usePowerPin1 = false;
                    } else if (tempBlade.usePowerPin2) {
                        configOutput << "bladePowerPin2";
                        tempBlade.usePowerPin2 = false;
                    } else if (tempBlade.usePowerPin3) {
                        configOutput << "bladePowerPin3";
                        tempBlade.usePowerPin3 = false;
                    } else if (tempBlade.usePowerPin4) {
                        configOutput << "bladePowerPin4";
                        tempBlade.usePowerPin4 = false;
                    } else if (tempBlade.usePowerPin5) {
                        configOutput << "bladePowerPin5";
                        tempBlade.usePowerPin5 = false;
                    } else if (tempBlade.usePowerPin6) {
                        configOutput << "bladePowerPin6";
                        tempBlade.usePowerPin6 = false;
                    } else configOutput << "-1";
                } else {
                    configOutput << "-1";
                }

                if (powerPin != 3) configOutput << ", ";
            }
            configOutput << ">()," << std::endl;
        } else if (blade.type == "Single Color") {
            configOutput << "\t\tSimpleBladePtr<CreeXPE2WhiteTemplate<550>, NoLED, NoLED, NoLED, ";
            if (blade.usePowerPin1) {
                configOutput << "bladePowerPin1, ";
            } else if (blade.usePowerPin2) {
                configOutput << "bladePowerPin2, ";
            } else if (blade.usePowerPin3) {
                configOutput << "bladePowerPin3, ";
            } else if (blade.usePowerPin4) {
                configOutput << "bladePowerPin4, ";
            } else if (blade.usePowerPin5) {
                configOutput << "bladePowerPin5, ";
            } else if (blade.usePowerPin6) {
                configOutput << "bladePowerPin6, ";
            } else configOutput << "-1, ";
            configOutput << "-1, -1, -1>()," << std::endl;
        }
    }
}
void Configuration::genWS281X(std::ofstream& configOutput, const Configuration::bladeConfig& blade) {
    std::string bladePin = blade.dataPin;
    std::string bladeColor = blade.type == "NeoPixel (RGB)" || blade.useRGBWithWhite ? blade.colorType : [=](std::string colorType) -> std::string { colorType.replace(colorType.find("W"), 1, "w"); return colorType; }(blade.colorType);

    configOutput << "WS281XBladePtr<" << blade.numPixels << ", " << bladePin << ", Color8::" << bladeColor << ", PowerPINS<";
    if (blade.usePowerPin1) {
        configOutput << "bladePowerPin1";
        if (blade.usePowerPin2 || blade.usePowerPin3 || blade.usePowerPin4 || blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
    }
    if (blade.usePowerPin2) {
        configOutput << "bladePowerPin2";
        if (blade.usePowerPin3 || blade.usePowerPin4 || blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
    }
    if (blade.usePowerPin3) {
        configOutput << "bladePowerPin3";
        if (blade.usePowerPin4 || blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
    }
    if (blade.usePowerPin4) {
        configOutput << "bladePowerPin4";
        if (blade.usePowerPin5 || blade.usePowerPin6) configOutput << ", ";
    }
    if (blade.usePowerPin5) {
        configOutput << "bladePowerPin5";
        if (blade.usePowerPin6) configOutput << ", ";
    }
    if (blade.usePowerPin6) {
        configOutput << "bladePowerPin6";
    }
    configOutput << ">>()";
};
void Configuration::outputConfigButtons(std::ofstream& configOutput) {
    configOutput << "#ifdef CONFIG_BUTTONS" << std::endl;
    configOutput << "Button PowerButton(BUTTON_POWER, powerButtonPin, \"pow\");" << std::endl;
    if (GeneralPage::instance->settings.buttons->num->GetValue() >= 2) configOutput << "Button AuxButton(BUTTON_AUX, auxPin, \"aux\");" << std::endl;
    if (GeneralPage::instance->settings.buttons->num->GetValue() == 3) configOutput << "Button Aux2Button(BUTTON_AUX2, aux2Pin, \"aux\");" << std::endl; // figure out aux2 syntax
    configOutput << "#endif" << std::endl << std::endl; // CONFIG_BUTTONS
}

void Configuration::readConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return;

    std::string section;
    while (!file.eof()) {
        file >> section;
        if (section == "//") {
            getline(file, section);
            continue;
        }
        if (std::strstr(section.data(), "/*")) {
            while (!file.eof()) {
                if (std::strstr(section.data(), "*/")) break;
                file >> section;
            }
            continue;
        }
        if (section == "#ifdef") {
            file >> section;
            if (section == "CONFIG_TOP") Configuration::readConfigTop(file);
            if (section == "CONFIG_PROP") Configuration::readConfigProp(file);
            if (section == "CONFIG_PRESETS") Configuration::readConfigPresets(file);
        }
    }

    //GeneralPage::update();
    PropPage::instance->update();
    BladesPage::instance->update();
    PresetsPage::instance->update();
    Configuration::updateBladesConfig();
}
void Configuration::readConfig() {
    struct stat buffer;
    if (stat(PROFFIEOS_PATH "/config/ProffieConfig_autogen.h", &buffer) != 0) {
        if (wxMessageBox("No existing configuration file was detected. Would you like to import one?", "ProffieConfig", wxYES | wxNO) == wxYES) {
            Configuration::importConfig();
            return;
        } else return;
    }

    Configuration::readConfig(PROFFIEOS_PATH "/config/ProffieConfig_autogen.h");
}
void Configuration::importConfig() {
    wxFileDialog configLocation(MainWindow::instance, "Choose ProffieOS Config File", "", "", "C Header Files (*.h)|*.h", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (configLocation.ShowModal() == wxID_CANCEL) return; // User Closed

    MainWindow::instance->Destroy();
    MainWindow *frame = new MainWindow();
    frame->Show(true);

    Configuration::readConfig(configLocation.GetPath().ToStdString());
}

void Configuration::readConfigTop(std::ifstream& file) {
    std::string element;
    while (!file.eof() && element != "#endif") {
        file >> element;
        if (element == "//") {
            getline(file, element);
            continue;
        }
        if (std::strstr(element.data(), "/*")) {
            while (!file.eof()) {
                if (std::strstr(element.data(), "*/")) break;
                file >> element;
            }
            continue;
        }
        if (element == "#define" && !file.eof()) {
            getline(file, element);
            Configuration::readDefine(element);
        } else if (element == "const" && !file.eof()) {
            getline(file, element);
            std::strtok(element.data(), "="); // unsigned int maxLedsPerStrip =
            element = std::strtok(nullptr, " ;");
            GeneralPage::instance->settings.maxLEDs->num->SetValue(std::stoi(element));
        } else if (element == "#include" && !file.eof()) {
            file >> element;
            if (std::strstr(element.c_str(), "v1") != NULL) {
                GeneralPage::instance->settings.board->SetSelection(0);
            } else if (std::strstr(element.c_str(), "v2") != NULL) {
                GeneralPage::instance->settings.board->SetSelection(1);
            } else if (std::strstr(element.c_str(), "v3") != NULL) {
                GeneralPage::instance->settings.board->SetSelection(2);
            }
        } else if (element == "//PROFFIECONFIG") {
            file >> element;
            if (element == "ENABLE_MASS_STORAGE") GeneralPage::instance->settings.massStorage->SetValue(true);
            if (element == "ENABLE_WEBUSB") GeneralPage::instance->settings.webUSB->SetValue(true);
        }
    }
}
void Configuration::readConfigProp(std::ifstream& file) {
    std::string element;
    while (!file.eof() && element != "#endif") {
        file >> element;
        if (std::strstr(element.data(), "sa22c") != nullptr) PropPage::instance->settings.prop->SetValue("SA22C");
        if (std::strstr(element.data(), "fett263") != nullptr) PropPage::instance->settings.prop->SetValue("Fett263");
        if (std::strstr(element.data(), "shtok") != nullptr) PropPage::instance->settings.prop->SetValue("Shtok");
        if (std::strstr(element.data(), "BC") != nullptr) PropPage::instance->settings.prop->SetValue("BC");
        if (std::strstr(element.data(), "caiwyn") != nullptr) PropPage::instance->settings.prop->SetValue("Caiwyn");
    }
}
void Configuration::readConfigPresets(std::ifstream& file) {
    std::string element;
    while (!file.eof() && element != "#endif") {
        file >> element;
        if (element == "//") {
            getline(file, element);
            continue;
        }
        if (std::strstr(element.data(), "/*")) {
            while (!file.eof()) {
                if (std::strstr(element.data(), "*/")) break;
                file >> element;
            }
            continue;
        }
        if (element == "Preset") readPresetArray(file);
        if (element == "BladeConfig") readBladeArray(file);
    }
}
void Configuration::readDefine(std::string& define) {
# define CHKDEF(str) if (std::strncmp(define.c_str(), str, strlen(str)) == 0)
# define CHKPRP(str) if (std::strstr(define.c_str(), str) != nullptr)
# define DEFVAL std::strtok(nullptr, " ")
# define DEFNUM std::stod(DEFVAL)
    define = std::strtok(&define[0], " ");

    // General Defines
    CHKDEF("NUM_BLADES") {
        uint8_t bladeNum = DEFNUM;
        Configuration::instance->blades.clear();
        while (bladeNum != Configuration::instance->blades.size()) { Configuration::instance->blades.push_back(Configuration::bladeConfig()); }
        PresetsPage::instance->update();
    }
    CHKDEF("NUM_BUTTONS") GeneralPage::instance->settings.buttons->num->SetValue(DEFNUM);
    CHKDEF("VOLUME") GeneralPage::instance->settings.volume->num->SetValue(DEFNUM);
    CHKDEF("CLASH_THRESHOLD_G") GeneralPage::instance->settings.clash->num->SetValue(DEFNUM);
    CHKDEF("SAVE_STATE") {
        GeneralPage::instance->settings.colorSave->SetValue(true);
        GeneralPage::instance->settings.presetSave->SetValue(true);
        GeneralPage::instance->settings.volumeSave->SetValue(true);
    }
    CHKDEF("SAVE_COLOR_CHANGE") GeneralPage::instance->settings.colorSave->SetValue(true);
    CHKDEF("SAVE_PRESET") GeneralPage::instance->settings.presetSave->SetValue(true);
    CHKDEF("SAVE_VOLUME") GeneralPage::instance->settings.volumeSave->SetValue(true);
    CHKDEF("DISABLE_COLOR_CHANGE") GeneralPage::instance->settings.disableColor->SetValue(true);
    CHKDEF("DISABLE_TALKIE") GeneralPage::instance->settings.noTalkie->SetValue(true);
    CHKDEF("DISABLE_BASIC_PARSER_STYLES") GeneralPage::instance->settings.noBasicParsers->SetValue(true);
    CHKDEF("ENABLE_DEVELOPER_COMMANDS") GeneralPage::instance->settings.enableDeveloperCommands->SetValue(true);
    CHKDEF("DISABLE_DIAGNOSTIC_COMMANDS") GeneralPage::instance->settings.disableDiagnosticCommands->SetValue(true);
    CHKDEF("PLI_OFF_TIME") GeneralPage::instance->settings.pliTime->num->SetValue(DEFNUM);
    CHKDEF("IDLE_OFF_TIME") GeneralPage::instance->settings.idleTime->num->SetValue(DEFNUM);
    CHKDEF("MOTION_TIMEOUT") GeneralPage::instance->settings.motionTime->num->SetValue(DEFNUM);

    // Prop Specific
    CHKPRP("STAB_ON") PropPage::instance->settings.stabOn->SetValue(true);
    CHKPRP("STAB_ON_PREON") PropPage::instance->settings.stabOnPreon->SetValue(true);
    CHKPRP("STAB_ON_NO_BM") PropPage::instance->settings.stabOnNoBattle->SetValue(true);
    CHKPRP("SWING_ON") PropPage::instance->settings.swingOn->SetValue(true);
    CHKPRP("SWING_ON_PREON") PropPage::instance->settings.swingOnPreon->SetValue(true);
    CHKPRP("SWING_ON_NO_BM") PropPage::instance->settings.swingOnNoBattle->SetValue(true);
    CHKPRP("SWING_ON_SPEED") PropPage::instance->settings.swingOnSpeed->num->SetValue(DEFNUM);
    CHKPRP("THRUST_ON") PropPage::instance->settings.thrustOn->SetValue(true);
    CHKPRP("THRUST_ON_PREON") PropPage::instance->settings.thrustOnPreon->SetValue(true);
    CHKPRP("THRUST_ON_NO_BM") PropPage::instance->settings.thrustOnNoBattle->SetValue(true);
    CHKPRP("TWIST_ON") PropPage::instance->settings.twistOn->SetValue(true);
    CHKPRP("TWIST_ON_PREON") PropPage::instance->settings.twistOnPreon->SetValue(true);
    CHKPRP("TWIST_ON_NO_BM") PropPage::instance->settings.twistOnNoBattle->SetValue(true);
    CHKPRP("TWIST_OFF") PropPage::instance->settings.twistOff->SetValue(true);
    CHKPRP("TWIST_OFF_NO_POSTOFF") PropPage::instance->settings.twistOffFast->SetValue(true);

    CHKPRP("NO_LOCKUP_HOLD") PropPage::instance->settings.noLockupHold->SetValue(true);
    CHKPRP("ENABLE_AUTO_SWING_BLAST") PropPage::instance->settings.multiBlastSwing->SetValue(true);
    CHKPRP("NO_BLADE_NO_GEST_ONOFF") PropPage::instance->settings.disableGestureNoBlade->SetValue(true);
    CHKPRP("BUTTON_CLASH") PropPage::instance->settings.pwrClash->SetValue(true);
    CHKPRP("BUTTON_LOCKUP") PropPage::instance->settings.pwrLockup->SetValue(true);
    CHKPRP("HOLD_BUTTON_OFF") PropPage::instance->settings.pwrHoldOff->SetValue(true);
    CHKPRP("HOLD_BUTTON_LOCKUP") PropPage::instance->settings.auxHoldLockup->SetValue(true);
    CHKPRP("USE_BC_MELT_STAB") PropPage::instance->settings.meltGestureAlways->SetValue(true);
    CHKPRP("CIRCULAR_VOLUME_MENU") PropPage::instance->settings.volumeCircular->SetValue(true);
    CHKPRP("CIRCULAR_DIM_MENU") PropPage::instance->settings.brightnessCircular->SetValue(true);
    CHKPRP("EDIT") PropPage::instance->settings.editEnable->SetValue(true);
    CHKPRP("EDIT_MODE_MENU") PropPage::instance->settings.editMode->SetValue(true);
    CHKPRP("EDIT_SETTINGS_MENU") PropPage::instance->settings.editSettings->SetValue(true);

    CHKPRP("DISABLE_TALKIE") PropPage::instance->settings.beepErrors->SetValue(true);
    CHKPRP("TRACK_PLAYER_NO_PROMPTS") PropPage::instance->settings.trackPlayerPrompts->SetValue(false);
    CHKPRP("SAY_COLOR_LIST") PropPage::instance->settings.spokenColors->SetValue(true);
    CHKPRP("SAY_BATTERY_VOLTS") PropPage::instance->settings.spokenBatteryVolts->SetValue(true);
    CHKPRP("SAY_BATTERY_PERCENT") PropPage::instance->settings.spokenBatteryPercent->SetValue(true);

    CHKPRP("FETT263_FORCE_PUSH") PropPage::instance->settings.forcePushBM->SetValue(true);
    else CHKPRP("FORCE_PUSH") PropPage::instance->settings.forcePush->SetValue(true);
    CHKPRP("FORCE_PUSH_ALWAYS") PropPage::instance->settings.forcePush->SetValue(true);
    CHKPRP("FORCE_PUSH_LENGTH") PropPage::instance->settings.forcePushLength->num->SetValue(DEFNUM);

    CHKPRP("DISABLE_QUOTE_PLAYER") PropPage::instance->settings.enableQuotePlayer->SetValue(false);
    CHKPRP("RANDOMIZE_QUOTE_PLAYER") PropPage::instance->settings.randomizeQuotePlayer->SetValue(true);
    CHKPRP("QUOTE_PLAYER_START_ON") PropPage::instance->settings.quotePlayerDefault->SetValue(true);

    CHKPRP("SPECIAL_ABILITIES") PropPage::instance->settings.specialAbilities->SetValue(true);
    CHKPRP("MULTI_PHASE") PropPage::instance->settings.multiPhase->SetValue(true);
    CHKPRP("SPIN_MODE") PropPage::instance->settings.spinMode->SetValue(true);
    CHKPRP("SAVE_CHOREOGRAPHY") PropPage::instance->settings.saveChoreo->SetValue(true);
    CHKPRP("SAVE_GESTURE_OFF") PropPage::instance->settings.saveGesture->SetValue(true);
    CHKPRP("DUAL_MODE_SOUND") PropPage::instance->settings.dualModeSound->SetValue(true);
    CHKPRP("QUICK_SELECT_ON_BOOT") PropPage::instance->settings.quickPresetSelect->SetValue(true);
    CHKPRP("DISABLE_MULTI_BLAST_TOGGLE") PropPage::instance->settings.multiBlastDisableToggle->SetValue(true);
    else CHKPRP("DISABLE_MULTI_BLAST") PropPage::instance->settings.multiBlast->SetValue(false);
    CHKPRP("DISABLE_CHANGE_FONT") PropPage::instance->settings.fontChangeOTF->SetValue(false);
    CHKPRP("DISABLE_CHANGE_STYLE") PropPage::instance->settings.styleChangeOTF->SetValue(false);
    CHKPRP("DISABLE_COPY_PRESET") PropPage::instance->settings.presetCopyOTF->SetValue(false);
    CHKPRP("CLASH_STRENGTH_SOUND") PropPage::instance->settings.clashStrengthSound->SetValue(true);
    CHKPRP("MAX_CLASH") PropPage::instance->settings.clashStrengthSoundMaxClash->num->SetValue(DEFNUM);

    CHKPRP("BATTLE_MODE_START_ON") PropPage::instance->settings.battleModeOnStart->SetValue(true);
    CHKPRP("BATTLE_MODE_ALWAYS_ON") PropPage::instance->settings.battleModeAlways->SetValue(true);
    CHKPRP("DISABLE_BM_TOGGLE") PropPage::instance->settings.battleModeNoToggle->SetValue(true);
    CHKPRP("GESTURE_AUTO_BATTLE_MODE") PropPage::instance->settings.gestureEnBattle->SetValue(true);
    CHKPRP("LOCKUP_DELAY") PropPage::instance->settings.lockupDelay->num->SetValue(DEFNUM);
    CHKPRP("BM_CLASH_DETECT") PropPage::instance->settings.battleModeClash->num->SetValue(DEFNUM);
    CHKPRP("BM_DISABLE_OFF_BUTTON") PropPage::instance->settings.battleModeDisablePWR->SetValue(true);

# undef CHKDEF
# undef CHKPRP
# undef DEFVAL
# undef DEFNUM
}
void Configuration::readPresetArray(std::ifstream& file) {
# define CHKSECT if (file.eof() || element == "#endif" || strstr(element.data(), "};") != NULL) return
# define RUNTOSECTION element.clear(); while (element != "{") { file >> element; CHKSECT; }
    // In future get array name?
    char* tempData;
    std::string presetInfo;
    std::string element;
    RUNTOSECTION;
    uint8_t preset = -1;
    Configuration::instance->presets.clear();
    while (!false) {
        presetInfo.clear();
        RUNTOSECTION;
        Configuration::instance->presets.push_back(Configuration::presetConfig());
        preset++;

        while (std::strstr(presetInfo.data(), "}") == nullptr) {
            file >> element;
            CHKSECT;
            presetInfo.append(element);
        }

        tempData = std::strtok(presetInfo.data(), ",\"");
        Configuration::instance->presets[preset].dirs.assign(tempData == nullptr ? "" : tempData);
        tempData = std::strtok(nullptr, ",\"");
        Configuration::instance->presets[preset].track.assign(tempData == nullptr ? "" : tempData);

        tempData = std::strtok(nullptr, ""); // Get rest of data out of strtok
        element = tempData != nullptr ? tempData : "";
        for (uint32_t blade = 0; blade < Configuration::instance->blades.size(); blade++) {
            presetInfo = element; // Assign to presetInfo

            element = presetInfo.substr(presetInfo.find("(),") + 2); // Copy next into element

            presetInfo = presetInfo.substr(presetInfo.find("StylePtr"), presetInfo.find("(),") - 1) + "()"; // Get Style
            Configuration::instance->presets[preset].styles.push_back(presetInfo);
        }
        //std::strtok(nullptr, "\""); // clear bladestyles
        tempData = std::strtok(element.data(), ",\"");
        Configuration::instance->presets[preset].name.assign(tempData == nullptr ? "" : tempData);
    }
# undef CHKSECT
# undef RUNTOSECTION
}
void Configuration::readBladeArray(std::ifstream& file) {
# define CHKSECT if (file.eof() || element == "#endif" || strstr(element.data(), "};") != NULL) return
# define RUNTOSECTION element.clear(); while (element != "{") { file >> element; CHKSECT; }
    // In future get detect val and presetarray association
    std::string element;
    std::string bladeInfo;
    RUNTOSECTION;
    RUNTOSECTION;
    file >> element; // Clear resistance value... maybe use this in the future?
    CHKSECT;
    uint32_t numBlades = Configuration::instance->blades.size();
    Configuration::instance->blades.clear();
    for (uint32_t blade = 0; blade < numBlades; blade++) {
        bladeInfo.clear();
        while (std::strstr(bladeInfo.data(), "),") == nullptr) { // Gather entire blade data
            file >> element;
            CHKSECT;
            bladeInfo.append(element);
        }
        if (std::strstr(bladeInfo.data(), "SubBlade") != nullptr) {
            if (std::strstr(bladeInfo.data(), "NULL") == nullptr) { // Top Level SubBlade
                Configuration::instance->blades.push_back(Configuration::bladeConfig());
                if (std::strstr(bladeInfo.data(), "WithStride")) Configuration::instance->blades[Configuration::instance->blades.size() - 1].subBladeWithStride = true;
            } else { // Lesser SubBlade
                blade--;
                numBlades--;
                // Switch to operating on previous blade
            }

            Configuration::instance->blades[blade].isSubBlade = true;
            std::strtok(bladeInfo.data(), "("); // SubBlade(
            Configuration::instance->blades[blade].subBlades.push_back({ std::stoi(std::strtok(nullptr, "(,")), std::stoi(std::strtok(nullptr, " (,")) });
            bladeInfo = std::strtok(nullptr, ""); // Clear out mangled data from strtok, replace with rest of data ("" runs until end of what's left)
            // Rest will be handled by WS281X "if"
        }
        if (std::strstr(bladeInfo.data(), "WS281XBladePtr") != nullptr) {
            if (Configuration::instance->blades.size() - 1 != blade) Configuration::instance->blades.push_back(Configuration::bladeConfig());
            bladeInfo = std::strstr(bladeInfo.data(), "WS281XBladePtr"); // Shift start to blade data, in case of SubBlade;

            // This must be done first since std::strtok is destructive (adds null chars)
            if (std::strstr(bladeInfo.data(), "bladePowerPin1") != nullptr) Configuration::instance->blades[blade].usePowerPin1 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin2") != nullptr) Configuration::instance->blades[blade].usePowerPin2 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin3") != nullptr) Configuration::instance->blades[blade].usePowerPin3 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin4") != nullptr) Configuration::instance->blades[blade].usePowerPin4 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin5") != nullptr) Configuration::instance->blades[blade].usePowerPin5 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin6") != nullptr) Configuration::instance->blades[blade].usePowerPin6 = true;

            std::strtok(bladeInfo.data(), "<,"); // Clear WS281XBladePtr
            Configuration::instance->blades[blade].numPixels = std::stoi(std::strtok(nullptr, "<,"));
            Configuration::instance->blades[blade].dataPin = std::strtok(nullptr, ",");
            std::strtok(nullptr, ":"); // Clear Color8::
            element = std::strtok(nullptr, ":,"); // Set to color order;
            Configuration::instance->blades[blade].useRGBWithWhite = strstr(element.data(), "W") != nullptr;
            Configuration::instance->blades[blade].colorType.assign(element);

            continue;
        }
        if (std::strstr(bladeInfo.data(), "SimpleBladePtr") != nullptr) {
            Configuration::instance->blades.push_back(Configuration::bladeConfig());
            uint8_t numLEDs = 0;
            auto getCreeTemplate = [](const std::string& element) -> std::string {
                if (std::strstr(element.data(), "RedOrange") != nullptr) return "RedOrange";
                if (std::strstr(element.data(), "Amber") != nullptr) return "Amber";
                if (std::strstr(element.data(), "White") != nullptr) return "White";
                if (std::strstr(element.data(), "Red") != nullptr) return "Red";
                if (std::strstr(element.data(), "Green") != nullptr) return "Green";
                if (std::strstr(element.data(), "Blue") != nullptr) return "Blue";
                // With this implementation, RedOrange must be before Red
                return "<None>";
            };

            // These must be read first since std::strtok is destructive (adds null chars)
            if (std::strstr(bladeInfo.data(), "bladePowerPin1") != nullptr) Configuration::instance->blades[blade].usePowerPin1 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin2") != nullptr) Configuration::instance->blades[blade].usePowerPin2 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin3") != nullptr) Configuration::instance->blades[blade].usePowerPin3 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin4") != nullptr) Configuration::instance->blades[blade].usePowerPin4 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin5") != nullptr) Configuration::instance->blades[blade].usePowerPin5 = true;
            if (std::strstr(bladeInfo.data(), "bladePowerPin6") != nullptr) Configuration::instance->blades[blade].usePowerPin6 = true;

            std::strtok(bladeInfo.data(), "<"); // Clear SimpleBladePtr and setup strtok

            element = std::strtok(nullptr, "<,");
            Configuration::instance->blades[blade].Cree1.assign(getCreeTemplate(element));
            if (Configuration::instance->blades[blade].Cree1 != "<None>") {
                numLEDs++;
                Configuration::instance->blades[blade].Cree1Resistance = std::stoi(std::strtok(nullptr, "<>"));
            }
            element = std::strtok(nullptr, "<,");
            Configuration::instance->blades[blade].Cree2.assign(getCreeTemplate(element));
            if (Configuration::instance->blades[blade].Cree2 != "<None>") {
                numLEDs++;
                Configuration::instance->blades[blade].Cree2Resistance = std::stoi(std::strtok(nullptr, "<>"));
            }
            element = std::strtok(nullptr, "<, ");
            Configuration::instance->blades[blade].Cree3.assign(getCreeTemplate(element));
            if (Configuration::instance->blades[blade].Cree3 != "<None>") {
                numLEDs++;
                Configuration::instance->blades[blade].Cree3Resistance = std::stoi(std::strtok(nullptr, "<>"));
            }
            element = std::strtok(nullptr, "<, ");
            Configuration::instance->blades[blade].Cree4.assign(getCreeTemplate(element));
            if (Configuration::instance->blades[blade].Cree4 != "<None>") {
                numLEDs++;
                Configuration::instance->blades[blade].Cree4Resistance = std::stoi(std::strtok(nullptr, "<>"));
            }

            if (numLEDs <= 2) Configuration::instance->blades[blade].type.assign("Single Color");
            if (numLEDs == 3) Configuration::instance->blades[blade].type.assign("Tri-Star Cree");
            if (numLEDs >= 4) Configuration::instance->blades[blade].type.assign("Quad-Star Cree");
        }
    }
# undef CHKSECT
# undef RUNTOSECTION
}

Configuration::ProffieBoard Configuration::parseBoardType(const std::string& value) {
    return value == "ProffieBoard V1" ? Configuration::ProffieBoard::V1 :
               value == "ProffieBoard V2" ? Configuration::ProffieBoard::V2 :
               Configuration::ProffieBoard::V3;
}
Configuration::SaberProp Configuration::parsePropSel(const std::string& value) {
    return value == PR_SA22C ? Configuration::SaberProp::SA22C :
               value == PR_FETT263 ? Configuration::SaberProp::FETT263 :
               value == PR_BC ? Configuration::SaberProp::BC :
               value == PR_CAIWYN ? Configuration::SaberProp::CAIWYN :
               value == PR_SHTOK ? Configuration::SaberProp::SHTOK :
               Configuration::SaberProp::DEFAULT;
}

void Configuration::updateBladesConfig() {
    if (BladesPage::instance->lastBladeSelection >= 0 && BladesPage::instance->lastBladeSelection < (int32_t)Configuration::instance->blades.size()) { // Save Options
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].type = BladesPage::instance->settings.bladeType->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].usePowerPin1 = BladesPage::instance->settings.usePowerPin1->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].usePowerPin2 = BladesPage::instance->settings.usePowerPin2->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].usePowerPin3 = BladesPage::instance->settings.usePowerPin3->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].usePowerPin4 = BladesPage::instance->settings.usePowerPin4->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].usePowerPin5 = BladesPage::instance->settings.usePowerPin5->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].usePowerPin6 = BladesPage::instance->settings.usePowerPin6->GetValue();

        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].dataPin = BladesPage::instance->settings.bladeDataPin->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].numPixels = BladesPage::instance->settings.bladePixels->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].colorType = Configuration::instance->blades[BladesPage::instance->lastBladeSelection].type == "NeoPixel (RGB)" ? BladesPage::instance->settings.blade3ColorOrder->GetValue() : BladesPage::instance->settings.blade4ColorOrder->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].useRGBWithWhite = BladesPage::instance->settings.blade4UseRGB->GetValue();

        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree1 = BladesPage::instance->settings.star1Color->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree1Resistance = BladesPage::instance->settings.star1Resistance->num->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree2 = BladesPage::instance->settings.star2Color->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree2Resistance = BladesPage::instance->settings.star2Resistance->num->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree3 = BladesPage::instance->settings.star3Color->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree3Resistance = BladesPage::instance->settings.star3Resistance->num->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree4 = BladesPage::instance->settings.star4Color->GetValue();
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].Cree4Resistance = BladesPage::instance->settings.star4Resistance->num->GetValue();

        if (BladesPage::instance->lastSubBladeSelection != -1 && BladesPage::instance->lastSubBladeSelection < (int32_t)Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.size()) {
            Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades[BladesPage::instance->lastSubBladeSelection].startPixel = BladesPage::instance->settings.subBladeStart->GetValue();
            Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades[BladesPage::instance->lastSubBladeSelection].endPixel = BladesPage::instance->settings.subBladeEnd->GetValue();
        }
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBladeWithStride = BladesPage::instance->settings.subBladeUseStride->GetValue();
    }

    // Check if SubBlades need to be removed (changed from Neopixel)
    if (BD_HASSELECTION && BladesPage::instance->lastBladeSelection == BladesPage::instance->settings.bladeSelect->GetSelection() && !BD_ISNEOPIXEL) {
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].isSubBlade = false;
        Configuration::instance->blades[BladesPage::instance->lastBladeSelection].subBlades.clear();
    }
}

