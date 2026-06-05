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

#include <wx/gdicmn.h>

#include "config/blades/bladeconfig.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/bitmap.hpp"
#include "ui/build.hpp"
#include "ui/builders/selector.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/split.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

#include "../../core/state.hpp"

PresetsPage::PresetsPage(config::Config& config) : mConfig{config} {
    static const auto arrayTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&PresetsPage::onArrayChoice>();
        return table;
    }()};
    observeWith(mArraySel.choice(), arrayTable);

    static const auto presetTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&PresetsPage::onPresetChoice>();
        return table;
    }()};
    observeWith(mPresetSel.choice(), presetTable);

    static const auto displayTable{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&PresetsPage::onDisplayChoice>();
        return table;
    }()};
    observeWith(mDisplaySel.choice(), displayTable);

    const auto arrayFilter{[](
        const data::base::Choice::ROContext& ctxt, int32& idx
    ) {
        if (idx == -1 and ctxt.num())
            idx = 0;
    }};
    mArraySel.choice().setFilter(arrayFilter);

    const auto displayFilter{[](
        const data::base::Choice::ROContext& ctxt, int32& idx
    ) {
        if (idx == -1 and ctxt.num())
            idx = 0;
    }};
    mDisplaySel.choice().setFilter(displayFilter);

    activate();
}

void PresetsPage::deinit() {
    if (mStylesDlg) {
        mStylesDlg->deinit();
        mStylesDlg->CallAfter([dlg=mStylesDlg] {
            dlg->Destroy();
        });
    }

    deactivate();
}

void PresetsPage::onActivate() {
    mArraySel.bind(&mConfig.presetArrays_);
    mDisplaySel.bind(&mConfig.bladeConfigs_);
}

void PresetsPage::preDeactivate() {
    // This'll cascade to preset and style sels
    mArraySel.bind(nullptr);
    mDisplaySel.bind(nullptr);
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
        pcui::Button{
          .win_={.base_={.expand_=true}},
          .label_=_("Style Aliases"),
          .func_=[this](const pcui::CallbackContext& ctxt) {
              onStylesButton(ctxt);
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Group{
          .win_={.base_={.expand_=true}},
          .label_=_("Presets Array"),
          .orient_=wxVERTICAL,
          .children_={
            pcui::Stack{
              .base_={.expand_=true},
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::builders::Selector{
                  .data_=mArraySel,
                  .builder_=[](
                      data::base::Model *model
                  ) -> pcui::DescriptorPtr {
                      if (not model) return pcui::Spacer{.size_=0}();

                      const auto& array{
                          *dynamic_cast<config::presets::Array *>(model)
                      };

                      return pcui::Label{
                        .win_={
                          .base_={
                            .border_={
                              .size_=pcui::interControlSpacing(),
                              .dirs_=wxRIGHT
                            },
                          },
                          .show_=not (array.issues() |
                              data::logic::Equals{.val_=0}),
                          .tooltip_=_("There's issues with this array, open the edit dialog to see them."),
                        },
                        .label_=pcui::syms::NO_ENTRY,
                      }();
                  }
                }(),
                pcui::Choice{
                  .win_={
                    .base_={
                      .minSize_={150, -1},
                      .proportion_=1,
                    },
                    .tooltip_=_("The currently-selected preset array to be edited.\nEach preset array has unique presets."),
                  },
                  .data_=mArraySel,
                  .style_=pcui::Choice::PopUp{
                    .unselected_=_("Select Array"),
                  },
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      auto vec{data::context(mConfig.presetArrays_)};
                      using namespace config::presets;
                      auto& cfg{dynamic_cast<Array&>(*vec.children()[idx])};
                      return cfg.name_;
                  },
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={
                    .base_={
                      .minSize_=pcui::iconButtonSize(),
                      .expand_=true,
                    },
                    .enable_=mArraySel.choice() | data::logic::HasSelection{},
                  },
                  .bitmap_={
                    .src_=pcui::Bitmap("edit").color(wxSYS_COLOUR_WINDOWTEXT),
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
                  .func_=[this] { onRemoveButton(); },
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
                    .tooltip_=_("The preset to edit."),
                  },
                  .data_=mPresetSel,
                  .style_=pcui::Choice::List{},
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      auto sel{data::context(mPresetSel)};
                      auto vec{data::context(*sel.bound())};
                      using namespace config::presets;
                      auto& cfg{dynamic_cast<Preset&>(*vec.children()[idx])};
                      return cfg.name_;
                  },
                }(),
                pcui::Stack{
                  .orient_=wxHORIZONTAL,
                  .children_={
                    pcui::Button{
                      .win_={
                        .base_={.minSize_=pcui::iconButtonSize()},
                        .enable_=mArraySel.choice() |
                            data::logic::HasSelection{},
                      },
                      .label_=pcui::syms::PLUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=[this] { onAddPresetButton(); },
                    }(),
                    pcui::Button{
                      .win_={
                        .base_={.minSize_=pcui::iconButtonSize()},
                        .enable_=mPresetSel.choice() |
                            data::logic::HasSelection{},
                      },
                      .label_=pcui::syms::MINUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=[this] { onRemovePresetButton(); },
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
                    .enable_=mPresetSel | data::logic::CanMoveUp{},
                  },
                  .label_=pcui::syms::UP_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onMoveUpButton(); },
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mPresetSel | data::logic::CanMoveDown{},
                  },
                  .label_=pcui::syms::DOWN_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onMoveDownButton(); },
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mPresetSel.choice() | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::DOUBLE_SQUARES,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onDuplicateButton(); },
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
          .ctrl_=pcui::builders::Selector{
            .data_=mPresetSel,
            .builder_=[](data::base::Model *model) {
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

              auto *preset{dynamic_cast<config::presets::Preset *>(model)};
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
          .ctrl_=pcui::builders::Selector{
            .data_=mPresetSel,
            .builder_=[](data::base::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true},
                },
              };

              if (model == nullptr) return text();

              auto *preset{dynamic_cast<config::presets::Preset *>(model)};
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
          .ctrl_=pcui::builders::Selector{
            .data_=mPresetSel,
            .builder_=[](data::base::Model *model) {
              pcui::Text text{
                .win_={
                  .base_={.expand_=true},
                },
              };

              if (model == nullptr) return text();

              auto *preset{dynamic_cast<config::presets::Preset *>(model)};
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
              auto displaySel{data::context(mDisplaySel)};
              auto vec{data::context(*displaySel.bound())};
              auto& array{dynamic_cast<config::blades::BladeConfig&>(
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
                  auto numBlades{data::context(mConfig.numBlades())};

                  while (mBladeStrings.size() < numBlades.val()) {
                      mBladeStrings.push_back(
                          std::make_unique<data::prim::String>()
                      );
                  }

                  updateBladeStrings();
              }

              return *mBladeStrings[idx];
          },
        }(),
      }
    }();
}

pcui::DescriptorPtr PresetsPage::style() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Split{
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
              pcui::builders::Selector{
                .data_=mStyleSel,
                .builder_=[](data::base::Model *model) {
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

                  auto *style{dynamic_cast<config::presets::Style *>(model)};
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
              pcui::builders::Selector{
                .data_=mStyleSel,
                .builder_=[](data::base::Model *model) {
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
                      .wrap_=pcui::Text::Wrap::None,
                    },
                  };

                  if (model == nullptr) {
                      text.win_.enable_ = false;
                      text.data_ = _("Select or create preset and blade to edit style...");
                      return text();
                  }

                  auto *style{dynamic_cast<config::presets::Style *>(model)};
                  text.data_ = style->content_;
                  return text();
                }
              }(),
            }
          }(),
        }(),
        pcui::Stack{
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Button{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize(true)},
                .enable_=mStyleSel.choice() | data::logic::HasSelection{},
                .tooltip_=_("Format style code"),
              },
              .label_=pcui::syms::INDENT_ARROWS,
              .style_=pcui::Button::Style::Companion,
              .exactFit_=true,
              .func_=[this] { onFormatButton(); },
            }(),
          }
        }(),
      },
    }();
}

// This could probably stand to be cleaned up. The nesting and repetitive
// checks are a bit much.
void PresetsPage::updateBladeStrings() {
    using namespace config::blades;

    auto displaySel{data::context(mDisplaySel)};
    
    size count{0};

    if (auto *bladeCfg{displaySel.selected<BladeConfig>()}) {
        auto bladeVec{data::context(bladeCfg->blades_)};
        size labelIdx{0};
        size mainIdx{0};
        for (const auto& child : bladeVec.children()) {
            auto& blade{dynamic_cast<Blade&>(*child)};
            auto type{data::context(blade.type().choice())};

            if (type.idx() == Blade::eSimple or type.idx() == Blade::eServo) {
                if (count == mBladeStrings.size()) return;

                auto label{data::context(*mBladeStrings[count])};
                label.change(wxString::Format(
                    _("Blade %d"), mainIdx
                ).utf8_string());

                ++count;
            } else if (type.idx() == Blade::eWS281X) {
                auto& ws281x{blade.ws281x()};

                auto splits{data::context(ws281x.splits_)};

                if (splits.children().empty()) {
                    if (count == mBladeStrings.size()) return;

                    auto label{data::context(*mBladeStrings[count])};
                    label.change(wxString::Format(
                        _("Blade %d"), mainIdx
                    ).utf8_string());

                    ++count;
                } else {
                    size subIdx{0};
                    for (const auto& child : splits.children()) {
                        auto& split{dynamic_cast<WS281X::Split&>(*child)};

                        auto type{data::context(split.type_)};
                        auto selType{static_cast<WS281X::Split::Type>(
                            type.selected()
                        )};

                        switch (selType) {
                            using enum WS281X::Split::Type;
                            case eReverse:
                            case eStandard:
                            case eList:
                            {
                                if (count == mBladeStrings.size()) return;

                                mBladeStrings[count]->change(wxString::Format(
                                    _("Blade %d:%d"), mainIdx, subIdx
                                ).utf8_string());

                                ++count;
                                break;
                            }
                            case eStride:
                            case eZig_Zag:
                            {
                                auto segments{data::context(split.segments_)};
                                for (
                                        size idx{0};
                                        idx < segments.val();
                                        ++idx
                                    ) {
                                    if (count == mBladeStrings.size()) return;

                                    auto& label{*mBladeStrings[count]};
                                    label.change(wxString::Format(
                                        _("Blade %d:%d:%d"),
                                        mainIdx,
                                        subIdx,
                                        idx
                                    ).utf8_string());

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
            } else if (type.idx() == Blade::eUnassigned) {
                if (count == mBladeStrings.size()) return;

                mBladeStrings[count]->change(_("Unassigned").utf8_string());

                ++count;
            } else assert(0);

            ++mainIdx;
        }
    }

    for (auto idx{count}; idx < mBladeStrings.size(); ++idx) {
        mBladeStrings[idx]->change(_("Unassigned").utf8_string());
    }
}

void PresetsPage::onEditButton(const pcui::CallbackContext& ctxt) {
    if (mArrayDlg) {
        mArrayDlg->Show();
        mArrayDlg->Raise();
        return;
    }

    using namespace config::presets;

    auto sel{data::context(mArraySel)};
    auto& cfg{dynamic_cast<Array&>(*sel.selected())};
    mArrayDlg = new PresetArrayDlg(ctxt.topLevel_, cfg, false);
    const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
        if (evt.GetEventObject() == mArrayDlg) mArrayDlg = nullptr;
    }};
    mArrayDlg->Bind(wxEVT_DESTROY, onDestroy);

    mArrayDlg->Show();
}

void PresetsPage::onAddButton(const pcui::CallbackContext& ctxt) {
    // Only ever allow one of these dialogs. Not a technical
    // limitation, just don't want things cluttered.
    if (mArrayDlg) mArrayDlg->Destroy();

    auto vec{data::context(mConfig.presetArrays_)};
    auto& cfg{vec.append<config::presets::Array>(mConfig)};

    PresetArrayDlg dlg(ctxt.topLevel_, cfg, true);

    auto res{dlg.ShowModal()};

    if (res != wxID_OK) {
        // Make sure the dialog is crippled before removing
        // the model it's linked to.
        pcui::cripple(&dlg);
        vec.remove(vec.children().size() - 1);
    } else {
        mArraySel.choice().choose(
            static_cast<int32>(vec.children().size() - 1)
        );
    }
}

void PresetsPage::onRemoveButton() {
    auto sel{data::context(mArraySel)};
    auto vec{data::context(mConfig.presetArrays_)};

    vec.remove(*sel.selected());
}

void PresetsPage::onAddPresetButton() {
    auto sel{data::context(mPresetSel)};
    auto vec{data::context(const_cast<data::base::Vector&>(*sel.bound()))};

    size insertPos{};
    auto pref{state::prefs::get<state::prefs::Enum::Add_Preset_Insertion>()};
    switch (pref) {
        using enum state::prefs::enums::AddPresetInsertion;
        case Before_Selected:
            if (sel.choiceIdx() != -1) {
                insertPos = sel.choiceIdx();
                break;
            }
            [[fallthrough]];
        case Begin:
            insertPos = 0;
            break;
        case After_Selected:
            if (sel.choiceIdx() != -1) {
                insertPos = sel.choiceIdx() + 1;
                break;
            }
            [[fallthrough]];
        case End:
            insertPos = vec.children().size();
            break;
        case Max:
            assert(0);
            __builtin_unreachable();
    }

    vec.insert(insertPos, std::make_unique<config::presets::Preset>(mConfig));
    mPresetSel.choice().choose(static_cast<int32>(insertPos));
}

void PresetsPage::onRemovePresetButton() {
    auto sel{data::context(mPresetSel)};
    auto choice{data::context(mPresetSel.choice())};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};

    auto lastIdx{choice.idx()};

    vec.remove(choice.idx());

    // Try to keep selection around the area of the last one, especially for
    // if user wants to delete several items.
    if (lastIdx < choice.num())
        choice.choose(lastIdx);
}

void PresetsPage::onMoveUpButton() {
    auto sel{data::context(mPresetSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.moveUp(sel.choiceIdx());
}

void PresetsPage::onMoveDownButton() {
    auto sel{data::context(mPresetSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.moveDown(sel.choiceIdx());
}

void PresetsPage::onDuplicateButton() {
    auto sel{data::context(mPresetSel)};
    auto vec{data::context(const_cast<data::base::Vector&>(*sel.bound()))};
    auto *source{sel.selected<config::presets::Preset>()};

    vec.insert(
        sel.choiceIdx() + 1,
        std::make_unique<config::presets::Preset>(
            *source, mConfig
        )
    );
    mPresetSel.choice().choose(sel.choiceIdx() + 1);
}

void PresetsPage::onFormatButton() {
    auto sel{data::context(mStyleSel)};
    auto *style{sel.selected<config::presets::Style>()};
    style->content_.change(style->format());
}

void PresetsPage::onStylesButton(const pcui::CallbackContext& ctxt) {
    if (mStylesDlg) {
        mStylesDlg->Show();
        mStylesDlg->Raise();
        return;
    }

    mStylesDlg = new StyleAliasesDlg(ctxt.topLevel_, mConfig);
    const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
        if (evt.GetEventObject() == mStylesDlg)
            mStylesDlg = nullptr;
    }};
    mStylesDlg->Bind(wxEVT_DESTROY, onDestroy);

    mStylesDlg->Show();
}

void PresetsPage::onArrayChoice() {
    if (mArrayDlg) {
        pcui::cripple(mArrayDlg);

        mArrayDlg->CallAfter([dlg=mArrayDlg] {
            dlg->Destroy();
        });
    }

    auto arraySel{data::context(mArraySel)};
    auto presetSel{data::context(mPresetSel)};

    data::base::Vector *presets{nullptr};
    if (arraySel.choiceIdx() != -1) {
        using namespace config::presets;
        auto& curArray{*arraySel.selected<Array>()};
        presets = &curArray.presets_;
    }
    presetSel.bind(presets);
}

void PresetsPage::onPresetChoice() {
    auto presetSel{data::context(mPresetSel)};
    auto styleSel{data::context(mStyleSel)};

    data::base::Vector *styles{nullptr};
    using namespace config::presets;
    if (auto *curPreset{presetSel.selected<Preset>()}) {
        styles = &curPreset->styles_;
    }
    styleSel.bind(styles);
}

void PresetsPage::onDisplayChoice() {
    updateBladeStrings();
}

