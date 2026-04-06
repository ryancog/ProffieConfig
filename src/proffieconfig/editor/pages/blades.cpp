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

#include "config/blades/bladeconfig.hpp"
#include "config/blades/ws281x.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/selector.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/parent.hpp"

BladesPage::BladesPage(config::Config& config) : mConfig{config} {
    mArraySel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& arraySel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&BladesPage::mArraySel>(arraySel)};

        if (page.mDlg) {
            pcui::cripple(page.mDlg);

            page.mDlg->CallAfter([dlg=page.mDlg] {
                dlg->Close(true);
            });

            page.mDlg = nullptr;
        }

        // Always detach first
        page.mIssueReceiver.detach();

        if (ctxt.choice() != -1) {
            using namespace config::blades;
            data::Vector::ROContext bladeConfigs{page.mConfig.bladeConfigs_};
            auto& selModel{*bladeConfigs.children()[ctxt.choice()]};
            auto& selected{static_cast<BladeConfig&>(selModel)};
            page.mIssueReceiver.attach(selected.issues());
        }
    };

    data::Selector::Context{mArraySel}.bind(&config.bladeConfigs_);
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
          .base_={.expand_=true},
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
                  .data_=mArraySel.choice_,
                  .style_=pcui::Choice::PopUp{
                    .unselected_=_("Select Array"),
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
                      if (mDlg) mDlg->Close(true);

                      data::Vector::Context vec{mConfig.bladeConfigs_};
                      auto& cfg{vec.addCreate<config::blades::BladeConfig>()};

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
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Group{
          .base_={.expand_=true, .proportion_=1},
          .label_=_("Blades"),
          .orient_=wxVERTICAL,
          .children_={
            pcui::Choice{
              .win_={.base_={.expand_=true, .proportion_=1}},
              .data_=mBladeSel.choice_,
              .style_=pcui::Choice::List{},
            }(),
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize()},
                    .enable_=mArraySel.choice_ | data::logic::HasSelection{},
                  },
                  .label_="+",
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize()},
                    .enable_=mBladeSel.choice_ | data::logic::HasSelection{},
                  },
                  .label_="-",
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
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
                .data_=blade.type().choice_
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
    return pcui::Label{.label_="TODO"}();
}

pcui::DescriptorPtr BladesPage::ws281x(config::blades::WS281X& ws281x) {
    return pcui::Label{.label_="TODO"}();
}

BladesPage::IssueReceiver::~IssueReceiver() {
    detach();
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
        label = "\u26D4\uFE0E"; // No entry sym
    } else if (issues & eIssue_Warnings) {
        label = "\u26A0"; // warn
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
    pixelPowerPins->SetToolTip(_("The power pins to use for this blade.\nWS281X blades can have as many as are desired (though 2 is generally enough for most blades)"));

    auto *pinNameSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *powerPinName{new pcui::Text(
        this,
        config.bladeArrays.powerPinNameEntry,
        wxTE_PROCESS_ENTER,
        false,
        _("Pin Name")
    )};
    auto *addPowerPin{new wxButton(
        this,
        ID_PinNameAdd,
        "+",
        wxDefaultPosition,
        SMALLBUTTONSIZE,
        wxBU_EXACTFIT
    )};
    pinNameSizer->Add(powerPinName, wxSizerFlags(1));
    pinNameSizer->Add(addPowerPin, wxSizerFlags().Bottom());

    pixelMainSizer->Add(length, wxSizerFlags().Expand());
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(dataPin);
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(colorOrder3, wxSizerFlags().Expand());
    pixelMainSizer->Add(colorOrder4, wxSizerFlags().Expand());
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(hasWhite, wxSizerFlags().Expand());
    pixelMainSizer->Add(
        whiteUseRGB,
        wxSizerFlags().Expand().Border(wxTOP, 5)
    );
    pixelMainSizer->AddSpacer(10);
    pixelMainSizer->Add(pixelBrightness, wxSizerFlags().Expand());
    pixelMainSizer->AddSpacer(10);
    pixelMainSizer->Add(pixelPowerPins, wxSizerFlags(1).Expand());
    pixelMainSizer->AddSpacer(5);
    pixelMainSizer->Add(pinNameSizer, wxSizerFlags().Expand());

    auto *pixelSplitSizer{new pcui::StaticBox(
        wxVERTICAL, this, _("SubBlades")
    )};
    auto *splitSelect{new pcui::Choice(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitSelectionProxy
    )};
    auto *splitButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addSplit{new wxButton(
        pixelSplitSizer->childParent(),
        ID_AddSplit,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *removeSplit{new wxButton(
        pixelSplitSizer->childParent(),
        ID_RemoveSplit,
        "-",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    splitButtonSizer->Add(addSplit, wxSizerFlags(1));
    splitButtonSizer->AddSpacer(5);
    splitButtonSizer->Add(removeSplit, wxSizerFlags(1));

    auto *splitType{new pcui::Radios(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitTypeProxy,
        { _("Standard"), _("Reverse"), _("Stride"), _("ZigZag"), _("List") },
        _("Type")
    )};
    splitType->SetToolTip(_(
        "Standard: Split data into continuous sections.\n\n"
        "Reverse: Identical to Standard, but reverses the bladestyle direction.\n\n"
        "Stride: Useful for KR style blades where the data signal \"strides\" back and forth across sides.\n\n"
        "ZigZag: Similar to Stride, but organizes data in the opposite manner perpendicular to the data signal.\n\n"
        "List: Discrete LEDs to make part of a SubBlade."
    ));
    
    auto *splitStartEndSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *splitStart{new pcui::Numeric(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitStartProxy,
        _("Start")
    )};
    auto *splitEnd{new pcui::Numeric(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitEndProxy,
        _("End")
    )};
    splitStartEndSizer->Add(splitStart, wxSizerFlags(1));
    splitStartEndSizer->AddSpacer(5);
    splitStartEndSizer->Add(splitEnd, wxSizerFlags(1));

    auto *splitLength{new pcui::Numeric(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitLengthProxy,
        _("Length")
    )};
    auto *splitSegments{new pcui::Numeric(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitSegmentsProxy,
        _("Segments")
    )};
    splitSegments->SetToolTip(_("Stride length or number of ZigZag columns"));
    auto *splitList{new pcui::Text(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitListProxy,
        0,
        false,
        _("Pixel List")
    )};
    splitList->SetToolTip(_("Data goes along each LED according to their order in the list"));
    auto *splitBrightness{new pcui::Numeric(
        pixelSplitSizer->childParent(),
        config.bladeArrays.splitBrightnessProxy,
        _("Brightness"),
        wxHORIZONTAL
    )};

    pixelSplitSizer->Add(splitSelect, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(5);
    pixelSplitSizer->Add(splitButtonSizer, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(10);
    pixelSplitSizer->Add(splitType, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(10);
    pixelSplitSizer->Add(
        splitStartEndSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5)
    );
    pixelSplitSizer->Add(splitLength, wxSizerFlags().Expand());
    pixelSplitSizer->Add(
        splitSegments, wxSizerFlags().Expand().Border(wxTOP, 10)
    );
    pixelSplitSizer->Add(splitList, wxSizerFlags().Expand());
    pixelSplitSizer->AddSpacer(10);
    pixelSplitSizer->Add(splitBrightness, wxSizerFlags().Expand());

    mPixelSizer->Add(pixelMainSizer, wxSizerFlags().Expand());
    mPixelSizer->AddSpacer(10);
    mPixelSizer->Add(
        new SplitVisualizer(this, config.bladeArrays),
        wxSizerFlags().Expand()
    );
    mPixelSizer->AddSpacer(10);
    mPixelSizer->Add(pixelSplitSizer, wxSizerFlags(1).Expand());

    setupSizer->Add(bladeType);
    setupSizer->AddSpacer(10);
    setupSizer->Add(mSimpleSizer, wxSizerFlags(1).Expand());
    setupSizer->Add(mPixelSizer, wxSizerFlags(1).Expand());

    settingsSizer->Add(noSelectText, wxSizerFlags().Center());
    settingsSizer->Add(setupSizer, wxSizerFlags(1).Expand());

    return settingsSizer;
}
*/

