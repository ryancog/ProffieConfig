#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/onboard/pages/setup.hpp
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

#include <wx/gauge.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include "data/primitive/models/string.hpp"
#include "ui/types.hpp"
#include "ui/indication/progress.hpp"

namespace onboard {

struct Frame;

struct Setup {
    Setup(Frame&);
    ~Setup();

    pcui::DescriptorPtr ui();

    void startSetup();

    data::prim::String errorMessage_;

private:
    Frame& mParent;
    wxTimer *mLoadingTimer{nullptr};

    pcui::Progress::Data mProgress;
    data::prim::String mStatusMessage;

    bool mOSInstalled{false};
#   if defined(_WIN32) or defined(__linux__)
    bool mDriverInstalled{false};
#   endif
};

} // namespace onboard

