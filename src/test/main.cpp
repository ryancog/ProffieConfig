/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * test/main.cpp
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

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <wx/app.h>

#include "app/app.h"
#include "config/info.h"
#include "ui/message.h"
#include "versions/versions.h"

class Test : public wxApp {
public:
    bool OnInit() override {
        if (not App::init("Test")) {
            pcui::showMessage(_("Test is Already Running"), App::getAppName());
            return false;
        }

        Config::setExecutableVersion(wxSTRINGIZE(VERSION));
        Versions::loadLocal();

        auto res{Catch::Session().run(argc, static_cast<char **>(argv))};

        return false;
    }
};

// NOLINTNEXTLINE
wxIMPLEMENT_APP(Test);

