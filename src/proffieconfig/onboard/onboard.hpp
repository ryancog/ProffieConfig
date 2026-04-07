#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/onboard/onboard.hpp
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

#include "data/bool.hpp"
#include "data/choice.hpp"
#include "data/string.hpp"
#include "ui/frame.hpp"

#include "pages/info.hpp"
#include "pages/setup.hpp"
#include "pages/welcome.hpp"

namespace onboard {

wxStaticText *createHeader(wxWindow *, const wxString&);

struct Frame : pcui::Frame {
    enum Phase {
        ePhase_Welcome,
        ePhase_Setup_Pre,
        ePhase_Setup_Prog,
        ePhase_Setup_Fail,
        ePhase_Setup_Done,
        ePhase_Info,
        ePhase_Max,
    };

    Frame();
    ~Frame() override;

    static Frame* instance;

private:
    friend struct Setup;

    pcui::DescriptorPtr ui();

    void bindEvents();
    void createMenuBar();

    data::Choice mPhase;

    data::Bool mMayCancel;
    data::Bool mMaySkip;
    data::Bool mMayGoBack;
    data::String mNextButton;

    bool mSetupDone{false};

    Welcome mWelcomePage;
    Setup mSetupPage;
    Info mInfoPage;
};

} // namespace onboard

