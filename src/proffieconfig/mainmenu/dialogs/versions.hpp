#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/versions.hpp
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

#include "data/base/model.hpp"
#include "data/primitive/models/selector.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/dialog.hpp"
#include "ui/types.hpp"

struct VersionsDlg : pcui::Dialog, data::Receiver {
    VersionsDlg(wxWindow *);
    ~VersionsDlg() override;

private:
    void onActivate() override;

    pcui::DescriptorPtr ui();

    pcui::DescriptorPtr props();
    static pcui::DescriptorPtr propInfo(data::base::Model *);

    pcui::DescriptorPtr os();
    static pcui::DescriptorPtr osInfo(data::base::Model *);

    void onFetchButton();

    void onPropInstallButton();
    void onPropRemoveButton();

    void onPropInstalledChange(size);
    void onPropAvailChoice();

    void updatePropInstall();

    void onOsInstallButton();
    void onOsRemoveButton();

    void onOsInstalledChange(size);
    void onOsAvailChoice();

    void updateOsInstall();

    data::prim::String mPropInstall;
    data::prim::String mOsInstall;

    data::prim::Selector mPropSel;
    data::prim::Selector mOsSel;

    data::prim::Selector mAvailPropSel;
    data::prim::Selector mAvailOsSel;
};

