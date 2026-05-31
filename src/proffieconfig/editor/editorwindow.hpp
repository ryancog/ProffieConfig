#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/editorwindow.hpp
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

#include <wx/event.h>
#include <wx/timer.h>

#include "config/config.hpp"
#include "data/receiver.hpp"
#include "ui/frame.hpp"

#include "pages/general.hpp"
#include "pages/props.hpp"
#include "pages/presets.hpp"
#include "pages/blades.hpp"
#include "dialogs/injections.hpp"

struct EditorWindow : pcui::Frame, data::Receiver {
    EditorWindow(wxWindow *, config::Info&);
    ~EditorWindow() override;

    void Fit() final;

    // Handles errors
    bool save();

private:
    void onActivate() override;
    void onDeactivate() override;

    void createMenuBar();
    void createToolBar();

    void bindEvents();

    void onIsSaved();
    void onCanUndo();
    void onCanRedo();

    void onClose(wxCloseEvent&);

    void onVerify(wxCommandEvent&);
    void onSave(wxCommandEvent&);
    void onExport(wxCommandEvent&);
    void onManageInjections(wxCommandEvent&);

    void onUndo(wxCommandEvent&);
    void onRedo(wxCommandEvent&);

    void onStyleEditor(wxCommandEvent&);

    void onPage(wxCommandEvent&);
    void onTimer(wxTimerEvent&);

    void configureResizing();

    config::Info& mInfo;

    GeneralPage mGeneralPage;
    PropsPage mPropsPage;
    BladesPage mBladesPage;
    PresetsPage mPresetsPage;
    InjectionsDlg *mInjectionDlg{nullptr};

    enum {
        ePage_General = wxID_HIGHEST,
        ePage_Props,
        ePage_Presets,
        ePage_Blades,

        eID_Props_Scrolled,

        eID_Export,
        eID_Verify,
        eID_Verify_Cacheless,
        eID_Injections,
        eID_Style_Editor,

        ePage_First = ePage_General,
        ePage_Last = ePage_Blades,
    };

    size mCurrentPage{0};

    bool mAnimating{false};
    wxTimer *mAnimationTimer;
    size mAnimationCount;
    ssize mAnimationStartMillis;

    wxSize mBestSize{-1, -1};
    wxSize mStartSize{-1, -1};
};

