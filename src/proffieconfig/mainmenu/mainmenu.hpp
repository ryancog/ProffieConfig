#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/mainmenu.hpp
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

#include <map>

#include "config/config.hpp"
#include "data/primitive/models/choice.hpp"
#include "data/primitive/models/selector.hpp"
#include "ui/frame.hpp"
#include "ui/types.hpp"

struct EditorWindow;

struct MainMenu : pcui::Frame {
    enum {
        // on macOS menu items cannot have ID 0
        // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl?
        eID_Licenses = 2,

        eID_Docs,
        eID_Issue,
        eID_Logs,

        eID_Manage_Versions,
        eID_Update_Manifest,

        eID_Refresh_Dev,
        eID_Apply_Changes,

        eID_Open_Serial,
        eID_Add_Config,
        eID_Remove_Config,
        eID_Edit_Config,

        eID_Board_Selection,
        eID_Config_Selection,

        eID_Run_Setup,
        eID_Async_Start,
        eID_Async_Done,
    };

    MainMenu(wxWindow * = nullptr);
    ~MainMenu() override;

    void removeEditor(EditorWindow *);

    data::prim::Choice board_;
    data::prim::Selector configSel_;

    static MainMenu* instance;

private:
    std::map<config::Info *, EditorWindow *> mEditors;

    pcui::DescriptorPtr ui();

    void createMenuBar();
    void bindEvents();

    void importConfig();
};

