#include "editorwindow.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/editorwindow.cpp
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

#include <chrono>
#include <filesystem>
#include <thread>

#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/uri.h>

#include "config/misc/injection.hpp"
#include "config/presets/preset.hpp"
#include "ui/bitmap.hpp"
#include "ui/build.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/dialogs/progress.hpp"
#include "ui/helpers/busy.hpp"
#include "utils/defer.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/string.hpp"

#include "../tools/arduino.hpp"

EditorWindow::EditorWindow(wxWindow *parent, config::Info& info) : 
    pcui::Frame(
        parent,
        wxID_ANY,
        info.name()
    ),
    mInfo{info},
    mGeneralPage(*info.config()),
    mPropsPage(*info.config()),
    mPresetsPage(*info.config()),
    mBladesPage(*info.config()) {

    createMenuBar();
    createToolBar();

    bindEvents();

    wxCommandEvent event{wxEVT_MENU, ePage_General};
    event.SetInt(0);
    wxPostEvent(this, event);
}

EditorWindow::~EditorWindow() = default;

void EditorWindow::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evt) {
        evt.Skip();

        defer {
            if (not evt.GetVeto()) {
                pcui::cripple(this);
                mPresetsPage.deinit();
                mBladesPage.deinit();
                mInfo.unload();
            }
        };

        if (not evt.CanVeto()) return;

        data::Bool::ROContext isSaved{mInfo.config()->isSaved()};
        if (isSaved.val()) return;

        data::String::ROContext name{mInfo.name()};

        auto choice{pcui::showMessage(
            wxString::Format(_("\"%s\" Has Unsaved Changes"), name.val()),
            {
                .caption_=_("Close Editor"),
                .style_=wxICON_WARNING | wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT,
                .labels_={.yes_=_("Save Changes"), .no_=_("Discard Changes")},
                .parent_=this
            }
        )};

        if (choice == wxCANCEL or (choice == wxYES and not save())) {
            evt.Skip(false);
            evt.Veto();
        }
    });

    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        save();
    }, wxID_SAVE); 

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        const auto name{data::String::ROContext{mInfo.name()}.val()};
        wxFileDialog fileDlg(
            this,
            _("Export ProffieOS Config File"),
            wxEmptyString,
            name + config::RAW_FILE_EXTENSION,
            _("ProffieOS Configuration") + " (*.h)|*.h",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        );
        if (fileDlg.ShowModal() == wxID_CANCEL) return;

        config::generate(*mInfo.config(), fileDlg.GetPath().ToStdString());
    }, eID_Export);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        pcui::BusyTracker busy;

        auto *prog{new pcui::ProgressDialog(
            this,
            _("Verify Config"),
            true
        )};

        prog->set(0, _("Initializing..."));

        std::thread{[this, prog, busy]() {
            arduino::verifyConfig(*mInfo.config(), *prog);
        }}.detach();
    }, eID_Verify);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxFileDialog fileDialog{
            this,
            _("Select Injection File"),
            wxEmptyString,
            wxEmptyString,
            _("C Header") + " (*.h)|*.h",
            wxFD_FILE_MUST_EXIST | wxFD_OPEN
        };
        if (fileDialog.ShowModal() == wxCANCEL) return;

        auto dst{
            paths::injectionDir() / fileDialog.GetFilename().ToStdWstring()
        };

        std::error_code ec;
        const auto src{fileDialog.GetPath().ToStdWstring()};
        if (not files::copyOverwrite(src, dst, ec)) {
            pcui::showMessage(
                ec.message(),
                {.caption_=_("Injection file could not be added.")}
            );
            return;
        }

        auto& injectionVec{mInfo.config()->injections_};
        data::Vector::Context injections{injectionVec};
        injections.add(std::make_unique<config::Injection>(
            &injectionVec, fileDialog.GetFilename().ToStdString()
        ));
    }, eID_Add_Injection);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { 
        std::string styleStr;

        data::Selector::ROContext styleSel{mPresetsPage.styleSel()};
        data::Choice::ROContext styleChoice{mPresetsPage.styleSel().choice_};
        if (styleChoice.idx() != -1) {
            data::Vector::ROContext styleVec{*styleSel.bound()};

            auto& model{*styleVec.children()[styleChoice.idx()]};
            auto& style{static_cast<config::presets::Style&>(model)};

            data::String::ROContext content{style.content_};
            styleStr = content.val();

            utils::trimWhitespaceOutsideString(styleStr);
        }

        constexpr cstring EDITOR_URL{
            "https://fredrik.hubbe.net/lightsaber/style_editor.html?S="
        };

        wxURI uri{EDITOR_URL + styleStr};
        wxLaunchDefaultBrowser(uri.BuildURI());
    }, eID_Style_Editor);

    auto windowSelectionHandler{[this](wxCommandEvent& evt) {
#       ifdef __WXOSX__
        GetToolBar()->ToggleTool(evt.GetId(), false);
        GetToolBar()->OSXSelectTool(evt.GetId());
#       endif

        pcui::cripple(this);

        if (evt.GetId() == ePage_General) {
            pcui::build(this, mGeneralPage.ui());
        } else if (evt.GetId() == ePage_Props) {
            pcui::build(this, mPropsPage.ui());
        } else if (evt.GetId() == ePage_Presets) {
            pcui::build(this, mPresetsPage.ui());
        } else if (evt.GetId() == ePage_Blades) {
            pcui::build(this, mBladesPage.ui());
        }
    }};
    Bind(wxEVT_MENU, windowSelectionHandler, ePage_General);
    Bind(wxEVT_MENU, windowSelectionHandler, ePage_Props);
    Bind(wxEVT_MENU, windowSelectionHandler, ePage_Presets);
    Bind(wxEVT_MENU, windowSelectionHandler, ePage_Blades);

    /*
    Bind(wxEVT_IDLE, [this](wxIdleEvent& evt) {
        if (mStartMicros == -1) return;

        constexpr auto RESIZE_TIME_MICROS{300 * 1000};
        static std::chrono::microseconds::rep lastFrameMicros{0};

        const wxDisplay display{this};
        if (not display.IsOk()) return;

        auto frameRate{display.GetCurrentMode().GetRefresh()};
        // On Wayland (I assume because of course things don't work on Wayland) this doesn't work
        if (frameRate == 0) frameRate = 60;
        const std::chrono::microseconds::rep frameIntervalMicros{(1000 * 1000) / frameRate};

        const auto nowMicros{std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()};
        
        if (nowMicros - lastFrameMicros < frameIntervalMicros) {
            evt.RequestMore();
            return;
        }

        const float64 completion{std::clamp<float64>(
            static_cast<float64>(nowMicros - mStartMicros) / static_cast<float64>(RESIZE_TIME_MICROS),
            0, 1
        )};

        const auto totalDelta{mBestSize - mStartSize};

        const auto newSize{mStartSize + (totalDelta * completion)};
        SetSizeHints(newSize, newSize);
        SetSize(newSize);

        if (completion == 1) {
            configureResizing();
            mStartMicros = -1;
        } else {
            evt.RequestMore();
        }
    });
    */
}

void EditorWindow::createMenuBar() {
    auto *file{new wxMenu};
    file->Append(eID_Verify, _("Verify Config") + "\tCtrl+R");
    file->AppendSeparator();
    file->Append(wxID_SAVE, _("Save Config") + "\tCtrl+S");
    file->Append(eID_Export, _("Export Config...") + "\tCtrl+Alt+S");
    file->AppendSeparator();
    file->Append(
        eID_Add_Injection,
        _("Add Injection...") + "\tCtrl+Alt+I",
        _("Add a header file to be injected into CONFIG_PRESETS during compilation.")
    );

    auto *tools{new wxMenu};
    tools->Append(
        eID_Style_Editor,
        _("Style Editor..."),
        _("Open the ProffieOS style editor")
    );

    auto *menuBar{new wxMenuBar};
    menuBar->Append(file, _("&File"));
    menuBar->Append(tools, _("&Tools"));
    appendDefaultMenuItems(menuBar);

    SetMenuBar(menuBar);
}

void EditorWindow::createToolBar() {
    auto *toolbar{CreateToolBar(wxTB_TEXT)};
    toolbar->AddRadioTool(
        ePage_General,
        _("General"),
        pcui::Bitmap("settings").scaleTo(32).realize()
    );
    toolbar->AddRadioTool(
        ePage_Props,
        _("Prop File"),
        pcui::Bitmap("props").scaleTo(32).realize()
    );
    toolbar->AddRadioTool(
        ePage_Presets,
        _("Presets"),
        pcui::Bitmap("presets").scaleTo(32).realize()
    );
    toolbar->AddRadioTool(
        ePage_Blades,
        _("Blade Arrays"),
        pcui::Bitmap("blade").scale(32).realize()
    );

#   ifdef __WXMSW__
    toolbar->AddStretchableSpace();
#   endif
#   ifdef __WXOSX__
    toolbar->OSXSetSelectableTools(true);
#   endif

    toolbar->Realize();
}

bool EditorWindow::save() {
    auto err{mInfo.save()};
    if (err) {
        pcui::showMessage(
            *err,
            {
                .caption_=_("Config Not Saved"),
                .style_=wxOK | wxCENTER | wxICON_ERROR,
                .parent_=this
            }
        );
    }
    return not err;
}

/*
void EditorWindow::configureResizing() {
    if (not generalPage or not propsPage or not bladesPage or not presetsPage) return;

    if (generalPage->IsShown()) {
        SetSizeHints(GetSize(), GetSize());
    } else if (propsPage->IsShown()) {
        auto bestSize{GetBestSize()};
        SetSizeHints(-1, -1, -1, -1);
        propsPage->setToActualMinSize();
        SetSizeHints(GetBestSize(), bestSize);
    } else if (bladesPage->IsShown()) {
        SetSizeHints(GetSize(), {GetSize().x, -1});
    } else if (presetsPage->IsShown()) {
        SetSizeHints(GetSize(), {-1, -1});
    }
    SetVirtualSize(-1, -1);
}
*/

/*
void EditorWindow::fitAnimated() {
    if (not generalPage or not propsPage or not bladesPage or not presetsPage) return;

    SetSizeHints(-1, -1, -1, -1);

    const auto clientDelta{GetSize() - GetClientSize()};
    if (propsPage->IsShown()) {
        propsPage->setToActualBestSize();
    }

    mStartSize = GetSize();
    mBestSize = GetBestSize();
    SetVirtualSize(mBestSize - clientDelta);
    mStartMicros = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void EditorWindow::Fit() {
    SetSizeHints(-1, -1, -1, -1);
    pcui::Frame::Fit();

    configureResizing();
}
*/

