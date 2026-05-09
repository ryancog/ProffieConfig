#include "customopts.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/customopts.cpp
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

#include "config/settings/define.hpp"
#include "data/context.hpp"
#include "ui/build.hpp"
#include "ui/builders/vecstack.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/text.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/hyperlink.hpp"
#include "ui/static/label.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

CustomOptionsDlg::CustomOptionsDlg(wxWindow *parent, config::Config& config) :
    pcui::Dialog(
        parent,
        wxID_ANY,
        _("Custom Options"),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    mConfig{config} {
    pcui::build(this, ui());
}

pcui::DescriptorPtr CustomOptionsDlg::ui() {
    return pcui::Stack{
      .base_={
        .minSize_={400, 500},
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Group{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .padded_=false,
          .children_={
            pcui::Scrolled{
              .win_={.base_={.expand_=true, .proportion_=1}},
              .scrollRate_={.y_=4},
              .child_=pcui::builders::VecStack{
                .base_={
                  .expand_=true,
                  .proportion_=1,
                  .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
                },
                .orient_=wxVERTICAL,
                .data_=mConfig.settings_.defines_,
                .builder_=option,
                .separator_=pcui::Stack{
                  .base_={.expand_=true},
                  .children_={
                    pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                    pcui::Divider{
                      .base_={.expand_=true},
                      .orient_=wxHORIZONTAL,
                    }(),
                    pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                  }
                }(),
                .empty_=pcui::Stack{
                  .base_={.expand_=true, .proportion_=1},
                  .children_={
                    pcui::StretchSpacer{}(),
                    pcui::Label{
                      .win_={
                        .base_={
                          .border_={
                            .size_=pcui::interGroupSpacing(),
                            .dirs_=wxALL
                          },
                          .align_=wxALIGN_CENTER,
                        },
                      },
                      .label_=_("Once you add custom options they'll show up here."),
                      .color_=wxSYS_COLOUR_GRAYTEXT,
                    }(),
                    pcui::StretchSpacer{}(),
                  }
                }(),
              }(),
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={.base_={.align_=wxALIGN_RIGHT}},
          .label_=_("Add Option"),
          .func_=[this] { addOption(); },
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        info(),
      }
    }();
}

pcui::DescriptorPtr CustomOptionsDlg::info() {
    return pcui::Group{
      .win_={.base_={.expand_=true}},
      .label_=_("Links For Additional ProffieOS Defines"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::Label{
          .label_=_("ProffieConfig already handles most of these"),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Hyperlink{
          .label_=_("Optional Defines"),
          .link_="https://pod.hubbe.net/config/the-config_top-section.html#optional-defines",
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Hyperlink{
          .label_=_("History of Clash Detection"),
          .link_="https://pod.hubbe.net/explainers/history-of-clash.html",
        }(),
      }
    }();
}

pcui::DescriptorPtr CustomOptionsDlg::option(data::base::Model& model) {
    auto& option{dynamic_cast<config::settings::Define&>(model)};

    return pcui::Stack{
      .base_={.expand_=true},
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Label{
          .win_={.base_={.align_=wxALIGN_CENTER}},
          .label_="#define"
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Text{
          .win_={.base_={.proportion_=5}},
          .data_=option.name_,
          .style_=pcui::Text::SingleLine{
            .hint_=_("Name"),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Text{
          .win_={.base_={.proportion_=3}},
          .data_=option.value_,
          .style_=pcui::Text::SingleLine{
            .hint_=_("Value"),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .label_=_("Remove"),
          .func_=[&option, &model](pcui::CallbackContext ctxt) {
              auto& vec{option.root<config::Config>().settings_.defines_};

              // Removing the model will destroy this UI first, and the UI
              // cannot be destroyed from inside the callback.
              ctxt.topLevel_->CallAfter([&vec, &model] {
                  auto vecCtxt{data::context(vec)};
                  vecCtxt.remove(model);
              });
          }
        }(),
      }
    }();
}

void CustomOptionsDlg::addOption() {
    auto vec{data::context(mConfig.settings_.defines_)};
    vec.append(std::make_unique<config::settings::Define>(mConfig));
}

