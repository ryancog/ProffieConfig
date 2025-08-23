#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/setup.h
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

#include <wx/gauge.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include "ui/notifier.h"

namespace Onboard {

class Setup : public wxPanel {
public:
    Setup(wxWindow*);

    bool isDone{false};
    void startSetup();
    // Called in from done or failed
    void finishSetup(bool done);

    wxString errorMessage;
    wxString statusMessage;

    enum {
        ID_DONE,
        ID_FAILED,
        ID_STATUS,
    };
    PCUI::Notifier notifier;

    wxStaticText *loadingText{nullptr};

private:
    wxTimer *mLoadingTimer{nullptr};
    wxGauge *mLoadingBar{nullptr};
    wxStaticText *mDoneMessage{nullptr};
    wxStaticText *mPressNextMessage{nullptr};

    bool mCoreInstalled{false};
    bool mOSInstalled{false};
#   if defined(_WIN32) or defined(__linux__)
    bool mDriverInstalled{false};
#   endif
};

} // namespace Onboard


