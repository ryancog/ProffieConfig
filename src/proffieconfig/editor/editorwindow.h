#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/editorwindow.h
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <wx/frame.h>
#include <wx/sizer.h>

#include "config/config.h"
#include "ui/frame.h"

// Forward declarations to get around circular dependencies
class GeneralPage;
class PropsPage;
class BladesPage;
class PresetsPage;

class EditorWindow : public PCUI::Frame, PCUI::NotifyReceiver {
public:
    EditorWindow(wxWindow *, Config::Config&);
    bool Destroy() final;
    void Fit() final;
    void FitAnimated();

    // Handles errors
    bool save();

    [[nodiscard]] Config::Config& getOpenConfig() const;

    GeneralPage *generalPage{nullptr};
    PropsPage *propsPage{nullptr};
    BladesPage *bladesPage{nullptr};
    PresetsPage *presetsPage{nullptr};

    enum {
        ID_General = 2,
        ID_Props,
        ID_Presets,
        ID_BladeArrays,

        ID_ExportConfig,
        ID_VerifyConfig,
        ID_AddInjection,

        ID_StyleEditor,

        ID_AsyncDone,
    };

private:
    void createMenuBar();
    void createUI(wxSizer *);
    void bindEvents();

    wxSize mBestSize{-1, -1};
    wxSize mStartSize{-1, -1};
    std::chrono::microseconds::rep mStartMicros;

    void configureResizing();

    void handleNotification(uint32) final;

    PCUI::Notifier mNotifyData;
    Config::Config& mConfig;

    const Utils::Version mInitialOSVersion;
};
