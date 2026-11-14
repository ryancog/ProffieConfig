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

#include "dialogs/preferences.hpp"
#include "dialogs/serialmonitor.hpp"
#include "dialogs/versions.hpp"

#include "../tools/ports.hpp"

struct EditorWindow;

struct MainMenu : pcui::Frame, data::Receiver, ports::Listener {
    enum {
        // on macOS menu items cannot have ID 0
        // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl?
        eID_Update_Manifest = wxID_HIGHEST,
        eID_Logs,
        eID_Licenses,

        eID_Guides,
        eID_Issue,
        eID_Run_Setup,

        eID_Port_Choice,
        eID_Config_Entry_Label,
        eID_Config_Entry_Line,
    };

    MainMenu(wxWindow * = nullptr);
    ~MainMenu() override;

    void removeEditor(EditorWindow *);

    static MainMenu* instance;

private:
    pcui::DescriptorPtr ui();

    void createMenuBar();
    void bindEvents();

    void on_port_update() final;

    void on_board_choice();
    void on_add_config();

    bool check_config_valid(
        const data::base::Vector::ROContext&,
        config::Info&
    );
    void on_remove_config(config::Info&);
    void on_edit_config(config::Info&);

    void onApplyConfig();
    void onOpenSerial();
    void on_manage_versions();

    pcui::DescriptorPtr config_entry(data::base::Model&);

    data::prim::Choice mBoardChoice;
    data::prim::Selector mConfigSel;

    std::string m_last_serial_selected;
    std::vector<arduino::Port> mBoards;
    std::map<config::Info *, EditorWindow *> mEditors;
    std::set<SerialMonitorDlg *> mMonitors;

    VersionsDlg *mVersionsDlg{nullptr};
    PreferencesDlg *mPrefsDlg{nullptr};
};

