#include "about.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/about.cpp
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

#include <string>

#include <wx/aboutdlg.h>
#include <wx/cpp.h>

#include "app/info.hpp"
#include "config/info.hpp"
#include "log/info.hpp"
#include "pconf/info.hpp"
#include "ui/info.hpp"
#include "utils/info.hpp"
#include "versions/info.hpp"

void showAbout(wxWindow *parent) {
    wxAboutDialogInfo aboutInfo;
    const auto componentVersions{
        std::string{} +
        "App: v" + app::version() + "\n"
        "Config: v" + config::version() + "\n"
        "Log: v" + logging::version() + "\n"
        "PConf: v" + pconf::version() + "\n"
        "pcui: v" + pcui::version() + "\n"
        "Utils: v" + utils::version() + "\n"
        "Versions: v" + versions::version() + "\n"
        // REVIEW
        // "Arduino CLI: v" + arduino::version() + "\n"
        //
    };
#   ifdef __WXOSX__
    aboutInfo.SetDescription(_("All-in-one Proffieboard Management Utility"));
    aboutInfo.SetVersion(
        wxSTRINGIZE(BIN_VERSION),
        "Core: v" + wxString{wxSTRINGIZE(BIN_VERSION)} + "\n"
        + componentVersions
    );
#   else
    aboutInfo.SetDescription(
        _("All-in-one Proffieboard Management Utility") + "\n\n"
        + componentVersions
    );
    aboutInfo.SetVersion(wxSTRINGIZE(BIN_VERSION));
#   endif
#   ifdef __WXGTK__
    aboutInfo.SetWebSite("https://proffieconfig.kafrenetrading.com");
#   endif
    aboutInfo.SetCopyright("Copyright (C) 2023-2026 Ryan Ogurek");
    aboutInfo.SetName("ProffieConfig");

    wxAboutBox(aboutInfo, parent);
}

