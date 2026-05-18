#include "props.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/props.cpp
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

#include "ui/build.hpp"
#include "ui/builders/choice.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

PropsPage::PropsPage(config::Config& config) : mConfig{config} {}

void PropsPage::deinit() {
    if (mButtonsDlg) {
        pcui::cripple(mButtonsDlg);

        mButtonsDlg->CallAfter([dlg=mButtonsDlg] {
            dlg->Destroy();
        });
    }

    if (mInfoDlg) {
        pcui::cripple(mInfoDlg);

        mInfoDlg->CallAfter([dlg=mInfoDlg] {
            dlg->Destroy();
        });
    }
}

pcui::DescriptorPtr PropsPage::ui() {
    return pcui::Stack{
      .base_={.border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL}},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Stack{
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Labeled{
              .label_=_("Prop File"),
              .ctrl_=pcui::Choice{
                .data_=mConfig.propChoice(),
                .style_=pcui::Choice::PopUp{
                  .unselected_=_("Default"),
                },
                .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                    return (*mConfig.propVec())[idx]->name_;
                },
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.align_=wxALIGN_BOTTOM},
                .tooltip_=_("View prop creator-provided information about this prop and its intended usage."),
              },
              .label_=_("Prop Description and Usage Info..."),
              .func_=[this](const pcui::CallbackContext& ctxt) {
                  onInfoButton(ctxt);
              },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.align_=wxALIGN_BOTTOM},
                .tooltip_=_("View button controls based on specific option settings and number of buttons."),
              },
              .label_=_("Button Controls..."),
              .func_=[this](const pcui::CallbackContext& ctxt) {
                  onControlsButton(ctxt);
              },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Scrolled{
          .scrollRate_={.x_=10, .y_=10},
          .child_=pcui::builders::Choice{
            .data_=mConfig.propChoice(),
            .builder_=[this](int32 idx) {
                if (idx == -1)
                    return pcui::Spacer{}();

                return (*mConfig.propVec())[idx]->layout();
            }
          }(),
        }(),
      }
    }();
}

void PropsPage::onInfoButton(const pcui::CallbackContext& ctxt) {
    if (mInfoDlg) {
        mInfoDlg->Show();
        mInfoDlg->Raise();
        return;
    }

    mInfoDlg = new PropInfoDlg(ctxt.topLevel_, mConfig);

    const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
        if (evt.GetEventObject() == mInfoDlg) mInfoDlg = nullptr;
    }};
    mInfoDlg->Bind(wxEVT_DESTROY, onDestroy);

    mInfoDlg->Show();
}

void PropsPage::onControlsButton(const pcui::CallbackContext& ctxt) {
    if (mButtonsDlg) {
        mButtonsDlg->Show();
        mButtonsDlg->Raise();
        return;
    }

    mButtonsDlg = new PropButtonsDlg(ctxt.topLevel_, mConfig);

    const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
        if (evt.GetEventObject() == mButtonsDlg) mButtonsDlg = nullptr;
    }};
    mButtonsDlg->Bind(wxEVT_DESTROY, onDestroy);

    mButtonsDlg->Show();
}

