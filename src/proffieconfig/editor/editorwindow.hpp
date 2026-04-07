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

#include "config/config.hpp"
#include "ui/frame.hpp"
#include "ui/types.hpp"

#include "pages/general.hpp"
#include "pages/props.hpp"
#include "pages/presets.hpp"
#include "pages/blades.hpp"

struct EditorWindow : pcui::Frame {
    EditorWindow(wxWindow *, config::Info&);
    ~EditorWindow() override;

    // void Fit() final;
    // void fitAnimated();

    // Handles errors
    bool save();

    [[nodiscard]] config::Config& getOpenConfig() const;

private:
    pcui::DescriptorPtr ui();

    void createMenuBar();
    void createToolBar();

    void bindEvents();

    // void configureResizing();

    config::Info& mInfo;

    GeneralPage mGeneralPage;
    PropsPage mPropsPage;
    BladesPage mBladesPage;
    PresetsPage mPresetsPage;

    enum {
        ePage_General = 100,
        ePage_Props,
        ePage_Presets,
        ePage_Blades,

        eID_Export,
        eID_Verify,
        eID_Add_Injection,
        eID_Style_Editor,
    };

    wxSize mBestSize{-1, -1};
    wxSize mStartSize{-1, -1};
    std::chrono::microseconds::rep mStartMicros;
};

