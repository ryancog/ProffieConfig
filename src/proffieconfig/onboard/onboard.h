#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/onboard/onboard.h
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

#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/panel.h>

#include "pages/info.h"
#include "pages/setup.h"
#include "pages/welcome.h"
#include "ui/frame.h"
#include "ui/notifier.h"

namespace Onboard {

wxStaticText *createHeader(wxWindow *, const wxString&);

class Frame : public pcui::Frame, pcui::NotifyReceiver {
public:
    static Frame* instance;
    Frame();
    ~Frame() override;

    enum {
        ID_Next,
        ID_Skip,
    };

private:
    void handleNotification(uint32) override;
    void bindEvents();

    Onboard::Welcome *mWelcomePage{nullptr};
    Onboard::Setup *mSetupPage{nullptr};
    Onboard::Info *mInfoPage{nullptr};
};

} // namespace Onboard

