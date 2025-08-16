#include "editorwindow.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/editor/editorwindow.cpp
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

#include <filesystem>
#include <fstream>
#include <thread>

#include <wx/arrstr.h>
#include <wx/combobox.h>
#include <wx/event.h>
#include <wx/filedlg.h>
#include <wx/gdicmn.h>
#include <wx/list.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/string.h>
#include <wx/toolbar.h>
#include <wx/tooltip.h>

#include "utils/paths.h"
#include "ui/message.h"
#include "ui/frame.h"
#include "utils/defer.h"
#include "utils/image.h"

#include "pages/bladespage.h"
#include "pages/generalpage.h"
#include "pages/presetspage.h"
#include "pages/propspage.h"

#include "../mainmenu/mainmenu.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"

#include "../tools/arduino.h"

EditorWindow::EditorWindow(wxWindow *parent, Config::Config& config) : 
    PCUI::Frame(
        parent,
        wxID_ANY,
        /* _("ProffieConfig Editor") + */ static_cast<string>(config.name)
    ),
    mConfig{config},
    mInitialOSVersion{config.settings.getOSVersion()} {
    Notifier::create(this, mNotifyData);
    auto *sizer{new wxBoxSizer{wxVERTICAL}};

    createMenuBar();
    createUI(sizer);
    bindEvents();
    initializeNotifier();

    wxCommandEvent event{wxEVT_MENU, ID_General};
    event.SetInt(0);
    wxPostEvent(this, event);

    SetSizerAndFit(sizer);
}

bool EditorWindow::Destroy() {
    mConfig.close();
    return PCUI::Frame::Destroy();
}

void EditorWindow::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event ) {
        if (not event.CanVeto()) {
            event.Skip();
            return;
        }

        if (not mConfig.isSaved()) {
#           ifdef __WINDOWS__
            const auto flags{static_cast<long>(wxICON_WARNING | wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT)};
            wxGenericMessageDialog saveDialog{
#           else
            const auto flags{wxICON_WARNING | wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT};
            wxMessageDialog saveDialog{
#           endif
                this,
                _("You currently have unsaved changes which will be lost otherwise."),
                wxString::Format(
                    _("Save Changes to \"%s\"?"),
                    static_cast<string>(mConfig.name)
                ),
                flags
            };
            saveDialog.SetYesNoCancelLabels(
                _("Save Changes"),
                _("Discard Changes"),
                _("Cancel")
            );
            auto saveChoice{saveDialog.ShowModal()};

            if (saveChoice == wxID_CANCEL or (saveChoice == wxID_YES and not save())) {
                event.Veto();
                return;
            }
        }

        if (not mInitialOSVersion.err and mInitialOSVersion != mConfig.settings.getOSVersion()) {
#           ifdef __WINDOWS__
            const auto flags{static_cast<long>(wxICON_INFORMATION | wxYES_NO | wxNO_DEFAULT)};
            wxGenericMessageDialog saveDialog{
#           else
            const auto flags{wxICON_INFORMATION | wxYES_NO | wxNO_DEFAULT};
            wxMessageDialog osChangeDlg{
#           endif
                this,
                _("Any settings no longer active in this ProffieOS version are not saved "
                        "once the editor is closed.\n\nAre you sure you want to continue?"),
                _("OS Version Changed"),
                flags
            };
            osChangeDlg.SetYesNoLabels(
                _("Continue"),
                _("Not Yet")
            );
            if (osChangeDlg.ShowModal() != wxID_YES) {
                event.Veto();
                return;
            }
        }

        reinterpret_cast<MainMenu *>(GetParent())->removeEditor(this);
        event.Skip();
    });
    Bind(Progress::EVT_UPDATE, [&](ProgressEvent& event) { 
        Progress::handleEvent(&event); 
    });
    Bind(Misc::EVT_MSGBOX, [&](Misc::MessageBoxEvent& evt) {
        PCUI::showMessage(evt.message, evt.caption, evt.style, this);
    }, wxID_ANY);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        save();
    }, wxID_SAVE); 
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        // TODO: Get the filepath for this
        // Config::save(mConfig, filepath); 
    }, ID_ExportConfig);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);

        auto *prog{new Progress(this)};
        const auto verifyStr{_("Verify Config")};
        prog->SetTitle(verifyStr);
        prog->Update(0, _("Initializing..."));

        std::thread{[this, prog, verifyStr]() {
            Defer defer{[this]() { mNotifyData.notify(ID_AsyncDone); }};

            auto res{Arduino::verifyConfig(mConfig, prog)};
            if (auto *err = std::get_if<string>(&res)) {
                auto *evt{new Misc::MessageBoxEvent(
                    Misc::EVT_MSGBOX, wxID_ANY, *err, _("Config Verification Failed")
                )};
                wxQueueEvent(this, evt);
                return;
            }
            auto result{std::get<Arduino::Result>(res)};

            wxString message{_("Config Verified Successfully!")};
            if (result.total != -1) {
                message += "\n\n";
                message += wxString::Format(
                    wxGetTranslation(Arduino::Result::USAGE_MESSAGE),
                    result.percent(),
                    result.used,
                    result.total
                );
            } 

            auto *evt{new Misc::MessageBoxEvent(
                Misc::EVT_MSGBOX, wxID_ANY, message, verifyStr
            )};
            wxQueueEvent(this, evt);
        }}.detach();
    }, ID_VerifyConfig);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxFileDialog fileDialog{
            this,
            _("Select Injection File"),
            wxEmptyString,
            wxEmptyString,
            "C Header (*.h)|*.h",
            wxFD_FILE_MUST_EXIST | wxFD_OPEN
        };
        if (fileDialog.ShowModal() == wxCANCEL) return;

        auto copyPath{Paths::injectionDir() / fileDialog.GetFilename().ToStdWstring()};
        const auto copyOptions{fs::copy_options::overwrite_existing};
        std::error_code err;
        if (not fs::copy_file(fileDialog.GetPath().ToStdWstring(), copyPath, copyOptions, err)) {
            PCUI::showMessage(err.message(), _("Injection file could not be added."));
            return;
        }

        mConfig.presetArrays.addInjection(fileDialog.GetFilename().ToStdString());
    }, ID_AddInjection);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { 
        wxLaunchDefaultBrowser("http://profezzorn.github.io/ProffieOS-StyleEditor/style_editor.html");
    }, ID_StyleEditor);
    auto windowSelectionHandler{[this](wxCommandEvent& evt) {
        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

#       ifdef __WXOSX__
        GetToolBar()->ToggleTool(evt.GetId(), false);
#       endif
        generalPage->Show(evt.GetId() == ID_General);
        propsPage->Show(evt.GetId() == ID_Props);
        bladesPage->Show(evt.GetId() == ID_BladeArrays);
        presetsPage->Show(evt.GetId() == ID_Presets);

        Layout();
        Fit();
    }};
    Bind(wxEVT_MENU, windowSelectionHandler, ID_General);
    Bind(wxEVT_MENU, windowSelectionHandler, ID_Props);
    Bind(wxEVT_MENU, windowSelectionHandler, ID_Presets);
    Bind(wxEVT_MENU, windowSelectionHandler, ID_BladeArrays);
}

void EditorWindow::handleNotification(uint32 id) {
    if (id == ID_AsyncDone) wxSetCursor(wxNullCursor);
}

void EditorWindow::createMenuBar() {
    auto *file{new wxMenu};
    file->Append(ID_VerifyConfig, _("Verify Config\tCtrl+R"));
    file->AppendSeparator();
    file->Append(wxID_SAVE, _("Save Config\tCtrl+S"));
    file->Append(ID_ExportConfig, _("Export Config..."));
    file->AppendSeparator();
    file->Append(
        ID_AddInjection,
        _("Add Injection..."),
        _("Add a header file to be injected into CONFIG_PRESETS during compilation.")
    );

    auto *tools{new wxMenu};
    tools->Append(
        ID_StyleEditor,
        _("Style Editor..."),
        _("Open the ProffieOS style editor")
    );

    auto *menuBar{new wxMenuBar};
    menuBar->Append(file, _("&File"));
    menuBar->Append(tools, _("&Tools"));
    SetMenuBar(menuBar);
}

void EditorWindow::createUI(wxSizer *sizer) {
    auto *toolbar{CreateToolBar(wxTB_TEXT)};
    toolbar->AddRadioTool(
        ID_General,
        _("General"),
        Image::loadPNG("settings", wxDefaultSize)
    );
    toolbar->AddRadioTool(
        ID_Props,
        _("Prop File"),
        Image::loadPNG("props", wxDefaultSize)
    );
    toolbar->AddRadioTool(
        ID_Presets,
        _("Presets"),
        Image::loadPNG("presets", wxDefaultSize)
    );
    toolbar->AddRadioTool(
        ID_BladeArrays,
        _("Blade Arrays"),
        Image::loadPNG("blade", wxDefaultSize)
    );
#   ifdef __WXMSW__
    toolbar->AddStretchableSpace();
#   endif
#   ifdef __WXOSX__
    toolbar->OSXSetSelectableTools(true);
#   endif
    toolbar->Realize();
#   ifdef __WXOSX__
    toolbar->OSXSelectTool(ID_General);
#   endif

    generalPage = new GeneralPage(this);
    propsPage = new PropsPage(this);
    presetsPage = new PresetsPage(this);
    bladesPage = new BladesPage(this);

    sizer->Add(
        generalPage, 
        wxSizerFlags(1).Border(wxALL, 20).Expand()
    );
    sizer->Add(
        propsPage,
        wxSizerFlags(1).Border(wxALL, 20).Expand()
    );
    sizer->Add(
        presetsPage,
        wxSizerFlags(1).Border(wxALL, 20).Expand()
    );
    sizer->Add(
        bladesPage,
        wxSizerFlags(1).Border(wxALL, 20).Expand()
    );
}

bool EditorWindow::save() {
    auto err{mConfig.save()};
    if (err) {
        PCUI::showMessage(*err, _("Config Not Saved"), wxOK | wxCENTER | wxICON_ERROR, this);
    }
    return not err;
}

void EditorWindow::Fit() {
    SetSizeHints(-1, -1, -1, -1);
    PCUI::Frame::Fit();
    if (not generalPage or not propsPage or not bladesPage or not presetsPage) return;

    SetMinSize(GetSize());
    if (generalPage->IsShown() or propsPage->IsShown()) {
        SetMaxSize(GetSize());
    } else if (bladesPage->IsShown()) {
        SetMaxSize({GetSize().x, -1});
    } else if (presetsPage->IsShown()) {
        SetMaxSize({-1, -1});
    }
}

Config::Config& EditorWindow::getOpenConfig() const { return mConfig; }



