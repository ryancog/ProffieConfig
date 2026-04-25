#include "info.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/info.cpp
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

#include <wx/sizer.h>

#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"

#include "ui/static/label.hpp"

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
pcui::DescriptorPtr onboard::Info::ui() {
    return pcui::Stack{
      .orient_=wxVERTICAL,
      .children_={
        pcui::Spacer{20}(),
        pcui::Label{
          .label_=_("Ready To Go!"),
          .font_=pcui::Font::Title,
        }(),
        pcui::Spacer{20}(),
        pcui::Label{
          .label_={
            _("ProffieConfig is all set up and ready to go!") + '\n' +
            '\n' +
            _("You can always re-run this setup from the main menu File drop-down.") + '\n' +
            '\n' +
            _(
              "Additionally, the documentation linked earlier is available in "
              "the Help menu, and don't hesitate to reach out directly either. "
              "I'm available in a few places which I list on the ProffieConfig website."
            ) + '\n' +
            '\n' +
            _("Happy Saber-ing! :)")
          },
          .wrapWidth_=550,
        }(),
      }
    }();
}

