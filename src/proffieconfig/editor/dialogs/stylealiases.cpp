#include "stylealiases.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/stylealiases.cpp
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

#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/translation.h>

#include "config/styles/style.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "ui/build.hpp"
#include "ui/builders/selector.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/text.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/split.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/values.hpp"

StyleAliasesDlg::StyleAliasesDlg(wxWindow *parent, config::Config& config) :
    Dialog(parent, wxID_ANY, _("Style Aliases")), mConfig{config} {

    mStyleSel.bind(&mConfig.styles_);

    pcui::build(this, ui());
}

void StyleAliasesDlg::deinit() {
    pcui::cripple(this);

    mStyleSel.bind(nullptr);
}

pcui::DescriptorPtr StyleAliasesDlg::ui() {
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
      }
    }();
}

pcui::DescriptorPtr StyleAliasesDlg::selection() {
    return pcui::Stack{
      .base_={.expand_=true},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Group{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .label_=_("Aliases"),
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
                  },
                  .data_=mStyleSel,
                  .style_=pcui::Choice::List{},
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      auto sel{data::context(mStyleSel)};
                      auto vec{data::context(*sel.bound())};
                      using namespace config::styles;
                      auto& style{dynamic_cast<Style&>(*vec.children()[idx])};
                      return style.name_;
                  },
                }(),
                pcui::Stack{
                  .orient_=wxHORIZONTAL,
                  .children_={
                    pcui::Button{
                      .win_={.base_={.minSize_=pcui::iconButtonSize()}},
                      .label_=pcui::syms::PLUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=[this] { onAddButton(); },
                    }(),
                    pcui::Button{
                      .win_={
                        .base_={.minSize_=pcui::iconButtonSize()},
                        .enable_=mStyleSel.choice() |
                            data::logic::HasSelection{},
                      },
                      .label_=pcui::syms::MINUS,
                      .style_=pcui::Button::Style::Companion,
                      .exactFit_=true,
                      .func_=[this] { onRemoveButton(); },
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
                    .enable_=mStyleSel | data::logic::CanMoveUp{},
                  },
                  .label_=pcui::syms::UP_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onMoveUpButton(); },
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mStyleSel | data::logic::CanMoveDown{},
                  },
                  .label_=pcui::syms::DOWN_ARROW,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onMoveDownButton(); },
                }(),
                pcui::Button{
                  .win_={
                    .base_={.minSize_=pcui::iconButtonSize(true)},
                    .enable_=mStyleSel.choice() | data::logic::HasSelection{},
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

pcui::DescriptorPtr StyleAliasesDlg::fields() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Labeled{
          .win_={.base_={.expand_=true}},
          .label_=_("Name"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::builders::Selector{
            .data_=mStyleSel,
            .builder_=[](data::base::Model *model) {
                pcui::Text text{
                  .win_={
                    .base_={.expand_=true},
                    .tooltip_=_(
                        "The name for the alias.\n"
                        "This is the name you can use in a preset's bladestyle.\n"
                        "There cannot be duplicate names."
                    )
                  },
                };
                
                if (model == nullptr) {
                    text.win_.enable_ = false;
                    return text();
                }

                auto *style{dynamic_cast<config::styles::Style *>(model)};
                text.data_=style->name_;
                return text();
            },
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
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
                          "Any comments about the alias goes here.\n"
                          "This doesn't affect the alias at all, but can be a place for helpful notes!"
                      ),
                    }, 
                    .style_=pcui::Text::MultiLine{},
                  };

                  if (model == nullptr) {
                      text.win_.enable_ = false;
                      text.data_ = _("Select or create style alias to edit comments...");
                      return text();
                  }

                  auto *style{dynamic_cast<config::styles::Style *>(model)};
                  text.data_ = style->comments_;
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
                .label_=_("Content"),
              }(),
              pcui::builders::Selector{
                .data_=mStyleSel,
                .builder_=[](data::base::Model *model) {
                  pcui::Text text{
                    .win_={
                      .base_={.expand_=true, .proportion_=1},
                      .tooltip_=_(
                          "Blade style content goes here.\n"
                          "This will be style code, or a small part of it, which you can use "
                          "in the preset bladestyles or in another alias later in the list.\n"
                          "Importantly, this is just the C++ Type; i.e. it shouldn't have any parentheses."
                      )
                    }, 
                    .style_=pcui::Text::MultiLine{
                      .wrap_=pcui::Text::Wrap::None,
                    },
                  };

                  if (model == nullptr) {
                      text.win_.enable_ = false;
                      text.data_ = _("Select or create style alias to edit style...");
                      return text();
                  }

                  auto *style{dynamic_cast<config::styles::Style *>(model)};
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

void StyleAliasesDlg::onAddButton() {
    auto sel{data::context(mStyleSel)};
    auto vec{data::context(const_cast<data::base::Vector&>(*sel.bound()))};
    vec.append<config::styles::Style>(mConfig);
}

void StyleAliasesDlg::onRemoveButton() {
    auto sel{data::context(mStyleSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.remove(sel.choiceIdx());
}

void StyleAliasesDlg::onMoveUpButton() {
    auto sel{data::context(mStyleSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.moveUp(sel.choiceIdx());
}

void StyleAliasesDlg::onMoveDownButton() {
    auto sel{data::context(mStyleSel)};
    auto& vec{const_cast<data::base::Vector&>(*sel.bound())};
    vec.moveDown(sel.choiceIdx());
}

void StyleAliasesDlg::onDuplicateButton() {
    auto sel{data::context(mStyleSel)};
    auto vec{data::context(
        const_cast<data::base::Vector&>(*sel.bound())
    )};

    auto& source{dynamic_cast<config::styles::Style&>(
        *vec.children()[sel.choiceIdx()]
    )};

    vec.insert(
        sel.choiceIdx() + 1,
        std::make_unique<config::styles::Style>(
            source, mConfig
        )
    );
    mStyleSel.choice().choose(sel.choiceIdx() + 1);
}

void StyleAliasesDlg::onFormatButton() {
    auto sel{data::context(mStyleSel)};
    auto *style{sel.selected<config::styles::Style>()};
    style->content_.change(style->format());
}

