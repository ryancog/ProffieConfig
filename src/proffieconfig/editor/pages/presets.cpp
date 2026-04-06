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
#include <wx/msgdlg.h>

#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/bitmap.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/text.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/selector.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/split.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

PresetsPage::PresetsPage(config::Config& config) : mConfig{config} {}

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
          .base_={.expand_=true},
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
                  .data_=mArraySel.choice_,
                  .style_=pcui::Choice::PopUp{
                    .unselected_=_("Select Array"),
                  }
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
                  },
                  .bitmap_={
                    .src_=pcui::Bitmap("edit").color(wxSYS_COLOUR_WINDOWTEXT),
                  },
                  .exactFit_=true,
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
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .win_={.base_={.proportion_=1}},
                  .label_=_("Remove"),
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Group{
          .base_={.expand_=true, .proportion_=1},
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
                  .data_=mPresetSel.choice_,
                  .style_=pcui::Choice::List{},
                }(),
                pcui::Stack{
                  .orient_=wxHORIZONTAL,
                  .children_={
                    pcui::Button{
                      .win_={.base_={.minSize_=pcui::iconButtonSize()}},
                      .label_=pcui::syms::PLUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                    }(),
                    pcui::Button{
                      .win_={.base_={.minSize_=pcui::iconButtonSize()}},
                      .label_=pcui::syms::MINUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                    }(),
                  }
                }()
              }
            }(),
            pcui::Stack{
              .orient_=wxVERTICAL,
              .children_={
                pcui::Button{
                  .win_={.base_={.minSize_=pcui::iconButtonSize(true)}},
                  .label_=pcui::syms::UP_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                }(),
                pcui::Button{
                  .win_={.base_={.minSize_=pcui::iconButtonSize(true)}},
                  .label_=pcui::syms::DOWN_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                }(),
                pcui::Button{
                  .win_={.base_={.minSize_=pcui::iconButtonSize(true)}},
                  .label_=pcui::syms::DOUBLE_SQUARES,
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

pcui::DescriptorPtr PresetsPage::fields() {
    return pcui::Stack{
      .base_={
        .minSize_={200, -1},
        .expand_=true,
      },
      .orient_=wxVERTICAL,
      .children_{
        pcui::Label{
          .label_=_("Preset Name"),
        }(),
        pcui::Selector{
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
              .mode_=pcui::Text::SingleLine{
                  .insertNewline_=true,
              },
            };

            if (model == nullptr) return text();

            auto *preset{static_cast<config::presets::Preset *>(model)};
            text.data_ = preset->name_;
            return text();
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=_("Font Directory"),
        }(),
        pcui::Selector{
          .data_=mPresetSel,
          .builder_=[](data::Model *model) {
            pcui::Text text{
              .win_={
                .base_={.expand_=true},
                .tooltip_=_(
                    "The path of the folder on the SD card where the font is stored.\n"
                    "If the font folder is inside another folder, it must be indicated by something like \"folderName/fontFolderName\".\n"
                    "In order to specify multiple directories (for example, to include a \"common\" directory), use a semicolon (;) to separate the folders (e.g. \"fontFolderName;common\")."
                )
              },
            };

            if (model == nullptr) return text();

            auto *preset{static_cast<config::presets::Preset *>(model)};
            text.data_ = preset->fontDir_;
            return text();
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=_("Track File"),
        }(),
        pcui::Selector{
          .data_=mPresetSel,
          .builder_=[](data::Model *model) {
            pcui::Text text{
              .win_={
                .base_={.expand_=true},
                .tooltip_=_(
                    "The path of the track file on the SD card. May be empty.\n"
                    "If the track is directly inside one of the folders specified in \"Font Directory\" then only the name of the track file is required."
                )
              },
            };

            if (model == nullptr) return text();

            auto *preset{static_cast<config::presets::Preset *>(model)};
            text.data_ = preset->track_;
            return text();
          }
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
          .data_=mDisplaySel.choice_,
          .style_=pcui::Choice::PopUp{
            .unselected_=_("Select Blade Array"),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=_("Blades"),
        }(),
        pcui::Choice{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .data_=mBladeSel.choice_,
          .style_=pcui::Choice::List{},
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
                .mode_=pcui::Text::MultiLine{},
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
                .mode_=pcui::Text::MultiLine{
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

/*
void PresetsPage::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        auto& presetArray{
            config.presetArrays.array(config.presetArrays.selection)
        };
        presetArray.addPreset();
        const auto lastIdx{
            static_cast<int32>(presetArray.presets().size() - 1)
        };
        presetArray.selection = lastIdx;
    }, eID_Add_Preset);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.removePreset(presetArray.selection);
    }, eID_Remove_Preset);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.movePresetUp(presetArray.selection);
    }, eID_Move_Preset_Up);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.movePresetDown(presetArray.selection);
    }, eID_Move_Preset_Down);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.duplicatePreset(presetArray.selection);
    }, eID_Duplicate_Preset);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        auto& presetArray{config.presetArrays.array(
            config.presetArrays.selection
        )};

        RenameArrayDlg renameDlg(
            mParent,
            config,
            presetArray,
            _("Rename Preset Array")
        );
        renameDlg.ShowModal();
    }, eID_Rename_Array);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};

        Config::PresetArray array(config);
        RenameArrayDlg dlg(
            mParent,
            config,
            array,
            _("Add Preset Array"),
            true
        );
        if (wxID_OK == dlg.ShowModal()) {
            config.presetArrays.addArray(array.name);
            const auto lastIdx{static_cast<int32>(
                config.presetArrays.arrays().size() - 1
            )};
            config.presetArrays.selection = lastIdx;
        }

    }, eID_Add_Array);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        presetArrays.removeArray(presetArrays.selection);
    }, eID_Remove_Array);
}

void PresetsPage::handleNotification(uint32 id) {
    bool rebound{id == pcui::Notifier::eID_Rebound};

    auto& presetArrays{mParent->getOpenConfig().presetArrays};
    if (rebound or id == Config::PresetArrays::NOTIFY_INJECTIONS) {
        rebuildInjections();
    }
    if (rebound or id == Config::PresetArrays::NOTIFY_SELECTION) {
        bool hasSelection{presetArrays.selection != -1};
        FindWindow(eID_Remove_Array)->Enable(hasSelection);
        FindWindow(eID_Rename_Array)->Enable(hasSelection);
        FindWindow(eID_Add_Preset)->Enable(hasSelection);
    }
    if (
            rebound or
            id == Config::PresetArrays::NOTIFY_ARRAY_NAME or
            id == Config::PresetArrays::NOTIFY_SELECTION
       ) {
        auto *issueButton{FindWindow(eID_Issue_Button)};
        // This is late for sizing reasons
        issueButton->SetLabel(L"\u26D4");

        if (presetArrays.selection == -1) {
            issueButton->Hide();
        } else {
            auto& selectedArray{presetArrays.array(presetArrays.selection)};
            auto selectedArrayName{static_cast<string>(selectedArray.name)};
            bool duplicate{false};
            for (const auto& array : presetArrays.arrays()) {
                if (&*array == &selectedArray) continue;

                if (static_cast<string>(array->name) == selectedArrayName) {
                    duplicate = true;
                    break;
                }
            }
            issueButton->Show(
                static_cast<string>(selectedArray.name).empty() or duplicate
            );
        }
    }
    if (
            rebound or
            id == Config::PresetArrays::NOTIFY_PRESETS or
            id == Config::PresetArrays::NOTIFY_SELECTION
       ) {
        if (presetArrays.selection == -1) {
            FindWindow(eID_Move_Preset_Up)->Disable();
            FindWindow(eID_Move_Preset_Down)->Disable();
            FindWindow(eID_Duplicate_Preset)->Disable();
            FindWindow(eID_Remove_Preset)->Disable();
        } else {
            auto& presetArray{presetArrays.array(presetArrays.selection)};
            bool hasPresetSelection{presetArray.selection != -1};

            bool notFirst{presetArray.selection > 0};
            const auto lastIdx{presetArray.selection.choices().size() - 1};
            bool notLast{presetArray.selection < lastIdx};

            auto *movePresetUp{FindWindow(eID_Move_Preset_Up)};
            movePresetUp->Enable(hasPresetSelection and notFirst);
            auto *movePresetDown{FindWindow(eID_Move_Preset_Down)};
            movePresetDown->Enable(hasPresetSelection and notLast);
            auto *removePreset{FindWindow(eID_Remove_Preset)};
            removePreset->Enable(hasPresetSelection);
            auto *dupPreset{FindWindow(eID_Duplicate_Preset)};
            dupPreset->Enable(hasPresetSelection);
        }
    }
    if (
            rebound or
            id == Config::PresetArrays::NOTIFY_TRACK_INPUT or
            id == Config::PresetArrays::NOTIFY_PRESETS or
            id == Config::PresetArrays::NOTIFY_SELECTION
       ) {
        auto hasInput{
            presetArrays.trackProxy.data() and
            not static_cast<string>(*presetArrays.trackProxy.data()).empty()
        };
        FindWindow(eID_Wav_Text)->Show(hasInput);
    }

    Layout();
}

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

