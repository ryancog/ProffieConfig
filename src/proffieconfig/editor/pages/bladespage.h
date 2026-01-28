#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/pages/bladespage.h
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
#include "../dialogs/awarenessdlg.h"

class ArrayEditDlg;

class BladesPage : public wxPanel, pcui::NotifyReceiver {
public:
    BladesPage(EditorWindow *);

private:
    EditorWindow *mParent{nullptr};

    enum {
        ID_OpenBladeAwareness = 2,

        ID_IssueIcon,

        ID_EditArray,
        ID_AddArray,
        ID_RemoveArray,

        ID_AddBlade,
        ID_RemoveBlade,
        ID_AddSplit,
        ID_RemoveSplit,

        ID_NoSelectText,
        ID_PinNameAdd,
    };

    BladeAwarenessDlg *mAwarenessDlg{nullptr};

    void bindEvents();
    void handleNotification(uint32) final;

    wxSizer *createBladeSelect();
    wxSizer *createBladeSettings();
   
    wxSizer *mSimpleSizer;
    wxSizer *mPixelSizer;
};
