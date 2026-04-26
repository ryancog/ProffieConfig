#include "presets.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/presets.cpp
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
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "data/choice.hpp"
#include "data/helpers/exclusive.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "data/selector.hpp"
#include "ui/bitmap.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/selector.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/split.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/parent.hpp"
#include "wx/event.h"

PresetsPage::PresetsPage(config::Config& config) : mConfig{config} {
    mArraySel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& arraySel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&PresetsPage::mArraySel>(arraySel)};

        if (page.mDlg) {
            pcui::cripple(page.mDlg);

            page.mDlg->CallAfter([dlg=page.mDlg] {
                dlg->Destroy();
            });
        }

        data::Selector::ROContext arraySelCtxt{arraySel};
        data::Selector::Context presetSel{page.mPresetSel};

        data::Vector *presets{nullptr};
        if (arraySelCtxt.choiceIdx() != -1) {
            using namespace config::presets;
            auto& curArray{*arraySelCtxt.selected<Array>()};
            presets = &curArray.presets_;
        }
        presetSel.bind(presets);
    };

    mPresetSel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& presetSel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&PresetsPage::mPresetSel>(presetSel)};

        data::Selector::ROContext presetSelCtxt{presetSel};
        data::Selector::Context styleSel{page.mStyleSel};

        data::Vector *styles{nullptr};
        using namespace config::presets;
        if (auto *curPreset{presetSelCtxt.selected<Preset>()}) {
            styles = &curPreset->styles_;
        }
        styleSel.bind(styles);
    };

    mDisplaySel.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& displaySel{*ctxt.model().parent<data::Selector>()};
        auto& page{utils::parent<&PresetsPage::mDisplaySel>(displaySel)};

        page.updateBladeStrings();
    };

    const auto displayFilter{[](
        const data::Choice::ROContext& ctxt, int32& idx
    ) {
        if (idx == -1 and ctxt.numChoices())
            idx = 0;
    }};
    mDisplaySel.choice_.setFilter(displayFilter);

    data::Selector::Context{mArraySel}.bind(&config.presetArrays_);
    data::Selector::Context{mDisplaySel}.bind(&config.bladeConfigs_);
}

void PresetsPage::deinit() {
    // This'll cascade to preset and style sels
    data::Selector::Context{mArraySel}.bind(nullptr);
    data::Selector::Context{mDisplaySel}.bind(nullptr);
}

pcui::DescriptorPtr PresetsPage::ui() {
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
        fields(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        displayAndBlade(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        style(),
      }
    }();
}

pcui::DescriptorPtr PresetsPage::selection() {
    return pcui::Stack{
      .base_={.expand_=true},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Group{
          .win_={.base_={.expand_=true}},
          .label_=_("Presets Array"),
          .orient_=wxVERTICAL,
          .children_={
            pcui::Stack{
              .base_={.expand_=true},
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Choice{
                  .win_={
                    .base_={
                      .minSize_={150, -1},
                      .proportion_=1,
                    },
                  },
                  .data_=mArraySel,
                  .style_=pcui::Choice::PopUp{
                    .unselected_=_("Select Array"),
                  },
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      data::Vector::ROContext vec{mConfig.presetArrays_};
                      using namespace config::presets;
                      auto& cfg{static_cast<Array&>(*vec.children()[idx])};
                      return cfg.name_;
                  },
                }(),
                pcui::Selector{
                  .data_=mArraySel,
                  .builder_=[](data::Model *model) -> pcui::DescriptorPtr {
                    if (not model) return pcui::Spacer{.size_=0}();

                    const auto& array{
                        *static_cast<config::presets::Array *>(model)
                    };

                    return pcui::Button{
                      .win_={
                        .base_={
                          .border_={
                            .size_=pcui::interControlSpacing(),
                            .dirs_=wxLEFT
                          },
                        },
                        .show_=not (array.issues() |
                          data::logic::Equals{.val_=0})
                      },
                      .label_=pcui::syms::NO_ENTRY,
                      .exactFit_=true,
                    }();
                  }
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={
                    .base_={
                      .minSize_=pcui::iconButtonSize(),
                      .expand_=true,
                    },
                    .enable_=mArraySel.hasSelection() | data::logic::IsSet{},
                  },
                  .bitmap_={
                    .src_=pcui::Bitmap("edit").color(wxSYS_COLOUR_WINDOWTEXT),
                  },
                  .exactFit_=true,
                  .func_=[this](const pcui::CallbackContext& ctxt) {
                      if (mDlg) {
                          mDlg->Show();
                          mDlg->Raise();
                          return;
                      }

                      using namespace config::presets;

                      data::Selector::Context sel{mArraySel};
                      auto& cfg{static_cast<Array&>(*sel.selected())};
                      mDlg = new PresetArrayDlg(ctxt.topLevel_, cfg, false);
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
                  .func_=[this](const pcui::CallbackContext& ctxt) {
                      // Only ever allow one of these dialogs. Not a technical
                      // limitation, just don't want things cluttered.
                      if (mDlg) mDlg->Destroy();

                      data::Vector::Context vec{mConfig.presetArrays_};
                      auto& cfg{vec.addCreate<config::presets::Array>()};

                      PresetArrayDlg dlg(ctxt.topLevel_, cfg, true);

                      auto res{dlg.ShowModal()};

                      if (res != wxID_OK) {
                          // Make sure the dialog is crippled before removing
                          // the model it's linked to.
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
                    .enable_=mArraySel.hasSelection() | data::logic::IsSet{},
                  },
                  .label_=_("Remove"),
                  .func_=[this] {
                      data::Selector::ROContext sel{mArraySel};
                      data::Vector::Context vec{mConfig.presetArrays_};

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
          .label_=_("Presets"),
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Stack{
              .base_={.expand_=true, .proportion_=1},
              .orient_=wxVERTICAL,
              .children_={
                pcui::Choice{
                  .win_={
                    .base_={
                      .minSize_={-1, 300},
                      .expand_=true,
                      .proportion_=1,
                    },
                    .tooltip_=_("The currently-selected preset array to be edited.\nEach preset array has unique presets."),
                  },
                  .data_=mPresetSel,
                  .style_=pcui::Choice::List{},
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      data::Selector::ROContext sel{mPresetSel};
                      data::Vector::ROContext vec{*sel.bound()};
                      using namespace config::presets;
                      auto& cfg{static_cast<Preset&>(*vec.children()[idx])};
                      return cfg.name_;
                  },
                }(),
                pcui::Stack{
                  .orient_=wxHORIZONTAL,
                  .children_={
                    pcui::Button{
                      .win_={
                        .base_={.minSize_=pcui::iconButtonSize()},
                        .enable_=mArraySel.hasSelection() |
                            data::logic::IsSet{},
                      },
                      .label_=pcui::syms::PLUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=[this] {
                          data::Selector::ROContext sel{mPresetSel};
                          data::Vector::Context vec{
                              const_cast<data::Vector&>(*sel.bound())
                          };
                          vec.addCreate<config::presets::Preset>();
                      },
                    }(),
                    pcui::Button{
                      .win_={
                        .base_={.minSize_=pcui::iconButtonSize()},
                        .enable_=mPresetSel.hasSelection() |
                            data::logic::IsSet{},
                      },
                      .label_=pcui::syms::MINUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=[this] {
                          data::Selector::ROContext sel{mPresetSel};
                          data::Vector::Context vec{
                              const_cast<data::Vector&>(*sel.bound())
                          };
                          vec.remove(*sel.selected());
                      },
                    }(),
                  }
                }()
              }
            }(),
            pcui::Stack{
              .orient_=wxVERTICAL,
              .children_={
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mPresetSel.canMoveUp() | data::logic::IsSet{},
                  },
                  .label_=pcui::syms::UP_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] {
                      data::Selector::ROContext sel{mPresetSel};
                      data::Vector::Context vec{
                          const_cast<data::Vector&>(*sel.bound())
                      };
                      vec.moveUp(sel.choiceIdx());
                  }
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mPresetSel.canMoveDown() | data::logic::IsSet{},
                  },
                  .label_=pcui::syms::DOWN_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] {
                      data::Selector::ROContext sel{mPresetSel};
                      data::Vector::Context vec{
                          const_cast<data::Vector&>(*sel.bound())
                      };
                      vec.moveDown(sel.choiceIdx());
                  }
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mPresetSel.hasSelection() | data::logic::IsSet{},
                  },
                  .label_=pcui::syms::DOUBLE_SQUARES,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] {
                      data::Selector::ROContext sel{mPresetSel};
                      data::Choice::Context choice{mPresetSel.choice_};
                      data::Vector::Context vec{
                          const_cast<data::Vector&>(*sel.bound())
                      };
                      vec.duplicate(
                          choice.idx(),
                          data::Vector::DuplicationMode::Insert
                      );
                      choice.choose(choice.idx() + 1);
                  }
                }(),
              }
            }(),
          }
        }(),
      }
    }();
}

pcui::DescriptorPtr PresetsPage::fields() {
    return pcui::Stack{
      .base_={
        .minSize_={200, -1},
        .expand_=true,
      },
      .orient_=wxVERTICAL,
      .children_{
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
          },
          .label_=_("Preset Name"),
          .ctrl_=pcui::Selector{
            .data_=mPresetSel,
            .builder_=[](data::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true},
                  .tooltip_=_(
                      "The name for the preset.\n"
                      "This appears on the OLED screen if no bitmap is supplied, otherwise it's just for reference.\n"
                      "\"\\n\" means \"enter.\" Hitting \"enter\" will insert \"\\n\" which means a new line in the text displayed on the OLED.\n"
                      "For example, \"my\\npreset\" will be displayed on the OLED as two lines, the first being \"my\" and the second being \"preset.\""
                  )
                },
                .style_=pcui::Text::SingleLine{
                  .onEnter_=pcui::Text::InsertLiteral{},
                },
              };

              if (model == nullptr) return text();

              auto *preset{static_cast<config::presets::Preset *>(model)};
              text.data_ = preset->name_;
              return text();
            }
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_(
                "The path of the folder on the SD card where the font is stored.\n"
                "If the font folder is inside another folder, it must be indicated by something like \"folderName/fontFolderName\".\n"
                "In order to specify multiple directories (for example, to include a \"common\" directory), use a semicolon (;) to separate the folders (e.g. \"fontFolderName;common\")."
            )
          },
          .label_=_("Font Directory"),
          .ctrl_=pcui::Selector{
            .data_=mPresetSel,
            .builder_=[](data::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true},
                },
              };

              if (model == nullptr) return text();

              auto *preset{static_cast<config::presets::Preset *>(model)};
              text.data_ = preset->fontDir_;
              return text();
            }
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Labeled{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_(
                "The path of the track file on the SD card. May be empty.\n"
                "If the track is directly inside one of the folders specified in \"Font Directory\" then only the name of the track file is required."
            ),
          },
          .label_=_("Track File"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Selector{
            .data_=mPresetSel,
            .builder_=[](data::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true},
                },
              };

              if (model == nullptr) return text();

              auto *preset{static_cast<config::presets::Preset *>(model)};
              text.data_ = preset->track_;
              return text();
            }
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr PresetsPage::displayAndBlade() {
    return pcui::Stack{
      .base_={.expand_=true},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Label{
          .label_=_("Display"),
        }(),
        pcui::Choice{
          .win_={
            .base_={.expand_=true},
            .tooltip_=_("Show blade listing corresponding to the selected blade array."),
          },
          .data_=mDisplaySel,
          .style_=pcui::Choice::PopUp{
            .unselected_=_("Select Blade Array"),
          },
          .emptyLabel_=_("[default]"),
          .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
              data::Selector::ROContext displaySel{mDisplaySel};
              data::Vector::ROContext vec{*displaySel.bound()};
              auto& array{static_cast<config::blades::BladeConfig&>(
                  *vec.children()[idx]
              )};
              return array.name_;
          },
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=_("Styles"),
        }(),
        pcui::Choice{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .data_=mStyleSel,
          .clamp_=mConfig.numBlades(),
          .style_=pcui::Choice::List{},
          .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
              if (idx == 0) {
                  data::Integer::ROContext numBlades{mConfig.numBlades()};
                  mBladeStrings.resize(numBlades.val());
                  updateBladeStrings();
              }

              return mBladeStrings[idx];
          },
        }(),
      }
    }();
}

pcui::DescriptorPtr PresetsPage::style() {
    return pcui::Split{
      .win_={.base_={
        .minSize_={500, -1},
        .expand_=true,
        .proportion_=1,
      }},
      .orient_=wxVERTICAL,
      .minPaneSize_=60,
      .child1_=pcui::Stack{
        .base_={.expand_=true, .proportion_=1},
        .children_={
          pcui::Label{
            .label_=_("Comments"),
          }(),
          pcui::Selector{
            .data_=mStyleSel,
            .builder_=[](data::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true, .proportion_=1},
                  .tooltip_=_(
                      "Any comments about the blade style goes here.\n"
                      "This doesn't affect the blade style at all, but can be a place for helpful notes!"
                  ),
                }, 
                .style_=pcui::Text::MultiLine{},
              };

              if (model == nullptr) {
                  text.win_.enable_ = false;
                  text.data_ = _("Select or create preset and blade to edit style comments...");
                  return text();
              }

              auto *style{static_cast<config::presets::Style *>(model)};
              text.data_ = style->comment_;
              return text();
            }
          }(),
        }
      }(),
      .child2_=pcui::Stack{
        .base_={.expand_=true, .proportion_=1},
        .children_={
          pcui::Spacer{.size_=2}(),
          pcui::Label{
            .label_=_("Blade Style"),
          }(),
          pcui::Selector{
            .data_=mStyleSel,
            .builder_=[](data::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true, .proportion_=1},
                  .tooltip_=_(
                      "Your blade style goes here.\n"
                      "This is the code which sets up what animations and effects your blade (or other LED) will do.\n"
                      "For getting/creating blade styles, see the Documentation (in \"Help->Documentation...\")."
                  )
                }, 
                .style_=pcui::Text::MultiLine{
                  .wrap_=pcui::Text::MultiLine::Wrap::None,
                },
              };

              if (model == nullptr) {
                  text.win_.enable_ = false;
                  text.data_ = _("Select or create preset and blade to edit style...");
                  return text();
              }

              auto *style{static_cast<config::presets::Style *>(model)};
              text.data_ = style->content_;
              return text();
            }
          }(),
        }
      }(),
    }();
}

// This could probably stand to be cleaned up. The nesting and repetitive
// checks are a bit much.
void PresetsPage::updateBladeStrings() {
    using namespace config::blades;

    data::Selector::ROContext displaySel{mDisplaySel};
    
    size count{0};

    if (auto *bladeCfg{displaySel.selected<BladeConfig>()}) {
        data::Vector::ROContext bladeVec{bladeCfg->blades_};
        size labelIdx{0};
        size mainIdx{0};
        for (const auto& child : bladeVec.children()) {
            auto& blade{static_cast<Blade&>(*child)};
            data::Choice::ROContext type{blade.type().choice_};

            if (type.idx() == Blade::eSimple) {
                if (count == mBladeStrings.size()) return;

                data::String::Context label{mBladeStrings[count]};
                label.change(wxString::Format(
                    _("Blade %d"), mainIdx
                ).ToStdString());

                ++mainIdx;
                ++count;
            } else if (type.idx() == Blade::eWS281X) {
                auto& ws281x{blade.ws281x()};

                data::Vector::ROContext splits{ws281x.splits_};

                if (splits.children().empty()) {
                    if (count == mBladeStrings.size()) return;

                    data::String::Context label{mBladeStrings[count]};
                    label.change(wxString::Format(
                        _("Blade %d"), mainIdx
                    ).ToStdString());

                    ++count;
                } else {
                    size subIdx{0};
                    for (const auto& child : splits.children()) {
                        auto& split{static_cast<WS281X::Split&>(*child)};

                        const auto splitType{static_cast<WS281X::Split::Type>(
                            split.type_.selected()
                        )};

                        switch (splitType) {
                            using enum WS281X::Split::Type;
                            case eReverse:
                            case eStandard:
                            case eList:
                            {
                                if (count == mBladeStrings.size()) return;

                                data::String::Context label{
                                    mBladeStrings[count]
                                };
                                label.change(wxString::Format(
                                    _("Blade %d:%d"), mainIdx, subIdx
                                ).ToStdString());

                                ++count;
                                break;
                            }
                            case eStride:
                            case eZig_Zag:
                            {
                                data::Integer::ROContext numSegments{
                                    split.segments_
                                };
                                for (
                                        size idx{0};
                                        idx < numSegments.val();
                                        ++idx
                                    ) {
                                    if (count == mBladeStrings.size()) return;

                                    data::String::Context label{
                                        mBladeStrings[count]
                                    };
                                    label.change(wxString::Format(
                                        _("Blade %d:%d:%d"),
                                        mainIdx,
                                        subIdx,
                                        idx
                                    ).ToStdString());

                                    ++count;
                                }
                                break;
                            }
                            case eMax:
                                __builtin_unreachable();
                        }
                        ++subIdx;
                    }
                }

                ++mainIdx;
            } else if (type.idx() == Blade::eUnassigned) {
                if (count == mBladeStrings.size()) return;

                data::String::Context label{mBladeStrings[count]};
                label.change(_("Unassigned").ToStdString());

                ++count;
            }
        }
    }

    for (auto idx{count}; idx < mBladeStrings.size(); ++idx) {
        data::String::Context label{mBladeStrings[idx]};
        label.change(_("Unassigned").ToStdString());
    }
}

/*
void PresetsPage::rebuildInjections() {
    mInjectionsSizer->Clear(true);

    auto& config{mParent->getOpenConfig()};

    for (const auto& injection : config.presetArrays.injections()) {
        auto *injectionSizer{new wxBoxSizer(wxHORIZONTAL)};
        auto *injectionText{new wxStaticText(
            mInjectionsSizer->childParent(),
            wxID_ANY,
            injection->filename
        )};
        auto *editButton{new wxButton(
            mInjectionsSizer->childParent(),
            wxID_ANY,
            _("Edit"),
            wxDefaultPosition,
            wxDefaultSize,
            wxBU_EXACTFIT
        )};
        auto *deleteButton{new wxButton(
            mInjectionsSizer->childParent(),
            wxID_ANY,
            _("Delete"),
            wxDefaultPosition,
            wxDefaultSize,
            wxBU_EXACTFIT
        )};

        editButton->Bind(wxEVT_BUTTON, [&injection](wxCommandEvent&) {
            wxLaunchDefaultApplication(
                (paths::injectionDir() / injection->filename).native()
            );
        });

        deleteButton->Bind(wxEVT_BUTTON, [this, &injection](wxCommandEvent&) {
            auto res{pcui::showMessage(
                _("This action cannot be undone!"),
                _("Delete Injection"),
                wxYES_NO | wxNO_DEFAULT
            )};
            if (wxNO == res) return;

            mParent->getOpenConfig().presetArrays.removeInjection(*injection);
        });

        injectionSizer->Add(injectionText, wxSizerFlags(1).Center());
        injectionSizer->AddSpacer(20);
        injectionSizer->Add(editButton);
        injectionSizer->AddSpacer(10);
        injectionSizer->Add(deleteButton);

        mInjectionsSizer->Add(injectionSizer, wxSizerFlags().Expand());
    }

    mInjectionsSizer->Show(not mInjectionsSizer->IsEmpty());
    mInjectionsSizer->Layout();
    Layout();
}
*/

