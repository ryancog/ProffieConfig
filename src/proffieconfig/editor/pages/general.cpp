#include "general.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/general.cpp
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

#include "config/strings.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/checkbox.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/divider.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

GeneralPage::GeneralPage(config::Config& config) : mConfig{config} {}

void GeneralPage::deinit() {
    if (mButtonDlg) {
        pcui::cripple(mButtonDlg);
        mButtonDlg->Destroy();
    }
}

pcui::DescriptorPtr GeneralPage::ui() {
    return pcui::Stack{
      .base_={.border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL}},
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .orient_=wxVERTICAL,
          .children_={
            setup(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                installation(),
                pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                audio(),
              }
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            tweaks(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxVERTICAL,
          .children_={
            editing(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            misc(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr GeneralPage::setup() {
    return pcui::Group{
      .win_={.base_={.expand_=true}},
      .label_=_("Setup"),
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .base_={.proportion_=1},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Choice{
              .win_={
                .base_={.expand_=true},
                .tooltip_=_("The hardware revision of the physical proffieboard."),
              },
              .data_=mConfig.boardSel(),
              .style_=pcui::Choice::PopUp{
                .unselected_=_("Select Board"),
              },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Choice{
              .win_={
                .base_={.expand_=true},
              },
              .data_=mConfig.osVersion_,
              .style_=pcui::Choice::PopUp{
                .unselected_=_("Select ProffieOS"),
              },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::CheckBox{
          .win_={
            .base_={.align_=wxALIGN_CENTER},
            .tooltip_=_("Enable to access the contents of your proffieboard's SD card via the USB connection."),
          },
          .label_=_("Enable Mass Storage"),
          .data_=mConfig.settings_.massStorage_,
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::CheckBox{
          .win_={
            .base_={.align_=wxALIGN_CENTER},
            .tooltip_=_("Enable to access the ProffieOS Workbench via USB.\nSee the POD Page \"The ProffieOS Workbench\" for more info."),
          },
          .label_=_("Enable WebUSB"),
          .data_=mConfig.settings_.webUsb_,
        }(),
      }
    }();
}

pcui::DescriptorPtr GeneralPage::misc() {
    return pcui::Group{
      .win_={.base_={.expand_=true, .proportion_=1}},
      .label_=_("Misc"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("Time (in minutes) since last activity before PLI goes to sleep."),
          },
          .label_=_("PLI Timeout (seconds)"),
          .ctrl_=pcui::Stepper{
            .win_={.base_={.expand_=true}},
            .data_=mConfig.settings_.pliOffTime_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("Time (in minutes) since last activity before accent LEDs go to sleep."),
          },
          .label_=_("Idle Timeout (minutes)"),
          .ctrl_=pcui::Stepper{
            .win_={.base_={.expand_=true}},
            .data_=mConfig.settings_.idleOffTime_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("Time (in minutes) since last activity before gesture controls are disabled."),
          },
          .label_=_("Motion Timeout (minutes)"),
          .ctrl_=pcui::Stepper{
            .win_={.base_={.expand_=true}},
            .data_=mConfig.settings_.motionTimeout_,
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr GeneralPage::installation() {
    return pcui::Group{
      .win_={.base_={.expand_=true}},
      .label_=_("Installation"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("Impact required to trigger a clash effect.\nMeasured in Gs."),
          },
          .label_=_("Clash Threshold (Gs)"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.proportion_=1}},
            .data_=mConfig.settings_.clashThreshold_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("The orientation of the Proffieboard in the saber."),
          },
          .label_=_("Orientation"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Choice{
            .win_={.base_={.proportion_=1}},
            .data_=mConfig.settings_.orientation_,
            .labeler_=[](uint32 idx) -> wxString {
                switch (static_cast<config::Orientation>(idx)) {
                    using enum config::Orientation;
                    case eOrient_Fets_Towards_Blade:
                        return _("FETs Towards Blade");
                    case eOrient_USB_Towards_Blade:
                        return _("USB Towards Blade");
                    case eOrient_USB_CCW_From_Blade:
                        return _("USB CCW From Blade");
                    case eOrient_USB_CW_From_Blade:
                        return _("USB CW From Blade");
                    case eOrient_Top_Towards_Blade:
                        return _("Top Towards Blade");
                    case eOrient_Bottom_Towards_Blade:
                        return _("Bottom Towards Blade");
                    case eOrient_Max:
                        break;
                }
                return {};
            },
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Labeled{
              .label_=_("X"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.proportion_=1}},
                .data_=mConfig.settings_.orientationRotation_.x_,
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .label_=_("Y"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.proportion_=1}},
                .data_=mConfig.settings_.orientationRotation_.y_,
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .label_=_("Z"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.proportion_=1}},
                .data_=mConfig.settings_.orientationRotation_.z_,
              }(),
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .win_={
            .tooltip_=_("Enable if you have an OLED/SSD1306 display connected."),
          },
          .label_=_("Enable OLED"),
          .data_=mConfig.settings_.enableOled_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .tooltip_={
              _("Configure physical buttons on the saber.") + '\n' +
              _("Not all prop files support all possible numbers of buttons, and controls may change depending on how many buttons are specified.")
            },
          },
          .label_=_("Buttons..."),
          .func_=[this](pcui::CallbackContext ctxt) {
              if (not mButtonDlg) {
                  mButtonDlg = new ButtonsDlg(ctxt.topLevel_, mConfig);
              }

              mButtonDlg->Show();
              mButtonDlg->Raise();
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr GeneralPage::tweaks() {
    return pcui::Group{
      .win_={.base_={.expand_=true}},
      .label_=_("Tweaks"),
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .orient_=wxVERTICAL,
          .children_={
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Do not play the same audio file twice consecutively when randomly selecting."),
              },
              .label_=_("No Repeat Random"),
              .data_=mConfig.settings_.noRepeatRandom_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Stop playing old sounds to ensure new sounds always play"),
              },
              .label_=_("Kill Old Players"),
              .data_=mConfig.settings_.killOldPlayers_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Use beeps instead of spoken messages for errors, which saves some memory.\nSee the POD page \"What is it beeping?\"."),
              },
              .label_=_("Disable Talkie"),
              .data_=mConfig.settings_.disableTalkie_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Use Female Talkie Voice"),
              },
              .label_=_("Female Talkie"),
              .data_=mConfig.settings_.femaleTalkie_,
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .orient_=wxVERTICAL,
          .children_={
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Disable color change controls"),
              },
              .label_=_("Disable Color Change"),
              .data_=mConfig.settings_.disableColorChange_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Disable basic styles in the ProffieOS Workbench to save memory."),
              },
              .label_=_("Disable Basic Parser Styles"),
              .data_=mConfig.settings_.disableBasicParserStyles_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::CheckBox{
              .win_={
                .tooltip_=_("Disable diagnostic commands in the Serial Monitor to save memory."),
              },
              .label_=_("Disable Diagnostic Commands"),
              .data_=mConfig.settings_.disableDiagnosticCommands_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={.base_={.expand_=true}},
              .label_=_("Custom Options..."),
            }(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr GeneralPage::editing() {
    return pcui::Group{
      .win_={.base_={.expand_=true}},
      .label_=_("Editing"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::CheckBox{
          .label_=_("Save State"),
          .data_=mConfig.settings_.saveState_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Enable All Edit Options"),
          .data_=mConfig.settings_.enableAllEditOptions_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .win_={
            .tooltip_=_("Save the volume level between board restarts."),
          },
          .label_=_("Save Volume"),
          .data_=mConfig.settings_.saveVolume_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .win_={
            .tooltip_=_("Save the currently-selected preset between board restarts."),
          },
          .label_=_("Save Preset"),
          .data_=mConfig.settings_.savePreset_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .win_={
            .tooltip_=_("Save color edits to presets."),
          },
          .label_=_("Save Color"),
          .data_=mConfig.settings_.saveColorChange_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Save Blade Dimming"),
          .data_=mConfig.settings_.saveBladeDimming_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Save Clash Threshold"),
          .data_=mConfig.settings_.saveClashThreshold_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Dynamic Blade Dimming"),
          .data_=mConfig.settings_.dynamicBladeDimming_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Dynamic Clash Threshold"),
          .data_=mConfig.settings_.dynamicClashThreshold_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Dynamic Blade Length"),
          .data_=mConfig.settings_.dynamicBladeLength_,
        }(),
      }
    }();
}

pcui::DescriptorPtr GeneralPage::audio() {
    return pcui::Group{
      .label_=_("Audio"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("Maximum volume level.\nDo not increase unless you know what you are doing, as this can damage your speaker."),
          },
          .label_=_("Max Volume"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.proportion_=1}},
            .data_=mConfig.settings_.volume_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::CheckBox{
              .win_={
                .base_={.align_=wxALIGN_CENTER},
                .tooltip_=_("Saber volume when saber turns on. Volume can be changed afterwards."),
              },
              .label_=_("Boot Volume"),
              .data_=mConfig.settings_.bootVolume_.enable_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Stepper{
              .win_={.base_={.proportion_=1}},
              .data_=mConfig.settings_.bootVolume_.value_,
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Enable Filtering"),
          .data_=mConfig.settings_.filter_.enable_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={.base_={.expand_=true}},
          .label_=_("Cutoff"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.proportion_=1}},
            .data_=mConfig.settings_.filter_.cutoff_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={.base_={.expand_=true}},
          .label_=_("Order"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.proportion_=1}},
            .data_=mConfig.settings_.filter_.order_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={.base_={.expand_=true}},
          .label_=_("Clash Suppression"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.proportion_=1}},
            .data_=mConfig.settings_.audioClashSuppressionLevel_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckBox{
          .label_=_("Don't Use Gyro For Clash"),
          .data_=mConfig.settings_.dontUseGyroForClash_,
        }(),
      }
    }();
}

