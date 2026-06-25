#include "propinfo.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/propinfo.hpp
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
#include "ui/controls/text.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

PropInfoDlg::PropInfoDlg(wxWindow *parent, config::Config& config) :
    pcui::Dialog(parent, wxID_ANY, _("Prop Info")),
    mConfig{config} {
    pcui::build(this, ui());
}

pcui::DescriptorPtr PropInfoDlg::ui() {
    return pcui::builders::Choice{
      .data_=mConfig.propChoice(),
      .builder_=[this](int32 idx) -> pcui::DescriptorPtr {
          pcui::Text label{
            .win_={
              .base_={
                .minSize_={300, 100},
                .expand_=true,
                .proportion_=1,
                .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
              },
            },
            .readOnly_=true,
            .autoLink_=true,
            .font_=pcui::Font::Monospace,
            .style_=pcui::Text::MultiLine{
                .scroll_={.vertical_=false, .horizontal_=false},
                .wrap_=pcui::Text::Wrap::None,
            },
          };

          if (idx == -1) {
              label.data_ = _("The Default ProffieOS Prop");
              label.align_ = pcui::Text::Align::Center;
          }
          else {
              label.data_ = mConfig.prop()->info_;
          }

          return label();
      },
    }();
}

