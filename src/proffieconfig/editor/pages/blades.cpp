#include "blades.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/blades.cpp
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

#include <wx/settings.h>
#include <wx/gdicmn.h>

#include "config/blades/bladeconfig.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/builders/selector.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/checkbox.hpp"
#include "ui/controls/checklist.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/combobox.hpp"
#include "ui/controls/radios.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/panel.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/string.hpp"

#include "../special/splitvisualizer.hpp"

BladesPage::BladesPage(config::Config& config) : mConfig{config} {
    static const auto arrayTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&BladesPage::onArrayChoice>();
        return table;
    }()};
    observeWith(mArraySel.choice(), arrayTable);

    static const auto bladeTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&BladesPage::onBladeChoice>();
        return table;
    }()};
    observeWith(mBladeSel.choice(), bladeTable);

    static const auto subTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&BladesPage::onSubChoice>();
        return table;
    }()};
    observeWith(mSubBladeSel.choice(), subTable);

    const auto arrayFilter{[](
        const data::base::Choice::ROContext& ctxt, int32& idx
    ) {
        if (idx == -1 and ctxt.num())
            idx = 0;
    }};
    mArraySel.choice().setFilter(arrayFilter);

    const auto powerPinFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};
    mPowerPinAddField.setFilter(powerPinFilter);

    activate();
}

void BladesPage::deinit() {
    if (mAwarenessDlg) {
        pcui::cripple(mAwarenessDlg);
        mAwarenessDlg->Destroy();
    }

    deactivate();
}

void BladesPage::onActivate() {
    mArraySel.bind(&mConfig.bladeConfigs_);
}

void BladesPage::preDeactivate() {
    // Do this pre so that the handling to unbind others is still active.
    mArraySel.bind(nullptr);
}

pcui::DescriptorPtr BladesPage::ui() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxHORIZONTAL,
      .children_={
        selection(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        blades(),
      }
    }();
}

pcui::DescriptorPtr BladesPage::selection() {
    return pcui::Stack{
      .base_={
        .minSize_={200, -1},
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Button{
          .win_={.base_={.expand_=true}},
          .label_=_("Blade Awareness..."),
          .func_=[this](const pcui::CallbackContext& ctxt) {
              onAwarenessButton(ctxt);
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Group{
          .win_={.base_={.expand_=true}},
          .label_=_("Blade Array"),
          .orient_=wxVERTICAL,
          .children_={
            pcui::Stack{
              .base_={.expand_=true},
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Label{
                  .win_={
                    .base_={
                      .border_={
                        .size_=pcui::interControlSpacing(),
                        .dirs_=wxRIGHT
                      },
                      .align_=wxALIGN_CENTER,
                    },
                    .show_=not (mIssueLabel | data::logic::IsEmpty{}),
                    .tooltip_=_("There's issues with this array, open the edit dialog to see them."),
                  },
                  .label_=mIssueLabel,
                }(),
                pcui::Choice{
                  .win_={
                    .base_={.proportion_=1},
                    .tooltip_=_("The currently-selected Blade Array to edit."),
                  },
                  .data_=mArraySel,
                  .style_=pcui::Choice::PopUp{
                    .unselected_=_("Select Array"),
                  },
                  .emptyLabel_=_("[default]"),
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      auto vec{data::context(mConfig.bladeConfigs_)};
                      auto& cfg{dynamic_cast<config::blades::BladeConfig&>(
                          *vec.children()[idx]
                      )};
                      return cfg.name_;
                  },
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={
                    .enable_=mArraySel.choice() | data::logic::HasSelection{},
                  },
                  .bitmap_={
                    .src_=pcui::Bitmap{"edit"}.color(wxSYS_COLOUR_WINDOWTEXT),
                  },
                  .exactFit_=true,
                  .func_=[this](const pcui::CallbackContext& ctxt) {
                      onEditButton(ctxt);
                  }
                }(),
              }
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Stack{
              .base_={.expand_=true},
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Button{
                  .win_={.base_={.proportion_=1}},
                  .label_=_("Add"),
                  .exactFit_=true,
                  .func_=[this](const pcui::CallbackContext& ctxt) {
                      onAddButton(ctxt);
                  }
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={
                    .base_={.proportion_=1},
                    .enable_=mArraySel.choice() | data::logic::HasSelection{},
                  },
                  .label_=_("Remove"),
                  .exactFit_=true,
                  .func_=[this] { onRemoveButton(); },
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Group{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .label_=_("Blades"),
          .orient_=wxVERTICAL,
          .children_={
            pcui::Choice{
              .win_={.base_={.expand_=true, .proportion_=1}},
              .data_=mBladeSel,
              .style_=pcui::Choice::List{},
              .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                  return wxString::Format(_("Blade %d"), idx);
              }
            }(),
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize()},
                    .enable_=mArraySel.choice() | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::PLUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onAddBladeButton(); },
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize()},
                    .enable_=mBladeSel.choice() | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::MINUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onRemoveBladeButton(); },
                }(),
              }
            }(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr BladesPage::blades() {
    const auto typeBuilder{[this](
        data::base::Model *model
    ) -> pcui::DescriptorPtr {
        using namespace config::blades;

        if (auto *ptr{dynamic_cast<WS281X *>(model)})
            return ws281x(*ptr);

        if (auto *ptr{dynamic_cast<Simple *>(model)})
            return simple(*ptr);

        if (auto *ptr{dynamic_cast<Servo *>(model)})
            return servo(*ptr);

        return pcui::Stack{
          .base_={.expand_=true, .proportion_=1},
          .children_={
            pcui::StretchSpacer{}(),
            pcui::Label{
              .win_={.base_={.align_=wxALIGN_CENTER}},
              .label_=_("\"Dummy\" Blade"),
              .font_=pcui::Font::Header,
              .color_=wxSYS_COLOUR_GRAYTEXT,
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Label{
              .win_={.base_={.align_=wxALIGN_CENTER}},
              .label_=_("This is useful to, e.g., act as a \"spacer\" to line up bladestyles across multiple arrays which use the same preset array."),
              .color_=wxSYS_COLOUR_GRAYTEXT,
              .wrapWidth_=300,
            }(),
            pcui::StretchSpacer{}(),
          }
        }();
    }};

    const auto builder{[this, typeBuilder](
        data::base::Model *model
    ) -> pcui::DescriptorPtr {
        if (model == nullptr) {
            return pcui::Label{
              .win_={
                .base_={
                  .minSize_={350, -1},
                  .proportion_=2,
                  .align_=wxALIGN_CENTER,
                },
              },
              .label_=_("No Blade Selected"),
              .font_=pcui::Font::Header,
              .color_=wxSYS_COLOUR_GRAYTEXT,
            }();
        }

        auto& blade{*dynamic_cast<config::blades::Blade *>(model)};
        return pcui::Stack{
            .base_={.expand_=true, .proportion_=2},
            .orient_=wxVERTICAL,
            .children_={
              pcui::Choice{
                .data_=blade.type(),
                .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                    using enum config::blades::Blade::Type;
                    if (idx == eWS281X) return "WS281X";
                    if (idx == eSimple) return _("Simple");
                    if (idx == eUnassigned) return _("Unassigned");
                    if (idx == eServo) return _("Servo");
                    return {};
                }
              }(),
              pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
              pcui::builders::Selector{
                .data_=blade.type(),
                .builder_=typeBuilder,
              }(),
            }
        }();
    }};

    return pcui::builders::Selector{
      .data_=mBladeSel,
      .builder_=builder,
    }();
}

pcui::DescriptorPtr BladesPage::simple(config::blades::Simple& simple) {
    auto& blade{simple.parent_};

    const auto led{[](
        config::blades::Simple::LED& led,
        uint32 idx
    ) -> pcui::DescriptorPtr {
        return pcui::Group{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .label_=wxString::Format(_("LED %d"), idx),
          .children_={
            pcui::Choice{
              .win_={.base_={.expand_=true}},
              .data_=led.profile_,
              .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                  switch (static_cast<config::LED>(idx)) {
                      using enum config::LED;
                      case eLED_None: return _("<None>");
                      case eLED_Cree_Red: return _("Cree Red");
                      case eLED_Cree_Green: return _("Cree Green");
                      case eLED_Cree_Blue: return _("Cree Blue");
                      case eLED_Cree_Amber: return _("Cree Amber");
                      case eLED_Cree_Red_Orange: return _("Cree Red-Orange");
                      case eLED_Cree_White: return _("Cree White");
                      case eLED_Red: return _("Red");
                      case eLED_Green: return _("Green");
                      case eLED_Blue: return _("Blue");
                      case eLED_Max: break;
                  }
                  return {};
              }
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .win_={
                .base_={.expand_=true},
                .enable_=led.powerPin_ | data::logic::IsEnabled{},
              },
              .label_=_("Power Pin"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::ComboBox{
                .win_={.base_={.expand_=true}},
                .data_=led.powerPin_,
                .defaults_={
                  "bladePowerPin1",
                  "bladePowerPin2",
                  "bladePowerPin3",
                  "bladePowerPin4",
                  "bladePowerPin5",
                  "bladePowerPin6",
                }
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .win_={
                .base_={.expand_=true},
                .enable_=led.resistance_ | data::logic::IsEnabled{},
                .tooltip_=_("The value of the resistor placed in series with this led."),
              },
              .label_=_("Resistance (mOhms)"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.expand_=true}},
                .data_=led.resistance_,
              }(),
            }(),
          }
        }();
    }};

    return pcui::Stack{
      .base_={.expand_=true, .proportion_=1},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true, .proportion_=1},
          .orient_=wxHORIZONTAL,
          .children_={
            led(simple.led1_, 1),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            led(simple.led2_, 2),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true, .proportion_=1},
          .orient_=wxHORIZONTAL,
          .children_={
            led(simple.led3_, 3),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            led(simple.led4_, 4),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Labeled{
          .label_=_("Brightness"),
          .orient_=wxHORIZONTAL,
          .ctrl_=pcui::Stepper{
            .data_=blade.brightness_,
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr BladesPage::ws281x(config::blades::WS281X& ws281x) {
    auto& blade{ws281x.parent_};

    { auto ctxt{data::context(mSubBladeSel)};
        ctxt.bind(&ws281x.splits_);

        auto selChoice{data::context(mSubBladeSel.choice())};
        if (
                mLastSubChoice != -1 and
                mLastSubChoice < selChoice.num()
           ) {
            selChoice.choose(mLastSubChoice);
        }
    }

    const auto onAddPowerPin{[this, &ws281x] {
        auto entry{data::context(mPowerPinAddField)};

        // Could be empty coming from add field enter action.
        if (entry.val().empty()) return;

        auto powerPins{data::context(ws281x.powerPins_)};

        powerPins.select(std::string{entry.val()});
        entry.clear();
    }};

    return pcui::Stack{
      .base_={.expand_=true, .proportion_=1},
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Group{
          .win_={.base_={.expand_=true}},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Labeled{
              .win_={.base_={.expand_=true}},
              .label_=_("Number of Pixels"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Stepper{
                .data_=ws281x.length_,
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .win_={
                .base_={.expand_=true},
                .tooltip_=_("The pin name or number used for WS281X data."),
              },
              .label_=_("Data Pin"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::ComboBox{
                .win_={.base_={.proportion_=1}},
                .data_=ws281x.dataPin_,
                .defaults_={
                  "bladePin",
                  "blade2Pin",
                  "blade3Pin",
                  "blade4Pin",
                }
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Panel{
              .win_={
                .base_={.expand_=true},
                .show_=not (ws281x.hasWhite_ | data::logic::IsSet{}),
              },
              .child_=pcui::Labeled{
                .win_={
                  .base_={
                    .expand_=true,
                    .border_={
                      .size_=pcui::interControlSpacing(),
                      .dirs_=wxBOTTOM
                    },
                  },
                  .tooltip_=_("The order of colors for your blade.\nMost of the time this can be left as \"GRB\"."),
                },
                .label_=_("Color Order"),
                .orient_=wxHORIZONTAL,
                .ctrl_=pcui::Choice{
                  .win_={.base_={.proportion_=1}},
                  .data_=ws281x.colorOrder3_,
                  .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                      switch (static_cast<config::ColorOrder3>(idx)) {
                          using enum config::ColorOrder3;
                          case eOrder3_GRB: return _("GRB");
                          case eOrder3_GBR: return _("GBR");
                          case eOrder3_BGR: return _("BGR");
                          case eOrder3_BRG: return _("BRG");
                          case eOrder3_RGB: return _("RGB");
                          case eOrder3_RBG: return _("RBG");
                          case eOrder3_Max: break;
                      }
                      return {};
                  }
                }(),
              }(),
            }(),
            pcui::Panel{
              .win_={
                .base_={.expand_=true},
                .show_=ws281x.hasWhite_ | data::logic::IsSet{},
              },
              .child_=pcui::Labeled{
                .win_={
                  .base_={
                    .expand_=true,
                    .border_={
                      .size_=pcui::interControlSpacing(),
                      .dirs_=wxBOTTOM
                    },
                  },
                  .tooltip_=_("The order of colors for your blade.\nMost of the time this can be left as \"GRBW\"."),
                },
                .label_=_("Color Order"),
                .orient_=wxHORIZONTAL,
                .ctrl_=pcui::Choice{
                  .win_={.base_={.proportion_=1}},
                  .data_=ws281x.colorOrder4_,
                  .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                      switch (static_cast<config::ColorOrder4>(idx)) {
                          using enum config::ColorOrder4;
                          case eOrder4_GRBW: return _("GRBW");
                          case eOrder4_GBRW: return _("GBRW");
                          case eOrder4_BGRW: return _("BGRW");
                          case eOrder4_BRGW: return _("BRGW");
                          case eOrder4_RGBW: return _("RGBW");
                          case eOrder4_RBGW: return _("RBGW");
                          case eOrder4_WGRB: return _("WGRB");
                          case eOrder4_WGBR: return _("WGBR");
                          case eOrder4_WBGR: return _("WBGR");
                          case eOrder4_WBRG: return _("WBRG");
                          case eOrder4_WRGB: return _("WRGB");
                          case eOrder4_WRBG: return _("WRBG");
                          case eOrder4_Max: break;
                      }
                      return {};
                  }
                }(),
              }(),
            }(),
            pcui::CheckBox{
              .win_={.base_={.align_=wxALIGN_RIGHT}},
              .label_=_("LEDs Have White Channel"),
              .data_=ws281x.hasWhite_,
            }(),
            pcui::CheckBox{
              .win_={
                .base_={
                  .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
                  .align_=wxALIGN_RIGHT,
                },
                .show_=ws281x.hasWhite_ | data::logic::IsSet{},
                .tooltip_=_("Use the RGB channels alongside the White channel to produce white light.\nThis can result in a brighter blade, but at the cost of higher battery usage and a less \"pure\" white."),
              },
              .label_=_("Use RGB With White"),
              .data_=ws281x.useRgbWithWhite_
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .win_={.base_={.expand_=true}},
              .label_=_("Brightness"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.proportion_=1}},
                .data_=blade.brightness_,
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Group{
              .win_={
                .base_={
                  .minSize_={-1, 200},
                  .expand_=true,
                  .proportion_=1,
                },
                .tooltip_={
                  _("The power pins to use for this blade.") + '\n' +
                  _("WS281X blades can have as many as are desired, though 2 is generally enough for most.")
                },
              },
              .label_=_("Power Pins"),
              .orient_=wxVERTICAL,
              .children_={
                pcui::CheckList{
                  .win_={.base_={.expand_=true, .proportion_=1}},
                  .data_=ws281x.powerPins_,
                }(),
                pcui::Stack{
                  .base_={.expand_=true},
                  .orient_=wxHORIZONTAL,
                  .children_={
                    pcui::Text{
                      .win_={.base_={.proportion_=1}},
                      .data_=mPowerPinAddField,
                      .style_=pcui::Text::SingleLine{
                        .hint_=_("Pin Name"),
                        .onEnter_=onAddPowerPin,
                      },
                    }(),
                    pcui::Button{
                      .win_={
                        .base_={.minSize_=pcui::iconButtonSize()},
                        .enable_=not (mPowerPinAddField |
                            data::logic::IsEmpty{}),
                      },
                      .label_=pcui::syms::PLUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=onAddPowerPin,
                    }(),
                  }
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        SplitVisualizer{
          .base_={.expand_=true, .proportion_=1},
          .blade_=ws281x,
          .subSel_=mSubBladeSel,
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        splits(ws281x),
      }
    }();
}

pcui::DescriptorPtr BladesPage::splits(config::blades::WS281X& ws281x) {
    return pcui::Group{
      .win_={.base_={.expand_=true, .proportion_=4}},
      .label_=_("SubBlades"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::Choice{
          .win_={.base_={.expand_=true}},
          .data_=mSubBladeSel,
          .style_=pcui::Choice::PopUp{
            .unselected_=_("Select SubBlade"),
          },
          .labeler_=[](uint32 idx) -> pcui::Choice::Label {
              return wxString::Format(_("SubBlade %d"), idx);
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Button{
              .win_={.base_={.proportion_=1}},
              .label_=_("Add"),
              .func_=[this, &ws281x] { onAddSplitButton(ws281x); },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.proportion_=1},
                .enable_=mSubBladeSel.choice() | data::logic::HasSelection{},
              },
              .label_=_("Remove"),
              .func_=[this, &ws281x] { onRemoveSplitButton(ws281x); },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::builders::Selector{
          .data_=mSubBladeSel,
          .builder_=[this](data::base::Model *model) -> pcui::DescriptorPtr {
              if (model == nullptr) {
                  return pcui::Stack{
                    .base_={.expand_=true, .proportion_=1},
                    .orient_=wxVERTICAL,
                    .children_={
                      pcui::StretchSpacer{}(),
                      pcui::Label{
                        .win_={
                          .base_={.proportion_=1, .align_=wxALIGN_CENTER},
                        },
                        .label_=_("Add SubBlades to edit them here."),
                        .color_=wxSYS_COLOUR_GRAYTEXT,
                        .wrapWidth_=120,
                      }(),
                      pcui::StretchSpacer{}(),
                    }
                  }();
              }

              using Split = config::blades::WS281X::Split;
              return split(dynamic_cast<Split&>(*model));
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr BladesPage::split(config::blades::WS281X::Split& split) {
    using enum config::blades::WS281X::Split::Type;

    return pcui::Stack{
      .base_={.expand_=true, .proportion_=1},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Radios{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_(
              "Standard: Split data into continuous sections.\n\n"
              "Reverse: Identical to Standard, but reverses the bladestyle direction.\n\n"
              "Stride: Useful for KR style blades where the data signal \"strides\" back and forth across sides.\n\n"
              "ZigZag: Similar to Stride, but organizes data in the opposite manner perpendicular to the data signal.\n\n"
              "List: Discrete LEDs to make part of a SubBlade."
            ),
          },
          .label_=_("Type"),
          .data_=split.type_,
          .radios_={
            pcui::Radios::Radio{.label_=_("Standard")},
            pcui::Radios::Radio{.label_=_("Reverse")},
            pcui::Radios::Radio{.label_=_("Stride")},
            pcui::Radios::Radio{.label_=_("ZigZag")},
            pcui::Radios::Radio{
                .win_={
                  .show_=split.root<config::Config>() |
                      config::Config::OSIsOrOverVersion{.ver_={8}},
                },
                .label_=_("List")
            },
          },
        }(),
        pcui::Panel{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxTOP},
            },
            .show_=[&] {
                auto type{data::context(split.type_)};
                return 
                    (type[eStandard] | data::logic::IsSet{}) or
                    (type[eReverse] | data::logic::IsSet{}) or
                    (type[eStride] | data::logic::IsSet{}) or
                    (type[eZig_Zag] | data::logic::IsSet{});
            }(),
          },
          .child_=pcui::Stack{
            .base_={.expand_=true, .proportion_=1},
            .orient_=wxHORIZONTAL,
            .children_={
              pcui::Labeled{
                .win_={.base_={.proportion_=1}},
                .label_=_("Start"),
                .orient_=wxVERTICAL,
                .ctrl_=pcui::Stepper{
                  .win_={.base_={.expand_=true}},
                  .data_=split.start_,
                }(),
              }(),
              pcui::Spacer{.size_=pcui::interControlSpacing()}(),
              pcui::Labeled{
                .win_={.base_={.proportion_=1}},
                .label_=_("End"),
                .orient_=wxVERTICAL,
                .ctrl_=pcui::Stepper{
                  .win_={.base_={.expand_=true}},
                  .data_=split.end_,
                }(),
              }(),
            }
          }(),
        }(),
        pcui::Labeled{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
            },
            .show_=[&] {
                auto type{data::context(split.type_)};
                return 
                    (type[eStandard] | data::logic::IsSet{}) or
                    (type[eReverse] | data::logic::IsSet{}) or
                    (type[eStride] | data::logic::IsSet{}) or
                    (type[eZig_Zag] | data::logic::IsSet{});
            }(),
          },
          .label_=_("Length"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Stepper{
            .win_={
              .base_={.expand_=true},
              .tooltip_=_("The number of pixels in your blade (total)."),
            },
            .data_=split.length_,
          }(),
        }(),
        pcui::Labeled{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
            },
            .show_=[&] {
                auto type{data::context(split.type_)};
                return 
                    (type[eStride] | data::logic::IsSet{}) or
                    (type[eZig_Zag] | data::logic::IsSet{});
            }(),
            .tooltip_=_("Stride length or number of ZigZag columns"),
          },
          .label_=_("Segments"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Stepper{
            .win_={
              .base_={.expand_=true},
            },
            .data_=split.segments_,
          }(),
        }(),
        pcui::Labeled{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
            },
            .show_=[&] {
                auto type{data::context(split.type_)};
                return type[eList] | data::logic::IsSet{};
            }(),
            .tooltip_=_("Data goes along each LED according to their order in the list")
          },
          .label_=_("List"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Text{
            .win_={.base_={.expand_=true}},
            .data_=split.list_,
          }(),
        }(),
        pcui::StretchSpacer{}(),
        pcui::Labeled{
          .win_={.base_={.expand_=true}},
          .label_=_("Brightness"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Stepper{
            .win_={.base_={.expand_=true}},
            .data_=split.brightness_,
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr BladesPage::servo(config::blades::Servo& servo) {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::StretchSpacer{}(),
        pcui::Group{
          .win_={
            .base_={
              .minSize_={140, -1},
              .align_=wxALIGN_CENTER,
            },
          },
          .children_={
            pcui::Labeled{
              .win_={.base_={.expand_=true}},
              .label_=_("Signal Pin"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::Text{
                .win_={.base_={.expand_=true}},
                .data_=servo.sigPin_,
              }(),
            }(),
          }
        }(),
        pcui::StretchSpacer{}(),
      }
    }();
}

void BladesPage::onAwarenessButton(const pcui::CallbackContext& ctxt) {
    if (not mAwarenessDlg) {
        mAwarenessDlg = new BladeAwarenessDlg(
            ctxt.topLevel_, mConfig
        );
    }

    mAwarenessDlg->Show();
    mAwarenessDlg->Raise();
}

void BladesPage::onEditButton(const pcui::CallbackContext& ctxt) {
    if (mArrayDlg) {
        mArrayDlg->Show();
        mArrayDlg->Raise();
        return;
    }

    using namespace config::blades;

    auto sel{data::context(mArraySel)};
    auto& cfg{dynamic_cast<BladeConfig&>(*sel.selected())};
    mArrayDlg = new BladeArrayDlg(ctxt.topLevel_, cfg, false);
    const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
        if (evt.GetEventObject() == mArrayDlg) mArrayDlg = nullptr;
    }};
    mArrayDlg->Bind(wxEVT_DESTROY, onDestroy);

    mArrayDlg->Show();
}

void BladesPage::onAddButton(const pcui::CallbackContext& ctxt) {
    // Only ever allow one of these dialogs. Not a technical
    // limitation, just don't want things cluttered.
    if (mArrayDlg)
        mArrayDlg->Destroy();

    auto vec{data::context(mConfig.bladeConfigs_)};
    auto& cfg{vec.append<config::blades::BladeConfig>(mConfig)};

    BladeArrayDlg dlg(ctxt.topLevel_, cfg, true);

    int res{};
    { data::hier::Model::CreationScope scope(&cfg);
        res = dlg.ShowModal();
    }

    if (res != wxID_OK) {
        // Undo append
        data::context(mConfig).undo();
    } else {
        mArraySel.choice().choose(
            static_cast<int32>(vec.children().size() - 1)
        );
    }
}

void BladesPage::onRemoveButton() {
    auto sel{data::context(mArraySel)};
    auto vec{data::context(mConfig.bladeConfigs_)};

    vec.remove(*sel.selected());
}

void BladesPage::onAddBladeButton() {
    auto sel{data::context(mBladeSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.append(std::make_unique<config::blades::Blade>(mConfig));
}

void BladesPage::onRemoveBladeButton() {
    auto sel{data::context(mBladeSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.remove(sel.choiceIdx());
}

void BladesPage::onAddSplitButton(config::blades::WS281X& ws281x) {
    auto vec{data::context(ws281x.splits_)};
    auto choice{data::context(mSubBladeSel.choice())};

    vec.append(std::make_unique<config::blades::WS281X::Split>(ws281x));
    choice.choose(static_cast<int32>(vec.children().size() - 1));
}

void BladesPage::onRemoveSplitButton(config::blades::WS281X& ws281x) {
    auto sel{data::context(mSubBladeSel)};
    auto vec{data::context(ws281x.splits_)};

    vec.remove(*sel.selected());
}

void BladesPage::onArrayChoice() {
    if (mArrayDlg) {
        pcui::cripple(mArrayDlg);

        mArrayDlg->CallAfter([dlg=mArrayDlg] {
            dlg->Destroy();
        });
    }

    auto arraySel{data::context(mArraySel)};

    // Always detach first
    detachIssues();

    using namespace config::blades;
    if (auto *array{arraySel.selected<BladeConfig>()}) {
        attachIssues(array->issues());

        mBladeSel.bind(&array->blades_);

        auto selChoice{data::context(mBladeSel.choice())};
        if (
                mLastBladeChoice != -1 and
                mLastBladeChoice < selChoice.num()
           ) {
            selChoice.choose(mLastBladeChoice);
        }
    } else {
        mBladeSel.bind(nullptr);
    }
}

void BladesPage::onBladeChoice() {
    auto bladeSel{data::context(mBladeSel)};

    // Only preserve real choices.
    if (bladeSel.choiceIdx() != -1)
        mLastBladeChoice = bladeSel.choiceIdx();

    using namespace config::blades;
    if (auto *blade{bladeSel.selected<Blade>()}) {
        mSubBladeSel.bind(&blade->ws281x().splits_);

        auto selChoice{data::context(mSubBladeSel.choice())};
        if (
            mLastSubChoice != -1 and
            mLastSubChoice < selChoice.num()
           ) {
            selChoice.choose(mLastSubChoice);
        }
    } else {
        mSubBladeSel.bind(nullptr);
    }
}

void BladesPage::onSubChoice() {
    auto subSel{data::context(mSubBladeSel)};

    if (subSel.choiceIdx() != -1)
        mLastSubChoice = subSel.choiceIdx();
}

void BladesPage::attachIssues(const data::base::Integer& issues) {
    mIssues = &issues;

    static const auto issueTable{[] {
        data::base::Integer::RecvTable table;
        table.onSet_ = data::map<&BladesPage::onIssues>();
        return table;
    }()};
    observeWith(issues, issueTable);

    // Update the label
    onIssues();
}

void BladesPage::detachIssues() {
    if (mIssues == nullptr) return;

    mIssueLabel.clear();

    repeal(*mIssues);
}

void BladesPage::onIssues() {
    auto issues{data::context(*mIssues)};

    using enum config::blades::BladeConfig::Issues;

    std::string label;
    if (issues.val() & eIssue_Errors) {
        label = pcui::syms::NO_ENTRY;
    } else if (issues.val() & eIssue_Warnings) {
        label = pcui::syms::WARNING;
    }

    mIssueLabel.change(std::move(label));
}

