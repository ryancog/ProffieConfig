#include "licenses.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/licenses.cpp
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
#include "ui/controls/text.hpp"
#include "ui/font.hpp"
#include "ui/helpers/foreach.hpp"
#include "ui/layout/collapsible.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/values.hpp"

#include "../../core/licenses.hpp"
#include "../mainmenu.hpp"

LicenseDialog::LicenseDialog(MainMenu *mainMenu) : 
    Dialog(
        mainMenu,
        wxID_ANY,
        _("ProffieConfig Copyright & License Info"),
        wxDEFAULT_DIALOG_STYLE | wxCENTER | wxRESIZE_BORDER
    ) {
    pcui::build(this, ui());
}

LicenseDialog::~LicenseDialog() {
    pcui::teardown(this);
}

pcui::DescriptorPtr LicenseDialog::ui() {
    auto licenseFont{pcui::detail::FontData{}.makeFont()};
    licenseFont.SetFamily(wxFONTFAMILY_TELETYPE);

    const auto licenseBoxWidth{[&] {
        int x{};
        int y{};
        GetTextExtent(
            'M',
            &x, &y,
            nullptr,
            nullptr,
            &licenseFont
        );
        return x * 80;
    }()};

    const auto entryGenerator{[&](
        const LicenseInfo& info
    ) -> pcui::DescriptorPtr {
        return pcui::Stack{
          .base_={.expand_=true},
          .children_={
            pcui::Label{
              .label_=info.name_,
              .font_=pcui::Font::Header,
            }(),
            pcui::Label{
              .label_=wxString{"Copyright "} + pcui::syms::COPYRIGHT + ' ' +
                  info.date_ + ' ' + info.author_
            }(),
            pcui::Label{
              .label_=info.detail_,
            }(),
            pcui::Collapsible{
              .win_={.base_={.expand_=true}},
              .showLabel_="Show License",
              .hideLabel_="Hide License",
              .child_=pcui::Text{
                .win_={
                  .base_={
                    .minSize_={licenseBoxWidth, -1},
                    .expand_=true,
                  },
                },
                .data_=info.license_,
                .autoLink_=true,
                .font_=licenseFont,
                .style_=pcui::Text::MultiLine{
                  .scroll_={.vertical_=false, .horizontal_=false},
                }
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
          }
        }();
    }};

    return pcui::Scrolled{
      .win_={
        .base_={
          .minSize_={-1, 600},
          .expand_=true,
          .proportion_=1,
        },
      },
      .scrollRate_={.y_=10},
      .child_=pcui::Stack{
        .base_={
            .expand_=true,
            .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxLEFT | wxRIGHT},
        },
        .children_={
          pcui::Spacer{.size_=pcui::winEdgeSpacing()}(),
          pcui::ForEach{
            .of_=LICENSES,
            .do_=entryGenerator,
          }(),
          pcui::Spacer{
            .size_=pcui::winEdgeSpacing() - pcui::interGroupSpacing()
          }(),
        },
      }(),
    }();
}

