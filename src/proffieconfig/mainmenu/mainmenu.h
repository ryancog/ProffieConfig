#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/mainmenu.h
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

#include <wx/button.h>
#include <wx/combobox.h>

#include "ui/frame.h"
#include "ui/notifier.h"

#include "../editor/editorwindow.h"

namespace Onboard {

class Overview;

} // namespace Onboard

class MainMenu : public PCUI::Frame, private PCUI::NotifyReceiver {
public:
    static MainMenu* instance;
    MainMenu(wxWindow * = nullptr);

    void removeEditor(EditorWindow *);

    PCUI::ChoiceData boardSelection;
    PCUI::ChoiceData configSelection;

    enum {
        // on macOS menu items cannot have ID 0
        // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl?
        ID_Copyright = 2,

        ID_Docs,
        ID_Issue,
        ID_Logs,

        ID_ManageVersions,
        ID_UpdateManifest,

        ID_RefreshDev,
        ID_ApplyChanges,

        ID_OpenSerial,
        ID_AddConfig,
        ID_RemoveConfig,
        ID_EditConfig,

        ID_BoardSelection,
        ID_ConfigSelection,

        ID_RunSetup,
        ID_AsyncStart,
        ID_AsyncDone,
    };

private:
    Config::Config *mConfigNeedShown{nullptr};

    PCUI::Notifier mNotifyData;
    vector<EditorWindow *> mEditors;

    friend Onboard::Overview;
    void createUI();
    void createMenuBar();
    void bindEvents();

    void updateConfigChoices();

    void handleNotification(uint32 id) final;
};
