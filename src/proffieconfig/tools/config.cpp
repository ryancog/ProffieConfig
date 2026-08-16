#include "config.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/tools/config.cpp
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

#include "data/context.hpp"
#include "ui/dialogs/message.hpp"

#include "../core/state.hpp"

void extraConfigOptionProcessing(config::Config& config) {
    auto keepSaveFiles{data::context(config.settings_.keepSaveFiles_)};
    if (keepSaveFiles.val()) {
        namespace prefs = state::prefs;
        switch (prefs::get<prefs::Enum::Handle_Keep_Save_Files>()) {
            using enum prefs::enums::HandleKeepSaveFiles;
            case Ignore_Alert:
            {
                auto res{pcui::showHideablePrompt(
                    _("This config used \"Keep Savefiles When Programming,\" but it is blacklisted by your preferences.") + "\n" +
                    _("For most cases, this is okay, and you don't want to use that option.") + "\n" +
                    "\n" +
                    _("It can be allowed in ProffieConfig's settings, though I advise caution."),
                    {
                        .caption_=_("Ignoring Keep Savefiles When Programming"),
                    }
                )};

                if (res.wantsHide_) {
                    prefs::set<prefs::Enum::Handle_Keep_Save_Files>(
                        prefs::enums::HandleKeepSaveFiles::Ignore
                    );
                }

                [[fallthrough]];
            }
            case Ignore:
                keepSaveFiles.set(false);
                break;
            case Allow_Hide_Unused:
            case Allow:
                break;
            case Max:
                std::unreachable();
        }
    }
}

