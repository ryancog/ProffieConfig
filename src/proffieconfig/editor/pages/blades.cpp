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

BladesPage::~BladesPage() {
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
                pcui::Button{
                  .win_={
                    .base_={
                      .border_={
                        .size_=pcui::interControlSpacing(),
                        .dirs_=wxLEFT
                      },
                    },
                    .show_=not (mIssueLabel | data::logic::IsEmpty{}),
                  },
                  .label_=mIssueLabel,
                  .exactFit_=true,
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
                      if (mDlg) mDlg->Destroy();

                      data::Vector::Context vec{mConfig.bladeConfigs_};
                      auto& cfg{vec.addCreate<config::blades::BladeConfig>()};

                      // For new creation, for things like this, it would make
                      // more sense to create a temporary and then add it in
                      // later on confirm, so as to not clutter the action
                      // tree.
                      BladeArrayDlg dlg(ctxt.topLevel_, cfg, true);

                      auto res{dlg.ShowModal()};
                      if (res != wxID_OK) {
                          pcui::cripple(&dlg);
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

        return pcui::Spacer{.size_=0}();
    }};

    const auto builder{[this, typeBuilder](
        data::Model *model
    ) -> pcui::DescriptorPtr {
        if (model == nullptr) {
            return pcui::Label{
              .win_={
                .base_={
                  .minSize_={350, -1},
                  .proportion_=1,
                  .align_=wxALIGN_CENTER,
                },
              },
              .label_=_("No Blade Selected"),
              .style_=pcui::text::Style::Header,
              .color_=wxSYS_COLOUR_GRAYTEXT,
            }();
        }

        auto& blade{*static_cast<config::blades::Blade *>(model)};
        return pcui::Stack{
            .base_={.expand_=true, .proportion_=1},
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
    return pcui::Stack{
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
              .win_={.base_={.expand_=true}},
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
                      .mode_=pcui::Text::SingleLine{
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
          .base_={.expand_=true},
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
      .win_={.base_={.expand_=true}},
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
        pcui::Stack{
          .base_={
            .expand_=true,
            .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxTOP},
          },
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Labeled{
              .win_={
                .base_={.proportion_=1},
                .show_={
                  (*split.type_.data()[eStandard] | data::logic::IsSet{}) or
                  (*split.type_.data()[eReverse] | data::logic::IsSet{}) or
                  (*split.type_.data()[eStride] | data::logic::IsSet{}) or
                  (*split.type_.data()[eZig_Zag] | data::logic::IsSet{})
                },
              },
              .label_=_("Start"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.expand_=true}},
                .data_=split.start_,
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .win_={
                .base_={.proportion_=1},
                .show_={
                  (*split.type_.data()[eStandard] | data::logic::IsSet{}) or
                  (*split.type_.data()[eReverse] | data::logic::IsSet{}) or
                  (*split.type_.data()[eStride] | data::logic::IsSet{}) or
                  (*split.type_.data()[eZig_Zag] | data::logic::IsSet{})
                },
              },
              .label_=_("End"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::Stepper{
                .win_={.base_={.expand_=true}},
                .data_=split.end_,
              }(),
            }(),
          }
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
            .win_={.base_={.expand_=true}},
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
 
/*
void BladesPage::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto res{pcui::showMessage(
            "This action cannot be undone!",
            "Remove Blade Array?",
            wxYES_NO | wxNO_DEFAULT
        )};
        if (res != wxYES) return;
        auto& bladeArrays{mParent->getOpenConfig().bladeArrays};
        bladeArrays.removeArray(bladeArrays.arraySelection);
    }, ID_RemoveArray);
}
*/

/*
void BladesPage::handleNotification(uint32 id) {
    bool rebound{id == pcui::Notifier::eID_Rebound};
    auto& bladeArrays{mParent->getOpenConfig().bladeArrays};

    if (rebound or id == Config::BladeArrays::ID_ARRAY_SELECTION) {
        bool hasSelection{bladeArrays.arraySelection != -1};
        FindWindow(ID_EditArray)->Enable(hasSelection);
        FindWindow(ID_RemoveArray)->Enable(hasSelection);
        FindWindow(ID_AddBlade)->Enable(hasSelection);
    }
    if (
            rebound or
            id == Config::BladeArrays::ID_ARRAY_SELECTION or
            id == Config::BladeArrays::ID_ARRAY_ISSUES
       ) {
        const auto issues{bladeArrays.arrayIssues};

        auto *issueIcon{FindWindow(ID_IssueIcon)};
        if (issues & Config::BladeConfig::ISSUE_ERRORS) {
            issueIcon->SetLabel(L"\u26D4"); // ⛔️

        } else if (issues & Config::BladeConfig::ISSUE_WARNINGS) {
            issueIcon->SetLabel(L"\u26A0"); // ⚠️
        }
        issueIcon->Show(issues != Config::BladeConfig::ISSUE_NONE);
    }
    if (
            rebound or 
            id == Config::BladeArrays::ID_ARRAY_SELECTION or
            id == Config::BladeArrays::ID_BLADE_SELECTION or
            id == Config::BladeArrays::ID_BLADE_TYPE_SELECTION
       ) {
        bool hasSelection{
            bladeArrays.bladeSelectionProxy.data() and
            *bladeArrays.bladeSelectionProxy.data() != -1
        };
        bool hasTypeSelection{
            bladeArrays.bladeTypeProxy.data() and
            *bladeArrays.bladeTypeProxy.data() != -1
        };
        bool isSimple{
            hasTypeSelection and
            *bladeArrays.bladeTypeProxy.data() == Config::Blade::SIMPLE
        };
        bool isPixel{
            hasTypeSelection and
            *bladeArrays.bladeTypeProxy.data() == Config::Blade::WS281X
        };

        FindWindow(ID_RemoveBlade)->Enable(hasSelection);

        mSimpleSizer->Show(isSimple);
        mPixelSizer->Show(isPixel);

        FindWindow(ID_NoSelectText)->Show(not hasSelection);
    }
    if (
            rebound or 
            id == Config::BladeArrays::ID_ARRAY_SELECTION or
            id == Config::BladeArrays::ID_BLADE_SELECTION or
            id == Config::BladeArrays::ID_BLADE_TYPE_SELECTION or
            id == Config::BladeArrays::ID_SPLIT_SELECTION
       ) {
        auto hasSelection{
            bladeArrays.splitSelectionProxy.data() and
            *bladeArrays.splitSelectionProxy.data() != -1
        };
        FindWindow(ID_RemoveSplit)->Enable(hasSelection);
    }
}

wxSizer *BladesPage::createBladeSettings() {
    auto& config{mParent->getOpenConfig()};

    auto *settingsSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *noSelectText(new wxStaticText(
        this,
        ID_NoSelectText,
        "No Blade Selected",
        wxDefaultPosition,
        wxSize(350, -1),
        wxALIGN_CENTER
    ));
    noSelectText->SetFont(noSelectText->GetFont().MakeLarger());

    auto *setupSizer{new wxBoxSizer(wxVERTICAL)};

    auto *bladeType{new pcui::Choice(
        this,
        config.bladeArrays.bladeTypeProxy,
        _("Blade Type")
    )};

    auto starSizer{[this](
            Config::BladeArrays::StarProxy& starProxy,
            const wxString& label
        ) {
        auto *starSizer{new pcui::StaticBox(
            wxVERTICAL,
            this,
            label
        )};
        auto *starColor{new pcui::Choice(
            starSizer->childParent(),
            starProxy.ledProxy
        )};
        auto *starPowerPin{new pcui::ComboBox(
            starSizer->childParent(),
            starProxy.powerPinProxy,
            _("Power Pin")
        )};
        auto *starResistance{new pcui::Numeric(
            starSizer->childParent(),
            starProxy.resistanceProxy,
            _("Resistance (mOhms)")
        )};
        starResistance->SetToolTip(_("The value of the resistor placed in series with this led."));

        starSizer->Add(starColor, wxSizerFlags().Expand());
        starSizer->AddSpacer(10);
        starSizer->Add(
            starPowerPin, 
            wxSizerFlags().Border(wxLEFT, 20).Expand()
        );
        starSizer->AddSpacer(5);
        starSizer->Add(
            starResistance,
            wxSizerFlags().Border(wxLEFT, 20).Expand()
        );

        return starSizer;
    }};

    mSimpleSizer = new wxBoxSizer(wxVERTICAL);
    auto *simpleSplit1Sizer{new wxBoxSizer(wxHORIZONTAL)};
    simpleSplit1Sizer->Add(starSizer(config.bladeArrays.star1Proxy, _("LED 1")));
    simpleSplit1Sizer->AddSpacer(10);
    simpleSplit1Sizer->Add(starSizer(config.bladeArrays.star2Proxy, _("LED 2")));
    auto *simpleSplit2Sizer{new wxBoxSizer(wxHORIZONTAL)};
    simpleSplit2Sizer->Add(starSizer(config.bladeArrays.star3Proxy, _("LED 3")));
    simpleSplit2Sizer->AddSpacer(10);
    simpleSplit2Sizer->Add(starSizer(config.bladeArrays.star4Proxy, _("LED 4")));
    auto *simpleBrightness{new pcui::Numeric(
        this,
        config.bladeArrays.simpleBrightnessProxy,
        _("Brightness"),
        wxHORIZONTAL
    )};
    mSimpleSizer->Add(simpleSplit1Sizer);
    mSimpleSizer->AddSpacer(10);
    mSimpleSizer->Add(simpleSplit2Sizer);
    mSimpleSizer->AddSpacer(10);
    mSimpleSizer->Add(simpleBrightness);

    mPixelSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *pixelMainSizer{new wxBoxSizer(wxVERTICAL)};
    auto *length{new pcui::Numeric(
        this,
        config.bladeArrays.lengthProxy,
        _("Number of Pixels"),
        wxHORIZONTAL
    )};
    length->SetToolTip(_("The number of pixels in your blade (total)."));
    auto *dataPin{new pcui::ComboBox(
        this,
        config.bladeArrays.dataPinProxy,
        _("Blade Data Pin"),
        wxHORIZONTAL
    )};
    dataPin->SetToolTip(_("The pin name or number used for WS281X data.\nSpecify custom pins by typing in this box."));
    auto *colorOrder3{new pcui::Choice(
        this,
        config.bladeArrays.colorOrder3Proxy,
        _("Color Order"),
        wxHORIZONTAL
    )};
    colorOrder3->SetToolTip(_("The order of colors for your blade.\nMost of the time this can be left as \"GRB\"."));
    auto *colorOrder4{new pcui::Choice(
        this,
        config.bladeArrays.colorOrder4Proxy,
        _("Color Order"),
        wxHORIZONTAL
    )};
    colorOrder4->SetToolTip(_("The order of colors for your blade.\nMost of the time this can be left as \"GRBW\"."));
    auto *hasWhite{new pcui::CheckBox(
        this,
        config.bladeArrays.hasWhiteProxy,
        0,
        _("LEDs Have White Channel")
    )};
    auto *whiteUseRGB{new pcui::CheckBox(
        this,
        config.bladeArrays.useRGBWithWhiteProxy,
        0,
        _("Use RGB with White")
    )};
    whiteUseRGB->SetToolTip(_("Use the RGB channels alongside the White channel to produce white light.\nThis can result in a brighter blade, but at the cost of higher battery usage and a less \"pure\" white."));

    auto *pixelBrightness{new pcui::Numeric(
        this,
        config.bladeArrays.pixelBrightnessProxy,
        _("Brightness"),
        wxHORIZONTAL
    )};

    auto *pixelPowerPins{new pcui::CheckList(
        this,
        config.bladeArrays.powerPinProxy,
        _("Power Pins")
    )};
    pixelPowerPins->SetMinSize(wxSize(-1, 200));
    pixelPowerPins->SetToolTip();
}
*/

