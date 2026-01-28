/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/main.cpp
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

#include <wx/app.h>
#include <wx/image.h>

#include "app/app.h"
#include "config/info.h"
#include "core/appstate.h"
#include "ui/message.h"
#include "versions/versions.h"

class ProffieConfig : public wxApp {
public:
    bool OnInit() override {
        if (not App::init("ProffieConfig")) {
            pcui::showMessage(_("ProffieConfig is Already Running"), App::getAppName());
            return false;
        }

        Config::setExecutableVersion(wxSTRINGIZE(VERSION));
        Versions::loadLocal();

        // Version and prop stuffage
        // Arduino relink

        AppState::init();

        return true;
    }
};

wxIMPLEMENT_APP(ProffieConfig);

