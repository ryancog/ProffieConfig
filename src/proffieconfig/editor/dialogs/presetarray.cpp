#include "presetarray.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/presetarray.cpp
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

#include "config/presets/array.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/values.hpp"

PresetArrayDlg::PresetArrayDlg(
    wxWindow *parent,
    config::presets::Array& array,
    bool mayCancel
) : pcui::Dialog(parent, wxID_ANY, _("Edit Preset Array")),
    mArray{array} {
    pcui::build(this, ui(mayCancel));

    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& evt) {
        auto issues{data::context(mArray.issues())};
        if (
                (evt.GetKeyCode() == WXK_RETURN or
                evt.GetKeyCode() == WXK_NUMPAD_ENTER) and
                issues.val() == 0
           ) {
            EndModal(wxID_OK);
        } else evt.Skip();
    });
}

pcui::DescriptorPtr PresetArrayDlg::ui(bool mayCancel) {
    using namespace config::presets;

    return pcui::Stack{
      .base_={
        .minSize_={300, -1},
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Labeled{
          .win_={.base_={.expand_=true}},
          .label_=_("Name"),
          .orient_=wxVERTICAL,
          .ctrl_=pcui::Text{
            .win_={
              .base_={.expand_=true},
              .focus_=true,
            },
            .data_=mArray.name_,
          }(),
        }(),
        pcui::Label{
          .win_={
            .base_={
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
              .align_=wxALIGN_RIGHT,
            },
            .show_=mArray.issues() | data::logic::BitAnd{
                .val_=Array::eIssue_Name_Empty
            },
          },
          .label_=_("Preset Array must be named"),
        }(),
        pcui::Label{
          .win_={
            .base_={
              .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
              .align_=wxALIGN_RIGHT,
            },
            .show_=mArray.issues() | data::logic::BitAnd{
                .val_=Array::eIssue_Name_Duplicate
            },
          },
          .label_=_("Preset Array has duplicate name"),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::DialogButtons{
          .ok_=pcui::Button{
            .win_={
              .enable_=mArray.issues() | data::logic::Equals{.val_=0},
            },
            .label_=mayCancel ? _("OK") : _("Close"),
            .default_=true,
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

