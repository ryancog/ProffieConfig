#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/buttons.h
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

#include <wx/dialog.h>
#include <wx/panel.h>

#include "ui/notifier.h"
#include "ui/static_box.h"

#include "../editorwindow.h"

class ButtonsDlg : public wxDialog, PCUI::NotifyReceiver {
public:
    ButtonsDlg(EditorWindow*);

private:
    void handleNotification(uint32) final;

    EditorWindow *mParent;

    enum {
        ID_AddButton,
    };

    wxScrolledWindow *mButtonsArea{nullptr};
    wxButton *mAddButton{nullptr};

    class ButtonPanel;

    void bindEvents();
    void createUI();
    void createButtonsArea();

    wxBoxSizer *header();
    static wxWindow *info(wxWindow*);
};

class ButtonsDlg::ButtonPanel : public PCUI::StaticBox {
public:
    ButtonPanel(
        wxScrolledWindow *,
        Config::Config&,
        Config::Settings::ButtonData&
    );
};
