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

#include <filesystem>
#include <thread>

#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/uri.h>

#include "config/misc/injection.hpp"
#include "config/presets/preset.hpp"
#include "data/context.hpp"
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

namespace {

ssize millis();

} // namespace

EditorWindow::EditorWindow(wxWindow *parent, config::Info& info) : 
    pcui::Frame(
        parent,
        wxID_ANY,
        // TODO: Add receiver and update name on change.
        data::context(info.name()).val()
    ),
    mInfo{info},
    mGeneralPage(*info.config()),
    mPropsPage(*info.config()),
    mPresetsPage(*info.config()),
    mBladesPage(*info.config()) {
    mAnimationTimer = new wxTimer(this);

    createMenuBar();
    createToolBar();

    bindEvents();

    // Immediately process this event to do initial UI build and setup toolbar
    // state.
    wxCommandEvent evt(wxEVT_MENU, ePage_General);
    ProcessEvent(evt);

    // Only start animations after initial setup has taken place.
    mAnimating = true;
}

EditorWindow::~EditorWindow() {
    delete mAnimationTimer;
}

void EditorWindow::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evt) {
        evt.Skip();

        defer {
            if (not evt.GetVeto()) {
                pcui::cripple(this);
                mGeneralPage.deinit();
                mPropsPage.deinit();
                mPresetsPage.deinit();
                mBladesPage.deinit();
                mInfo.unload();
            }
        };

        if (not evt.CanVeto()) return;

        auto isSaved{data::context(mInfo.config()->isSaved())};
        if (isSaved.val()) return;

        auto name{data::context(mInfo.name())};

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
        auto name{data::context(mInfo.name())};

        wxFileDialog fileDlg(
            this,
            _("Export ProffieOS Config File"),
            wxEmptyString,
            name.val() + config::RAW_FILE_EXTENSION,
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

        std::thread{[this, prog, busy]() {
            auto name{data::context(mInfo.name())};
            arduino::verifyConfig(name.val(), *mInfo.config(), *prog);
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

        auto& config{*mInfo.config()};
        config.injections_.append(std::make_unique<config::Injection>(
            config, fileDialog.GetFilename().ToStdString()
        ));
    }, eID_Add_Injection);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { 
        std::string styleStr;

        auto styleSel{data::context(mPresetsPage.styleSel())};
        auto styleChoice{data::context(mPresetsPage.styleSel().choice())};
        if (styleChoice.idx() != -1) {
            auto styleVec{data::context(*styleSel.bound())};

            auto& model{*styleVec.children()[styleChoice.idx()]};
            auto& style{dynamic_cast<config::presets::Style&>(model)};

            auto content{data::context(style.content_)};
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
        // For reasons I don't fully understand, probably because either myself
        // or wxWidgets is using the controls wrong, the selection gets weird
        // if this isn't done.
        GetToolBar()->ToggleTool(evt.GetId(), false);
        GetToolBar()->OSXSelectTool(evt.GetId());
#       endif
        
        // I'm somewhat proud that w/ the optimizations I added to wxWidgets
        // doing this redundantly isn't nearly as terrible as it used to be,
        // but even so, no sense in being wasteful...
        if (mCurrentPage == evt.GetId())
            return;

        pcui::cripple(this);

        // Set this first so it's available during the build.
        mCurrentPage = evt.GetId();

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

    Bind(wxEVT_TIMER, &EditorWindow::onTimer, this);
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

void EditorWindow::Fit() {
    // Stop a current animation from running anymore, if there is one.
    mAnimationTimer->Stop();

    wxDisplay display(this);
    if (not mAnimating or not display.IsOk()) {
        SetSizeHints(-1, -1, -1, -1);
        pcui::Frame::Fit();

        configureResizing();
        return;
    }
    
    // Have to free up the constraints so that GetBestSize() works properly.
    SetSizeHints(-1, -1, -1, -1);

    // For wxWidgets TLWs, GetBestSize is just the client size because it
    // queries GetWindowBorderSize() w/o taking into account TLW decoration.
    auto bestClientSize{GetBestSize()};
    mBestSize = ClientToWindowSize(bestClientSize);

    // A lot of the code in the underlying toolkits and wxWidgets is quite
    // pessimistic, honestly, and I think even in release things struggle to
    // hit 60fps (optimizations I'd like to continue working on, but in any
    // case...) sometimes, but try to handle higher refresh rates also.
    //
    // If things take too long there's handling to drop frames in onTimer().
    auto frameRate{display.GetCurrentMode().GetRefresh()};

    // On Wayland (I assume because of course things don't work on Wayland)
    // this doesn't work
    if (frameRate == 0)
        frameRate = 60;

    const auto frameIntervalMillis{1000 / frameRate};

    /*if (propsPage->IsShown()) {
        propsPage->setToActualBestSize();
    }*/

    // This actually does give the whole window size.
    mStartSize = GetSize();

    auto *panel{getUniqueChild()};

    // Set the panel size **and** position to where it should be once
    // done first so that GetBestSize() behaves correctly and it renders
    // in the right spot.
    panel->SetSize(
        0, 0,
        bestClientSize.x, bestClientSize.y
    );

    // To avoid lots of slow relayout, set the virtual size first,
    // explicitly perform a layout into that end size, and prevent
    // automatic layout during resize.
    panel->SetVirtualSize(panel->GetBestSize());
    panel->Layout();
    panel->SetAutoLayout(false);

    mAnimationCount = 0;
    mAnimationStartMillis = millis();
    mAnimationTimer->Start(frameIntervalMillis);
}

void EditorWindow::onTimer(wxTimerEvent& evt) {
    constexpr auto RESIZE_TIME_MILLIS{300};

    // The timer fires interval amount after when it is first called, so this
    // should be incremented first, treating the first frame as the size when
    // when the button was clicked.
    ++mAnimationCount;

    auto actualElapsed{millis() - mAnimationStartMillis};
    auto elapsed{static_cast<ssize>(mAnimationCount * evt.GetInterval())};

    // Drop the "frame," a prior one took too long.
    while (actualElapsed - elapsed > (ssize)evt.GetInterval()) {
        ++mAnimationCount;
        elapsed += evt.GetInterval();
    }

    const auto completion{std::clamp(
        static_cast<float64>(elapsed) / RESIZE_TIME_MILLIS,
        0.0, 1.0
    )};

    const auto totalDelta{mBestSize - mStartSize};
    const auto newSize{mStartSize + (totalDelta * completion)};

    // Set size hints to avoid the user being able to resize things here and
    // cause jank.
    SetSizeHints(newSize, newSize);
    SetSize(newSize);

    if (elapsed >= RESIZE_TIME_MILLIS) {
        mAnimationTimer->Stop();

        // Re-enable auto layout on resize, and limit resizing based on which
        // pane is shown.
        getUniqueChild()->SetAutoLayout(true);
        configureResizing();
    }
}

void EditorWindow::configureResizing() {
    switch (mCurrentPage) {
        case ePage_General:
            // Fixed size.
            SetSizeHints(GetSize(), GetSize());
            break;
        case ePage_Props:
            // Prevent from growing past max content area (or whereever the
            // display clamps the available space), but allow resizing within.
/*
            auto bestSize{GetBestSize()};
            SetSizeHints(-1, -1, -1, -1);
            propsPage->setToActualMinSize();
            SetSizeHints(GetBestSize(), bestSize);
*/
            break;
        case ePage_Presets:
        case ePage_Blades:
            // Allow increased size.
            SetSizeHints(GetSize(), {-1, -1});
            break;
        default:
    }
}

namespace {

ssize millis() {
    // lol, std::chrono
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

} // namespace

