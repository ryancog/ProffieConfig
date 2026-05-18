#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/props.hpp
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

#include "config/config.hpp"
#include "ui/types.hpp"

#include "../dialogs/propbuttons.hpp"
#include "../dialogs/propinfo.hpp"

struct PropsPage {
    PropsPage(config::Config&);
    void deinit();

    pcui::DescriptorPtr ui(wxWindowID);

private:
    void onInfoButton(const pcui::CallbackContext&);
    void onControlsButton(const pcui::CallbackContext&);

    config::Config& mConfig;

    PropButtonsDlg *mButtonsDlg{nullptr};
    PropInfoDlg *mInfoDlg{nullptr};
};

