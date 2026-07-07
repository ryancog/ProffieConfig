#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/general.hpp
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

#include "../dialogs/buttons.hpp"
#include "../dialogs/customopts.hpp"

struct GeneralPage {
    GeneralPage(config::Config&);
    void deinit();

    pcui::DescriptorPtr ui();

private:
    pcui::DescriptorPtr setup();
    pcui::DescriptorPtr misc();
    pcui::DescriptorPtr installation();
    pcui::DescriptorPtr tweaks();
    pcui::DescriptorPtr editing();
    pcui::DescriptorPtr audio();

    config::Config& mConfig;

    ButtonsDlg *mButtonDlg{nullptr};
    CustomOptionsDlg *mOptionsDlg{nullptr};
};

