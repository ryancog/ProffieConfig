#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/app/app.h
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

#include <wx/app.h>
#include <wx/menu.h>

#include <utils/types.h>

#include "private/export.h"

namespace App {

/**
 * Initialize an application
 *
 * @param appName the name to use in the Application
 * @param lockName the name to use to lock concurrent runs, defaults to appName
 *
 * @return if the app should continue running or not.
 */
APP_EXPORT bool init(const string& appName, const string& lockName = {});

APP_EXPORT void exceptionHandler();

struct APP_EXPORT Menus {
    wxMenuBar *menuBar;
    wxMenu *file;
    wxMenu *edit;
    wxMenu *view;
    wxMenu *help;
};

// This is a memory leak if not used!
APP_EXPORT Menus createDefaultMenuBar();

#if defined(__WIN32__)
APP_EXPORT [[nodiscard]] bool darkMode();
#endif

inline string getAppName() { return wxApp::GetGUIInstance()->GetAppName(); }

} // namespace App


