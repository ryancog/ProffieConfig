#include "manifest.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/manifest.cpp
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

#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/values.hpp"

#include "../mainmenu.hpp"
#include "../../core/state.hpp"

ManifestDialog::ManifestDialog(MainMenu *mainMenu) : 
    Dialog{mainMenu, wxID_ANY, _("Set Update Channel")} {
    constexpr cstring STABLE_CHANNEL{"stable"};

    { data::String::Context ctxt{mText};
        ctxt.change(state::manifestChannel.empty()
            ? STABLE_CHANNEL
            : state::manifestChannel
        );
    }
    const auto ui{pcui::Stack{
      .base_={
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .children_={
        pcui::Text{
          .win_={
            .base_={
              .expand_=true,
              .border_={.size_=2, .dirs_=wxALL},
            },
          },
          .data_=mText,
        }(),
        pcui::Spacer{.size_=8}(),
        pcui::DialogButtons{
          .ok_=pcui::Button{
            .win_={
              .enable_=not (mText | data::logic::IsEmpty{}),
            },
            .label_=_("Save"),
            .default_=true,
            .func_=[this] {
                data::String::Context str{mText};
                if (str.val() == STABLE_CHANNEL) state::manifestChannel.clear();
                else state::manifestChannel = str.val();

                state::saveState();
                Close();
            }
          }(),
          .cancel_=pcui::Button{
            .label_=_("Cancel"),
            .func_=[this] {
                Close();
            }
          }(),
          .apply_=pcui::Button{
            .label_=_("Reset to Default"),
            .func_=[this] {
                data::String::Context{mText}.change(STABLE_CHANNEL);
            }
          }(),
        }(),
      },
    }()};

    pcui::build(this, ui);
}

ManifestDialog::~ManifestDialog() {
    pcui::teardown(this);
}

