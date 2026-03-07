#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/onboard/onboard.hpp
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

#include "pages/info.hpp"
#include "pages/setup.hpp"
#include "pages/welcome.hpp"
#include "ui/build.hpp"
#include "ui/frame.hpp"

namespace onboard {

wxStaticText *createHeader(wxWindow *, const wxString&);

class Frame : public pcui::Frame {
public:
    static Frame* instance;
    Frame();
    ~Frame() override;

    enum {
        eID_Next,
        eID_Skip,
    };

private:
    pcui::DescriptorPtr ui();
    void bindEvents();

    onboard::Welcome mWelcomePage;
    onboard::Setup mSetupPage;
    onboard::Info mInfoPage;
};

} // namespace onboard

