#include "bladearray.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/bladearray.cpp
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

#include "data/logic/adapter.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/controls/text.hpp"
#include "ui/dialog.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "wx/event.h"

BladeArrayDlg::BladeArrayDlg(
    wxWindow *parent,
    config::blades::BladeConfig& config,
    bool mayCancel
) : pcui::Dialog(parent, wxID_ANY, _("Edit Blade Array")),
    mConfig{config} {
    pcui::build(this, ui(mayCancel));
}

pcui::DescriptorPtr BladeArrayDlg::ui(bool mayCancel) {
    using namespace config::blades;

    return pcui::Stack{
      .base_={
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Labeled{
          .base_={.expand_=true},
          .label_=_("Name"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Text{
            .win_={
              .base_={.expand_=true},
              .tooltip_=_("The name of the blade array.\nEach name must be unique, but it is for reference only (and thus specific names will not make a difference)."),
            },
            .data_=mConfig.name_,
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Labeled{
              .base_={
                .minSize_={100, -1},
                .proportion_=1,
              },
              .label_=_("Blade ID"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::Stepper{
                .win_={
                  .tooltip_=_("The ID of the blade associated with the currently-selected blade array.\nThis value can be measured by typing \"id\" into the Serial Monitor."),
                },
                .data_=mConfig.id_,
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.align_=wxALIGN_BOTTOM},
              },
              .label_="NO_BLADE",
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .base_={
                .minSize_={100, -1},
                .proportion_=2,
              },
              .label_=_("Preset Array"),
              .orient_=wxVERTICAL,
              .ctrl_=pcui::Stepper{
                .win_={
                  .tooltip_=_("The ID of the blade associated with the currently-selected blade array.\nThis value can be measured by typing \"id\" into the Serial Monitor."),
                },
                .data_=mConfig.id_,
              }(),
            }(),
          }
        }(),
        pcui::Label{
          .win_={
            .base_={
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP}
            },
            .show_=mConfig.issues() | data::logic::BitAnd{
                .val_=BladeConfig::eIssue_No_Preset_Array
            },
          },
          .label_=_("Blade Array is not linked to a Preset Array"),
        }(),
        pcui::Label{
          .win_={
            .base_={
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP}
            },
            .show_=mConfig.issues() | data::logic::BitAnd{
                .val_=BladeConfig::eIssue_Duplicate_ID
            },
          },
          .label_=_("Blade Array has duplicate ID"),
        }(),
        pcui::Label{
          .win_={
            .base_={
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP}
            },
            .show_=mConfig.issues() | data::logic::BitAnd{
                .val_=BladeConfig::eIssue_Duplicate_Name
            },
          },
          .label_=_("Blade Array has duplicate name"),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::DialogButtons{
          .base_={.expand_=true},
          .ok_=pcui::Button{
            .label_=mayCancel ? _("OK") : _("Close"),
            .func_=[this]() {
                EndModal(wxID_OK);
            },
          }(),
          .cancel_=mayCancel ? pcui::Button{
            .label_=_("Cancel"),
            .func_=[this]() {
                EndModal(wxID_CANCEL);
            },
          }() : nullptr,
        }(),
      }
    }();
}

