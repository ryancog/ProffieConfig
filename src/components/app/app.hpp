#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/app/app.hpp
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

#include "utils/types.hpp"

#include "app_export.h"

namespace app {

/**
 * Setup one-instance-only lock
 */
[[nodiscard]] APP_EXPORT bool setupExclusion(const std::string& lockName);

/**
 * Initialize an application
 *
 * @return if the app should continue running or not.
 */
[[nodiscard]] APP_EXPORT bool init();

#if defined(_WIN32)
APP_EXPORT bool darkMode();
#endif

/**
 * Set the app name.
 *
 * Is expected to happen first, and before init/setupExclusion.
 * Should only be called once.
 */
APP_EXPORT void setName(const wxString& appName);
APP_EXPORT wxString getName();

using ShowMessageFunc = int32(
    const wxString&, const wxString&, long, wxWindow *
);

/**
 * Provide app component with some UI helpers it cannot link
 */
APP_EXPORT void provideUI(
    ShowMessageFunc showMessage
);

} // namespace app

