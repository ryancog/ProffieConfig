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

#if _WIN32
#include <cfgmgr32.h>
#elif __APPLE__
#include <IOKit/IOKitLib.h>
#endif

#include "config/config.hpp"
#include "data/primitive/models/choice.hpp"
#include "data/primitive/models/selector.hpp"
#include "ui/frame.hpp"
#include "ui/types.hpp"

#include "dialogs/preferences.hpp"
#include "dialogs/serialmonitor.hpp"
#include "dialogs/versions.hpp"

struct EditorWindow;

struct MainMenu : pcui::Frame, data::Receiver {
    enum {
        // on macOS menu items cannot have ID 0
        // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl?
        eID_Manage_Versions = wxID_HIGHEST,
        eID_Update_Manifest,
        eID_Logs,
        eID_Licenses,

        eID_Guides,
        eID_Issue,
        eID_Run_Setup,
    };

    MainMenu(wxWindow * = nullptr);
    ~MainMenu() override;

    void removeEditor(EditorWindow *);

    static MainMenu* instance;

private:
#   if _WIN32
    static DWORD CALLBACK pnp_notify_cb(
        HCMNOTIFICATION,
        PVOID,
        CM_NOTIFY_ACTION,
        PCM_NOTIFY_EVENT_DATA,
        DWORD
    );
#   elif __APPLE__
    static void io_notify_cb(void *, io_iterator_t);
#   endif

    pcui::DescriptorPtr ui();

    void createMenuBar();
    void bindEvents();
    void refresh_boards();

    void on_board_choice();
    void onAddConfig();
    void onRemoveConfig();
    void onEditConfig();

    void onApplyConfig();
    void onOpenSerial();

    data::prim::Choice mBoardChoice;
    data::prim::Selector mConfigSel;

    std::string m_last_serial_selected;
    std::vector<arduino::Port> mBoards;
    std::map<config::Info *, EditorWindow *> mEditors;
    std::set<SerialMonitorDlg *> mMonitors;

    VersionsDlg *mVersionsDlg{nullptr};
    PreferencesDlg *mPrefsDlg{nullptr};

#   if _WIN32
    HCMNOTIFICATION m_pnp_notify_handle;
#   elif __APPLE__
    IONotificationPortRef m_io_notify_port;
    io_iterator_t m_io_device_iter{IO_OBJECT_NULL};
    io_iterator_t m_io_serial_iter{IO_OBJECT_NULL};
    io_iterator_t m_io_removed_iter{IO_OBJECT_NULL};
#   endif
};

