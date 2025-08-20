#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/pages/presetspage.h
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

#include "ui/static_box.h"

#include "../editorwindow.h"

class PresetsPage : public wxPanel, PCUI::NotifyReceiver {
public:
    PresetsPage(EditorWindow *);

    enum {
        ID_AddPreset,
        ID_RemovePreset,
        ID_MovePresetUp,
        ID_MovePresetDown,

        ID_AddArray,
        ID_RemoveArray,

        ID_RenameArray,
        ID_IssueButton,
        ID_WavText,
    };

private:
    EditorWindow *mParent{nullptr};

    PCUI::StaticBox *mInjectionsSizer;

    void createUI();
    void bindEvents();
    void handleNotification(uint32) override;

    void rebuildInjections();
};
