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

#include "config/blades/bladeconfig.hpp"
#include "config/blades/ws281x.hpp"
#include "config/strings.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
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
#include "ui/layout/selector.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/parent.hpp"
#include "utils/string.hpp"

#include "../special/splitvisualizer.hpp"

BladesPage::BladesPage(config::Config& config) : mConfig{config} {
    mArraySel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& arraySel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&BladesPage::mArraySel>(arraySel)};

        if (page.mDlg) {
            pcui::cripple(page.mDlg);

            page.mDlg->CallAfter([dlg=page.mDlg] {
                dlg->Destroy();
            });
        }

        data::Selector::Context bladeSel{page.mBladeSel};

        // Always detach first
        page.mIssueReceiver.detach();

        if (ctxt.idx() != -1) {
            using namespace config::blades;
            data::Vector::ROContext bladeConfigs{page.mConfig.bladeConfigs_};
            auto& selModel{*bladeConfigs.children()[ctxt.idx()]};
            auto& selected{static_cast<BladeConfig&>(selModel)};
            page.mIssueReceiver.attach(selected.issues());

            bladeSel.bind(&selected.blades_);

            data::Choice::Context selChoice{page.mBladeSel.choice_};
            if (
                    page.mLastBladeChoice != -1 and
                    page.mLastBladeChoice < selChoice.numChoices()
               ) {
                selChoice.choose(page.mLastBladeChoice);
            }
        } else {
            bladeSel.bind(nullptr);
        }
    };

    mBladeSel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& bladeSel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&BladesPage::mBladeSel>(bladeSel)};

        // Only preserve real choices.
        if (ctxt.idx() != -1)
            page.mLastBladeChoice = ctxt.idx();

        data::Selector::Context{page.mSubBladeSel}.bind(nullptr);
    };

    mSubBladeSel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& subSel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&BladesPage::mSubBladeSel>(subSel)};

        if (ctxt.idx() != -1)
            page.mLastSubChoice = ctxt.idx();
    };

    const auto powerPinFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
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

    data::Selector::Context{mArraySel}.bind(&config.bladeConfigs_);
}

void BladesPage::deinit() {
    data::Selector::Context{mArraySel}.bind(nullptr);
    mIssueReceiver.detach();
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
                      data::Vector::ROContext vec{mConfig.bladeConfigs_};
                      auto& cfg{static_cast<config::blades::BladeConfig&>(
                          *vec.children()[idx]
                      )};
                      return cfg.name_;
                  },
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={
                    .enable_=mArraySel.choice_ | data::logic::HasSelection{},
                  },
                  .bitmap_={
                    .src_=pcui::Bitmap{"edit"}.color(wxSYS_COLOUR_WINDOWTEXT),
                  },
                  .exactFit_=true,
                  .func_=[this](const pcui::CallbackContext& ctxt) {
                      if (mDlg) {
                          mDlg->Show();
                          mDlg->Raise();
                          return;
                      }

                      using namespace config::blades;

                      data::Selector::Context sel{mArraySel};
                      auto& cfg{static_cast<BladeConfig&>(*sel.selected())};
                      mDlg = new BladeArrayDlg(ctxt.topLevel_, cfg, false);
                      const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
                          if (evt.GetEventObject() == mDlg) mDlg = nullptr;
                      }};
                      mDlg->Bind(wxEVT_DESTROY, onDestroy);

                      mDlg->Show();
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
                      // Only ever allow one of these dialogs. Not a technical
                      // limitation, just don't want things cluttered.
                      if (mDlg)
                          mDlg->Destroy();

                      data::Vector::Context vec{mConfig.bladeConfigs_};
                      auto& cfg{vec.addCreate<config::blades::BladeConfig>()};

                      // For new creation, for things like this, it would make
                      // more sense to create a temporary and then add it in
                      // later on confirm, so as to not clutter the action
                      // tree.
                      BladeArrayDlg dlg(ctxt.topLevel_, cfg, true);

                      auto res{dlg.ShowModal()};
                      if (res != wxID_OK) {
                          vec.remove(vec.children().size() - 1);
                      } else {
                          data::Choice::Context{mArraySel.choice_}.choose(
                              static_cast<int32>(vec.children().size() - 1)
                          );
                      }
                  }
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={
                    .base_={.proportion_=1},
                    .enable_=mArraySel.choice_ | data::logic::HasSelection{},
                  },
                  .label_=_("Remove"),
                  .exactFit_=true,
                  .func_=[this] {
                      data::Selector::ROContext sel{mArraySel};
                      data::Vector::Context vec{mConfig.bladeConfigs_};

                      vec.remove(*sel.selected());
                  }
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
                    .enable_=mArraySel.choice_ | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::PLUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] {
                      data::Selector::ROContext sel{mBladeSel};
                      data::Vector::Context vec{
                          const_cast<data::Vector&>(*sel.bound())
                      };
                      vec.addCreate<config::blades::Blade>();
                  },
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize()},
                    .enable_=mBladeSel.choice_ | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::MINUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] {
                      data::Selector::ROContext sel{mBladeSel};
                      data::Vector::Context vec{
                          const_cast<data::Vector&>(*sel.bound())
                      };
                      vec.remove(*sel.selected());
                  },
                }(),
              }
            }(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr BladesPage::blades() {
    const auto typeBuilder{[this](data::Model *model) -> pcui::DescriptorPtr {
        using namespace config::blades;

        if (auto *ptr{dynamic_cast<WS281X *>(model)}) {
            return ws281x(*ptr);
        }
        if (auto *ptr{dynamic_cast<Simple *>(model)}) {
            return simple(*ptr);
        }

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
        data::Model *model
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

        auto& blade{*static_cast<config::blades::Blade *>(model)};
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
                    return {};
                }
              }(),
              pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
              pcui::Selector{
                .data_=blade.type(),
                .builder_=typeBuilder,
              }(),
            }
        }();
    }};

    return pcui::Selector{
      .data_=mBladeSel,
      .builder_=builder,
    }();
}

pcui::DescriptorPtr BladesPage::simple(config::blades::Simple& simple) {
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

    auto& blade{*simple.parent()->parent<config::blades::Blade>()};

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
    { data::Selector::Context ctxt{mSubBladeSel};
        ctxt.bind(&ws281x.splits_);

        data::Choice::Context selChoice{mSubBladeSel.choice_};
        if (
                mLastSubChoice != -1 and
                mLastSubChoice < selChoice.numChoices()
           ) {
            selChoice.choose(mLastSubChoice);
        }
    }

    auto& blade{*ws281x.parent()->parent<config::blades::Blade>()};

    const auto onAddPowerPin{[this, &ws281x] {
        data::String::Context entry{mPowerPinAddField};

        // Could be empty coming from add field enter action.
        if (entry.val().empty()) return;

        data::Selection::Context powerPins{ws281x.powerPins_};

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
              .func_=[this, &ws281x] {
                  data::Vector::Context vec{ws281x.splits_};
                  data::Choice::Context choice{mSubBladeSel.choice_};
                  vec.addCreate<config::blades::WS281X::Split>();
                  choice.choose(static_cast<int32>(
                      vec.children().size() - 1
                  ));
              }
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.proportion_=1},
                .enable_=mSubBladeSel.choice_ | data::logic::HasSelection{},
              },
              .label_=_("Remove"),
              .func_=[this, &ws281x] {
                  data::Selector::ROContext sel{mSubBladeSel};
                  data::Vector::Context vec{ws281x.splits_};

                  vec.remove(*sel.selected());
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Selector{
          .data_=mSubBladeSel,
          .builder_=[this](data::Model *model) -> pcui::DescriptorPtr {
              if (model == nullptr) {
                  return pcui::Stack{
                    .base_={.expand_=true, .proportion_=1},
                    .orient_=wxVERTICAL,
                    .children_={
                      pcui::StretchSpacer{}(),
                      pcui::Label{
                        .win_={.base_={.proportion_=1, .align_=wxALIGN_CENTER}},
                        .label_=_("Add SubBlades to edit them here."),
                        .color_=wxSYS_COLOUR_GRAYTEXT,
                        .wrapWidth_=120,
                      }(),
                      pcui::StretchSpacer{}(),
                    }
                  }();
              }

              using Split = config::blades::WS281X::Split;
              return split(static_cast<Split&>(*model));
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
          .labels_={
            _("Standard"),
            _("Reverse"),
            _("Stride"),
            _("ZigZag"),
            _("List"),
          },
        }(),
        pcui::Panel{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxTOP},
            },
            .show_={
              (*split.type_.data()[eStandard] | data::logic::IsSet{}) or
              (*split.type_.data()[eReverse] | data::logic::IsSet{}) or
              (*split.type_.data()[eStride] | data::logic::IsSet{}) or
              (*split.type_.data()[eZig_Zag] | data::logic::IsSet{})
            },
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
            .show_={
              (*split.type_.data()[eStandard] | data::logic::IsSet{}) or
              (*split.type_.data()[eReverse] | data::logic::IsSet{}) or
              (*split.type_.data()[eStride] | data::logic::IsSet{}) or
              (*split.type_.data()[eZig_Zag] | data::logic::IsSet{})
            },
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
            .show_={
              (*split.type_.data()[eStride] | data::logic::IsSet{}) or
              (*split.type_.data()[eZig_Zag] | data::logic::IsSet{})
            },
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
            .show_=*split.type_.data()[eList] | data::logic::IsSet{},
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

void BladesPage::IssueReceiver::onSet() {
    updateLabel();
}

void BladesPage::IssueReceiver::onAttach() {
    updateLabel();
}

void BladesPage::IssueReceiver::preDetach() {
    auto& page{utils::parent<&BladesPage::mIssueReceiver>(*this)};
    data::String::Context{page.mIssueLabel}.clear();
}

void BladesPage::IssueReceiver::updateLabel() {
    auto& page{utils::parent<&BladesPage::mIssueReceiver>(*this)};

    const auto issues{context<data::Integer>().val()};
    using enum config::blades::BladeConfig::Issues;

    std::string label;
    if (issues & eIssue_Errors) {
        label = pcui::syms::NO_ENTRY;
    } else if (issues & eIssue_Warnings) {
        label = pcui::syms::WARNING;
    }

    data::String::Context{page.mIssueLabel}.change(std::move(label));
}
 
