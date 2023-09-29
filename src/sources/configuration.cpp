#include "configuration.h"

#include "defines.h"
#include "generalpage.h"
#include "proppage.h"
#include "presetspage.h"
#include "bladespage.h"
#include "hardwarepage.h"

decltype(Configuration::defaults) Configuration::defaults;
decltype(Configuration::boardConfig) Configuration::boardConfig;
decltype(Configuration::propConfig) Configuration::propConfig;
decltype(Configuration::general) Configuration::general;
decltype(Configuration::options) Configuration::options;
decltype(Configuration::features) Configuration::features;
decltype(Configuration::tweaks) Configuration::tweaks;

std::vector<Configuration::presetConfig> Configuration::presets;
std::vector<Configuration::bladeConfig> Configuration::blades;

void Configuration::outputConfig() {
  Configuration::updateGeneralConfig();
  Configuration::updatePropConfig();
  Configuration::updatePresetsConfig();
  PresetsPage::update();
  Configuration::updateBladesConfig();
  BladesPage::update();
  Configuration::updateHardwareConfig();
  HardwarePage::update();

  std::ofstream configOutput("./resources/ProffieOS/config/ProffieConfig_autogen.h");

  outputConfigTop(configOutput);
  outputConfigProp(configOutput);
  outputConfigPresets(configOutput);
  outputConfigButtons(configOutput);

  configOutput.close();
}

void Configuration::outputConfigTop(std::ofstream& configOutput) {
  configOutput << "#ifdef CONFIG_TOP" << std::endl;
  outputConfigTopDefaults(configOutput);
  outputConfigTopGeneral(configOutput);

  configOutput << "#endif" << std::endl << std::endl; // CONFIG_TOP

}
void Configuration::outputConfigTopDefaults(std::ofstream& configOutput) {
  switch (Configuration::boardConfig.board) {
    case Configuration::PROFFIEBOARD::V1:
      configOutput << "#include \"proffieboard_v1_config.h\"" << std::endl;
      break;
    case Configuration::PROFFIEBOARD::V2:
      configOutput << "#include \"proffieboard_v2_config.h\"" << std::endl;
      break;
    case Configuration::PROFFIEBOARD::V3:
      configOutput << "#include \"proffieboard_v3_config.h\"" << std::endl;
  }
  configOutput << "#define NUM_BLADES " << [=]() -> int32_t {
    int32_t numBlades = 0;
    for (const Configuration::bladeConfig& blade : Configuration::blades) numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
    return numBlades;
  }() << std::endl;
  configOutput << "#define NUM_BUTTONS " << Configuration::general.numButtons << std::endl;
  configOutput << "#define VOLUME " << Configuration::general.volume << std::endl;
  configOutput << "const unsigned int maxLedsPerStrip = " << Configuration::general.maxLEDs << ";" << std::endl;
  configOutput << "#define CLASH_THRESHOLD_G " << Configuration::general.clashThreshold << std::endl;
  // Implement Blade Detect Config
  configOutput << "#define ENABLE_AUDIO" << std::endl;
  configOutput << "#define ENABLE_WS2811" << std::endl;
  configOutput << "#define ENABLE_SD" << std::endl;
  configOutput << "#define ENABLE_MOTION" << std::endl;
  configOutput << "#define SHARED_POWER_PINS" << std::endl;
  configOutput << "#define DISABLE_BASIC_PARSER_STYLES" << std::endl;
}
void Configuration::outputConfigTopGeneral(std::ofstream& configOutput) {
  if (Configuration::features.hasOLED) configOutput << "#define ENABLE_SSD1306" << std::endl;
  if (Configuration::options.saveColorChange) configOutput << "#define SAVE_COLOR_CHANGE" << std::endl;
  if (Configuration::options.savePreset) configOutput << "#define SAVE_PRESET" << std::endl;
  if (Configuration::options.saveVolume) configOutput << "#define SAVE_VOLUME" << std::endl;
  if (Configuration::tweaks.devCommands) configOutput << "#define ENABLE_DEVELOPER_COMMANDS" << std::endl;
  else configOutput << "#define DISABLE_DIAGNOSTIC_COMMANDS" << std::endl;
  configOutput << "#define PLI_OFF_TIME 60 * 1000 * " << Configuration::tweaks.pliTimeout << std::endl;
  configOutput << "#define IDLE_OFF_TIME 60 * 1000 * " << Configuration::tweaks.idleTimout << std::endl;
  configOutput << "#define MOTION_TIMEOUT 60 * 1000 * " << Configuration::tweaks.motionTimeout << std::endl;
}
void Configuration::outputConfigTopPropSpecific(std::ofstream& configOutput) {
  switch (Configuration::propConfig.prop) {
    case Configuration::SABERPROP::SA22C:
      outputConfigTop(configOutput);
      break;
    case Configuration::SABERPROP::FETT263:
      break;
    case Configuration::SABERPROP::BC:
      break;
    case Configuration::SABERPROP::SHTOK:
      break;
    case Configuration::SABERPROP::CAIWYN:
      break;
    default:
      // No Options
      break;
  }
}
void Configuration::outputConfigTopSA22C(std::ofstream& configOutput) {
  if (Configuration::propConfig.noLockupHold) configOutput << "#define SA22C_NO_LOCKUP_HOLD" << std::endl;
  if (Configuration::propConfig.stabOn) configOutput << "#define SA22C_STAB_ON" << std::endl;
  if (Configuration::propConfig.swingOn) configOutput << "#define SA22C_SWING_ON" << std::endl;
  if (Configuration::propConfig.twistOn) configOutput << "#define SA22C_TWIST_ON" << std::endl;
  if (Configuration::propConfig.thrustOn) configOutput << "#define SA22C_THRUST_ON" << std::endl;
  if (Configuration::propConfig.twistOff) configOutput << "#define SA22C_TWIST_OFF" << std::endl;
  if (Configuration::propConfig.forcePush) {
    configOutput << "#define SA22C_FORCE_PUSH" << std::endl;
    configOutput << "#define SA22C_FORCE_PUSH_LENGTH << " << Configuration::propConfig.forcePushLength << std::endl;
  }
  if (Configuration::propConfig.guestureEnBattle) configOutput << "#define GESTURE_AUTO_BATTLE_MODE" << std::endl;
  configOutput << "#define SA22C_LOCKUP_DELAY " << Configuration::propConfig.lockupDelay << std::endl;
}


void Configuration::outputConfigProp(std::ofstream& configOutput)
{
  configOutput << "#ifdef CONFIG_PROP" << std::endl;
  switch (Configuration::propConfig.prop) {
    case Configuration::SABERPROP::SA22C:
      configOutput << "#include \"../props/saber_sa22c_buttons.h\"" << std::endl;
      break;
    case Configuration::SABERPROP::FETT263:
      configOutput << "#include \"../props/saber_fett263_buttons.h\"" << std::endl;
      break;
    case Configuration::SABERPROP::BC:
      configOutput << "#include \"../props/saber_BC_buttons.h\"" << std::endl;
      break;
    case Configuration::SABERPROP::SHTOK:
      configOutput << "#include \"../prop/saber_shtok_buttons.h\"" << std::endl;
      break;
    case Configuration::SABERPROP::CAIWYN:
      configOutput << "#include \"../prop/saber_shtok_buttons\"" << std::endl;
      break;
    default:
      configOutput << "#include \"../prop/saber.h\"" << std::endl;
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
  for (const Configuration::presetConfig& preset : Configuration::presets) {
    configOutput << "\t{ \"" << preset.dirs << "\", \"" << preset.track << "\"," << std::endl;
    if (preset.styles.size() > 0) for (const std::string& style : preset.styles) configOutput << "\t\t" << style << "," << std::endl;
    else configOutput << "\t\t," << std::endl;
    configOutput << "\t\t\"" << preset.name << "\"}";
    // If not the last one, add comma
    if (&Configuration::presets[Configuration::presets.size() - 1] != &preset) configOutput << ",";
    configOutput << std::endl;
  }
  configOutput << "};" << std::endl;
}
void Configuration::outputConfigPresetsBlades(std::ofstream& configOutput) {
  for (const Configuration::bladeConfig& blade : Configuration::blades) {
    if (blade.type == "NeoPixel (RGB)" || blade.type == "NeoPixel (RGBW)") {
      bool firstSub = true;
      if (blade.isSubBlade) for (Configuration::bladeConfig::subBladeInfo subBlade : blade.subBlades) {
          configOutput << "\t\tSubBlade( " << subBlade.startPixel << ", " << subBlade.endPixel << ", ";
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
void Configuration::genWS281X(std::ofstream& configOutput, Configuration::bladeConfig blade) {
  std::string bladePin = blade.dataPin == "Pin 1" ? "bladePin" : blade.dataPin == "Pin 2" ? "blade2Pin" : blade.dataPin == "Pin 3" ? "blade3Pin" : "blade4Pin";
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
  if (Configuration::general.numButtons >= 2) configOutput << "Button AuxButton(BUTTON_AUX, auxPin, \"aux\");" << std::endl;
  if (Configuration::general.numButtons == 3) configOutput << ""; // figure out aux2 syntax
  configOutput << "#endif" << std::endl << std::endl; // CONFIG_BUTTONS
}


void Configuration::updateConfig() {
  updateGeneralConfig();
  updatePresetsConfig();
  updateBladesConfig();
  updateHardwareConfig();
}

void Configuration::updateGeneralConfig() {
  Configuration::boardConfig.board = GeneralPage::settings.board->GetValue() == "ProffieBoard V1" ? Configuration::PROFFIEBOARD::V1 : GeneralPage::settings.board->GetValue() == "ProffieBoard V2" ? Configuration::PROFFIEBOARD::V2 : Configuration::PROFFIEBOARD::V3;
  Configuration::boardConfig.massStorage = GeneralPage::settings.massStorage->GetValue();
  Configuration::boardConfig.webUSB = GeneralPage::settings.webUSB->GetValue();

  Configuration::options.saveVolume = GeneralPage::settings.volumeSave->GetValue();
  Configuration::options.savePreset = GeneralPage::settings.presetSave->GetValue();
  Configuration::options.saveColorChange = GeneralPage::settings.colorSave->GetValue();
  Configuration::options.disableColorChange = GeneralPage::settings.disableColor->GetValue();

  Configuration::tweaks.devCommands = !GeneralPage::settings.disableDev->GetValue();
  Configuration::general.numButtons = GeneralPage::settings.buttons.num->GetValue();
  Configuration::general.volume = GeneralPage::settings.volume.num->GetValue();
  Configuration::general.clashThreshold = GeneralPage::settings.clash.num->GetValue();

  Configuration::tweaks.pliTimeout = GeneralPage::settings.pliTime.num->GetValue();
  Configuration::tweaks.idleTimout = GeneralPage::settings.idleTime.num->GetValue();
  Configuration::tweaks.motionTimeout = GeneralPage::settings.motion.num->GetValue();
}
void Configuration::updatePropConfig() {
  const auto propValue = PropPage::settings.prop->GetValue();
  if (propValue == PR_DEFAULT) {
    Configuration::propConfig.prop = Configuration::SABERPROP::DEFAULT;
  } else if (propValue == PR_SA22C) {
    Configuration::propConfig.prop = Configuration::SABERPROP::SA22C;
  } else if (propValue == PR_FETT263) {
    Configuration::propConfig.prop = Configuration::SABERPROP::FETT263;
  } else if (propValue == PR_SHTOK) {
    Configuration::propConfig.prop = Configuration::SABERPROP::SHTOK;
  } else if (propValue == PR_BC) {
    Configuration::propConfig.prop = Configuration::SABERPROP::BC;
  } else if (propValue == PR_CAIWYN) {
    Configuration::propConfig.prop = Configuration::SABERPROP::CAIWYN;
  }

  Configuration::propConfig.noLockupHold = PropPage::settings.noLockupHold->GetValue();

  Configuration::propConfig.disableguestureNoBlade = PropPage::settings.disableguestureNoBlade->GetValue();
  // Stab On
  Configuration::propConfig.stabOn = PropPage::settings.stabOn->GetValue();
  Configuration::propConfig.stabOnFast = PropPage::settings.stabOnFast->GetValue();
  Configuration::propConfig.stabOnPreon = PropPage::settings.stabOnPreon->GetValue();
  Configuration::propConfig.stabOnNoBattle = PropPage::settings.stabOnNoBattle->GetValue();
  // Swing On
  Configuration::propConfig.swingOn = PropPage::settings.swingOn->GetValue();
  Configuration::propConfig.swingOnSpeed = PropPage::settings.swingOnSpeed.num->GetValue();
  Configuration::propConfig.swingOnFast = PropPage::settings.swingOnFast->GetValue();
  Configuration::propConfig.swingOnPreon = PropPage::settings.swingOnPreon->GetValue();
  Configuration::propConfig.swingOnNoBattle = PropPage::settings.swingOnNoBattle->GetValue();
  // Twist On
  Configuration::propConfig.twistOn = PropPage::settings.twistOn->GetValue();
  Configuration::propConfig.twistOnFast = PropPage::settings.twistOnFast->GetValue();
  Configuration::propConfig.twistOnPreon = PropPage::settings.twistOnPreon->GetValue();
  Configuration::propConfig.twistOnNoBattle = PropPage::settings.twistOnNoBattle->GetValue();
  // Thrust On
  Configuration::propConfig.thrustOn = PropPage::settings.thrustOn->GetValue();
  Configuration::propConfig.thrustOnFast = PropPage::settings.thrustOnFast->GetValue();
  Configuration::propConfig.thrustOnPreon = PropPage::settings.thrustOnPreon->GetValue();
  Configuration::propConfig.thrustOnNoBattle = PropPage::settings.thrustOnNoBattle->GetValue();
  // Twist Off
  Configuration::propConfig.twistOff = PropPage::settings.twistOff->GetValue();
  Configuration::propConfig.twistOffFast = PropPage::settings.twistOffFast->GetValue();
  Configuration::propConfig.twistOffPostoff = PropPage::settings.twistOffPostoff->GetValue();

  // Battle Mode
  Configuration::propConfig.guestureEnBattle = PropPage::settings.guestureEnBattle->GetValue();
  Configuration::propConfig.lockupDelay = PropPage::settings.lockupDelay.num->GetValue();
  Configuration::propConfig.battleModeToggle = PropPage::settings.battleModeToggle->GetValue();
  Configuration::propConfig.battleModeAlways = PropPage::settings.battleModeAlways->GetValue();
  Configuration::propConfig.battleModeOnStart = PropPage::settings.battleModeOnStart->GetValue();
  Configuration::propConfig.battleModeDisablePWR = PropPage::settings.battleModeDisablePWR->GetValue();
  Configuration::propConfig.battleModeClash = PropPage::settings.battleModeClash.num->GetValue();

  // Force Push
  Configuration::propConfig.forcePush = PropPage::settings.forcePush->GetValue();
  Configuration::propConfig.forcePushBM = PropPage::settings.forcePushBM->GetValue();
  Configuration::propConfig.forcePushLength = PropPage::settings.forcePushLength.num->GetValue();

  // Edit Mode/Settings
  Configuration::propConfig.editEnable = PropPage::settings.editEnable->GetValue();
  Configuration::propConfig.editMode = PropPage::settings.editMode->GetValue();
  Configuration::propConfig.editSettings = PropPage::settings.editSettings->GetValue();

  // Quote Player
  Configuration::propConfig.enableQuotePlayer = PropPage::settings.enableQuotePlayer->GetValue();
  Configuration::propConfig.randomizeQuotePlayer = PropPage::settings.randomizeQuotePlayer->GetValue();
  Configuration::propConfig.forcePlayerDefault = PropPage::settings.forcePlayerDefault->GetValue();
  Configuration::propConfig.quotePlayerDefault = PropPage::settings.quotePlayerDefault->GetValue();

  Configuration::propConfig.pwrClash = PropPage::settings.pwrClash->GetValue();
  Configuration::propConfig.pwrLockup = PropPage::settings.pwrLockup->GetValue();
  Configuration::propConfig.pwrHoldOff = PropPage::settings.pwrHoldOff->GetValue();
  Configuration::propConfig.auxHoldLockup = PropPage::settings.auxHoldLockup->GetValue();
  Configuration::propConfig.meltguestureAlways = PropPage::settings.meltguestureAlways->GetValue();
  Configuration::propConfig.volumeCircular = PropPage::settings.volumeCircular->GetValue();
  Configuration::propConfig.brightnessCircular = PropPage::settings.brightnessCircular->GetValue();
  Configuration::propConfig.pwrWakeguesture = PropPage::settings.pwrWakeguesture->GetValue();
  Configuration::propConfig.noExtraEffects = PropPage::settings.noExtraEffects->GetValue();
  Configuration::propConfig.specialAbilities = PropPage::settings.specialAbilities->GetValue();
  Configuration::propConfig.multiPhase = PropPage::settings.multiPhase->GetValue();
  Configuration::propConfig.spinMode = PropPage::settings.spinMode->GetValue();
  Configuration::propConfig.saveGuesture = PropPage::settings.saveGuesture->GetValue();
  Configuration::propConfig.saveChoreo = PropPage::settings.saveChoreo->GetValue();
  Configuration::propConfig.dualModeSound = PropPage::settings.dualModeSound->GetValue();
  Configuration::propConfig.clashStrengthSound = PropPage::settings.clashStrengthSound->GetValue();
  Configuration::propConfig.clashStrengthSoundMaxClash = PropPage::settings.clashStrengthSoundMaxClash.num->GetValue();
  Configuration::propConfig.quickPresetSelect = PropPage::settings.quickPresetSelect->GetValue();
  Configuration::propConfig.spokenColors = PropPage::settings.spokenColors->GetValue();
  Configuration::propConfig.spokenBatteryNone = PropPage::settings.spokenBatteryNone->GetValue();
  Configuration::propConfig.spokenBatteryVolts = PropPage::settings.spokenBatteryVolts->GetValue();
  Configuration::propConfig.spokenBatteryPercent = PropPage::settings.spokenBatteryPercent->GetValue();

  Configuration::propConfig.beepErrors = PropPage::settings.beepErrors->GetValue();
  Configuration::propConfig.trackPlayerPrompts = PropPage::settings.trackPlayerPrompts->GetValue();
  Configuration::propConfig.fontChangeOTF = PropPage::settings.fontChangeOTF->GetValue();
  Configuration::propConfig.styleChangeOTF = PropPage::settings.styleChangeOTF->GetValue();
  Configuration::propConfig.presetCopyOTF = PropPage::settings.presetCopyOTF->GetValue();
  Configuration::propConfig.battleToggle = PropPage::settings.battleToggle->GetValue();
  Configuration::propConfig.multiBlast = PropPage::settings.multiBlast->GetValue();
  Configuration::propConfig.multiBlastDisableToggle = PropPage::settings.multiBlastDisableToggle->GetValue();
  Configuration::propConfig.multiBlastSwing = PropPage::settings.multiBlastSwing->GetValue();
}
void Configuration::updatePresetsConfig() {
}
void Configuration::updateBladesConfig() {
  if (BladesPage::lastBladeSelection >= 0 && BladesPage::lastBladeSelection < (int32_t)Configuration::blades.size()) { // Save Options
    Configuration::blades[BladesPage::lastBladeSelection].type = BladesPage::settings.bladeType->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].usePowerPin1 = BladesPage::settings.usePowerPin1->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].usePowerPin2 = BladesPage::settings.usePowerPin2->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].usePowerPin3 = BladesPage::settings.usePowerPin3->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].usePowerPin4 = BladesPage::settings.usePowerPin4->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].usePowerPin5 = BladesPage::settings.usePowerPin5->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].usePowerPin6 = BladesPage::settings.usePowerPin6->GetValue();

    Configuration::blades[BladesPage::lastBladeSelection].dataPin = BladesPage::settings.bladeDataPin->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].numPixels = BladesPage::settings.bladePixels->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].colorType = Configuration::blades[BladesPage::lastBladeSelection].type == "NeoPixel (RGB)" ? BladesPage::settings.blade3ColorOrder->GetValue() : BladesPage::settings.blade4ColorOrder->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].useRGBWithWhite = BladesPage::settings.blade4UseRGB->GetValue();

    Configuration::blades[BladesPage::lastBladeSelection].Cree1 = BladesPage::settings.star1Color->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].Cree2 = BladesPage::settings.star2Color->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].Cree3 = BladesPage::settings.star3Color->GetValue();
    Configuration::blades[BladesPage::lastBladeSelection].Cree4 = BladesPage::settings.star4Color->GetValue();

    if (BladesPage::lastSubBladeSelection != -1 && BladesPage::lastSubBladeSelection < (int32_t)Configuration::blades[BladesPage::lastBladeSelection].subBlades.size()) {
      Configuration::blades[BladesPage::lastBladeSelection].subBlades[BladesPage::lastSubBladeSelection].startPixel = BladesPage::settings.subBladeStart->GetValue();
      Configuration::blades[BladesPage::lastBladeSelection].subBlades[BladesPage::lastSubBladeSelection].endPixel = BladesPage::settings.subBladeEnd->GetValue();
    }
    Configuration::blades[BladesPage::lastBladeSelection].subBladeWithStride = BladesPage::settings.subBladeUseStride->GetValue();
  }

  // Check if SubBlades need to be removed (changed from Neopixel)
  if (BD_HASSELECTION && BladesPage::lastBladeSelection == BladesPage::settings.bladeSelect->GetSelection() && !BD_ISNEOPIXEL) {
    Configuration::blades[BladesPage::lastBladeSelection].isSubBlade = false;
    Configuration::blades[BladesPage::lastBladeSelection].subBlades.clear();
  }
}
void Configuration::updateHardwareConfig() {
  Configuration::features.bladeDetect = HardwarePage::settings.bladeDetect->GetValue();
  Configuration::features.bladeDetectPin = HardwarePage::settings.bladeDetectPin->GetValue();
}
