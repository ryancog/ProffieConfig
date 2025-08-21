#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/pages/generalpage.h
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

#include "../editorwindow.h"
#include "../dialogs/customoptionsdlg.h"

class GeneralPage : public wxPanel, PCUI::NotifyReceiver {
public:
    GeneralPage(EditorWindow*);

    enum {
        ID_CustomOptions = 2,
    };

private:
    EditorWindow* mParent{nullptr};
    CustomOptionsDlg *mCustomOptDlg{nullptr};

    void bindEvents();
    void handleNotification(uint32) final;

    wxWindow *setupSection();

    wxWindow *miscSection();
    wxWindow *installationSection();
    wxWindow *tweaksSection();
    wxWindow *editingSection();
    wxWindow *audioSection();
};
