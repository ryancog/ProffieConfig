#include "awareness.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/awareness.cpp
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
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/checkbox.hpp"
#include "ui/controls/checklist.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/combobox.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/symbols.hpp"
#include "ui/values.hpp"

BladeAwarenessDlg::BladeAwarenessDlg(
    wxWindow* parent, config::Config& config
) : pcui::Dialog(
        parent,
        wxID_ANY,
        _("Blade Awareness")
    ),
    mConfig(config) {

    pcui::build(this, ui());
}

pcui::DescriptorPtr BladeAwarenessDlg::ui() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            detect(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            idSetup(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            idPower(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            idContinuous(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr BladeAwarenessDlg::idSetup() {
    auto& bladeId{mConfig.settings_.bladeAwareness_.bladeId_};

    return pcui::Group{
      .win_={
        .base_={.expand_=true, .proportion_=1},
        .tooltip_=_("Detect when a specific blade is inserted based on a resistor placed in the blade to give it an identifier."),
      },
      .label_=_("Blade ID Setup"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::CheckBox{
          .label_=_("Enable"),
          .data_=bladeId.enable_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .enable_=bladeId.mode_ | data::logic::IsEnabled{},
            .tooltip_=_("The mode to be used for Blade ID.\nSee the POD page \"Blade ID\" for more info."),
          },
          .label_=_("Mode"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Choice{
            .win_={.base_={.proportion_=1}},
            .data_=bladeId.mode_,
            .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                switch (static_cast<config::BladeIDMode>(idx)) {
                    using enum config::BladeIDMode;
                    case eBIDMode_Snapshot: return _("Snapshot");
                    case eBIDMode_External: return _("External Pullup");
                    case eBIDMode_Bridged: return _("Bridged Pullup");
                    case eBIDMode_Max:
                        break;
                }
                return {};
            }
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .enable_=bladeId.pin_ | data::logic::IsEnabled{},
            .tooltip_=_("The pin used to detect blade resistance values.\nCannot be the same as Detect Pin."),
          },
          .label_=_("Pin"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::ComboBox{
            .win_={.base_={.proportion_=1}},
            .data_=bladeId.pin_,
            .defaults_={
              "bladeIdentifyPin",
              "bladePin",
              "blade2Pin",
              "blade3Pin",
              "blade4Pin",
            }
          }(),
        }(),
        pcui::Labeled{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
            },
            .show_=bladeId.mode_ | data::logic::HasSelection{{
              config::eBIDMode_External
            }},
            .enable_=bladeId.pullup_ | data::logic::IsEnabled{},
            .tooltip_=_("The value of the pullup resistor placed on the Blade ID line."),
          },
          .label_=_("Pull-Up Resistance"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.proportion_=1}},
            .data_=bladeId.pullup_,
          }(),
        }(),
        pcui::Labeled{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
            },
            .show_=bladeId.mode_ | data::logic::HasSelection{{
              config::eBIDMode_Bridged
            }},
            .enable_=bladeId.bridgePin_ | data::logic::IsEnabled{},
            .tooltip_=_("The pin number or name of the pin which ID Pin is bridged to for pullup.\n This pin cannot be used for anything else."),
          },
          .label_=_("Bridge Pin"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::ComboBox{
            .win_={.base_={.proportion_=1}},
            .data_=bladeId.bridgePin_,
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr BladeAwarenessDlg::idPower() {
    auto& bladeId{mConfig.settings_.bladeAwareness_.bladeId_};

    const auto onAddPowerPin{[this, &bladeId] {
        auto entry{data::context(mPowerPinAddField)};

        // Could be empty coming from add field enter action.
        if (entry.val().empty()) return;

        auto powerPins{data::context(bladeId.powerPins_)};

        powerPins.select(std::string{entry.val()});
        entry.clear();
    }};

    return pcui::Group{
      .win_={
        .base_={.expand_=true, .proportion_=1},
        .tooltip_=_("Power up during Blade ID.\nThis is required for WS281X blades."),
      },
      .label_=_("Power for Blade ID"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::CheckBox{
          .label_=_("Enable"),
          .data_=bladeId.powerForId_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::CheckList{
          .win_={.base_={.expand_=true}},
          .data_=bladeId.powerPins_,
        }(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Text{
              .win_={
                .base_={.proportion_=1},
                .enable_=bladeId.powerForId_ | data::logic::IsSet{},
              },
              .data_=mPowerPinAddField,
              .style_=pcui::Text::SingleLine{
                .hint_=_("Pin Name"),
                .onEnter_=onAddPowerPin,
              },
            }(),
            pcui::Button{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize()},
                .enable_={
                    bladeId.powerForId_ | data::logic::IsSet{} and
                    not (mPowerPinAddField | data::logic::IsEmpty{})
                },
              },
              .label_=pcui::syms::PLUS,
              .style_=pcui::Button::Style::Companion,
              .exactFit_=true,
              .func_=onAddPowerPin,
            }(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr BladeAwarenessDlg::idContinuous() {
    auto& bladeId{mConfig.settings_.bladeAwareness_.bladeId_};

    return pcui::Group{
      .win_={
        .base_={.expand_=true, .proportion_=1},
        .tooltip_=_("Continuously monitor the Blade ID to detect changes."),
      },
      .label_=_("Continuous Scanning"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::CheckBox{
          .label_=_("Enable"),
          .data_=bladeId.continuous_.enable_,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .enable_=bladeId.continuous_.times_ | data::logic::IsEnabled{},
            .tooltip_=_("Number of times to read the Blade ID to average out the result and compensate for inaccurate readings."),
          },
          .label_=_("Number of Reads to Average"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.expand_=true}},
            .data_=bladeId.continuous_.times_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .enable_=bladeId.continuous_.interval_ | data::logic::IsEnabled{},
            .tooltip_=_("Scan the Blade ID and update accordingly every input number of millis."),
          },
          .label_=_("Scan Interval (ms)"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.expand_=true}},
            .data_=bladeId.continuous_.interval_,
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr BladeAwarenessDlg::detect() {
    auto& bladeDetect{mConfig.settings_.bladeAwareness_.bladeDetect_};

    return pcui::Group{
      .win_={
        .base_={.expand_=true, .proportion_=1},
        .tooltip_=_("Detect when a blade is inserted into the saber or not."),
      },
      .label_=_("Blade Detect"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::CheckBox{
          .label_=_("Enable"),
          .data_=bladeDetect.enable_
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .enable_=bladeDetect.pin_ | data::logic::IsEnabled{},
            .tooltip_=_("The pin which will be bridged to BATT- when blade is inserted.\nCannot be the same as ID Pin.")
          },
          .label_=_("Pin"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::ComboBox{
            .win_={.base_={.proportion_=1}},
            .data_=bladeDetect.pin_,
            .defaults_={
              "bladePin",
              "blade2Pin",
              "blade3Pin",
              "blade4Pin",
            }
          }(),
        }(),
      }
    }();
}

