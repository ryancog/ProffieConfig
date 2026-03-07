#include "welcome.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/welcome.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <wx/hyperlink.h>
#include <wx/sizer.h>

#include "ui/build.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/hyperlink.hpp"
#include "ui/static/label.hpp"
#include "utils/paths.hpp"

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
pcui::DescriptorPtr onboard::Welcome::ui() {
    /*
     * So, cool story, the WIDE window bug on macOS has to do with double
     * newline characters. E.g. "\n\n".
     *
     * Why? Don't Know.
     */
    return pcui::Stack{
        .orient_=wxVERTICAL,
        .children_={
            pcui::Spacer{20}(),
            pcui::Label{
                .label_=wxString::Format(_("Welcome to ProffieConfig %s!"), wxSTRINGIZE(BIN_VERSION)),
                .style_=pcui::text::Style::Header,
            }(),
            pcui::Spacer{40}(),
            pcui::Label{
                .base_={.align_=wxALIGN_CENTER,},
                .label_=_("Thank you for trying out ProffieConfig, the all-in-one proffieboard management utility!"),
            }(),
            pcui::Spacer{20}(),
            pcui::Label{
                .base_={.align_=wxALIGN_CENTER,},
                .label_=_("Online guides are available at the link below:"),
            }(),
            pcui::Hyperlink{
                .label_=_("Guides And Documentation"),
                .link_=paths::website() + "/guides",
            }(),
            pcui::Spacer{20}(),
            pcui::Label{
                .base_={.align_=wxALIGN_CENTER,},
                .label_=_(
                    "To start, ProffieConfig needs to do some setup.\n"
                    "Press \"Next\" when you're ready to continue, and we'll get started!"
                ),
            }(),
        }
    }();
}

