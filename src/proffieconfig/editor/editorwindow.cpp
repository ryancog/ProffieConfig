#include "editorwindow.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include <filesystem>
#include <fstream>

#include <wx/filedlg.h>
#include <wx/event.h>
#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/statbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tooltip.h>
#include <wx/menu.h>

#include "pages/bladespage.h"
#include "pages/generalpage.h"
#include "pages/presetspage.h"
#include "pages/propspage.h"

#include "paths/paths.h"
#include "ui/message.h"
#include "utils/defer.h"

#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"

#include "../tools/arduino.h"
#include "wx/gdicmn.h"

EditorWindow::EditorWindow(wxWindow *parent, std::shared_ptr<Config::Config> config) : 
    PCUI::Frame(
        parent,
        wxID_ANY,
        _("ProffieConfig Editor") + " - " + static_cast<string>(config->name)
    ),
    mConfig{std::move(config)} {
    auto *sizer{new wxBoxSizer{wxVERTICAL}};

    createMenuBar();
    createPages(sizer);
    bindEvents();
}

void EditorWindow::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event ) {
        if (!event.CanVeto()) {
            event.Skip();
            return;
        }

        if (not mConfig->isSaved()) {
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
                    static_cast<string>(mConfig->name)
                ),
                flags
            };
            saveDialog.SetYesNoCancelLabels(
                _("Save Changes"),
                _("Discard Changes"),
                _("Cancel")
            );
            auto saveChoice{saveDialog.ShowModal()};

            if (
                    saveChoice == wxID_CANCEL or 
                    not save()
                ) {
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
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) {
        auto& msgEvent{static_cast<Misc::MessageBoxEvent&>(event)};
        PCUI::showMessage(msgEvent.message, msgEvent.caption, msgEvent.style, this);
    }, wxID_ANY);

    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        save();
    }, wxID_SAVE); 

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        // TODO: Get the filepath for this
        // Config::save(mConfig, filepath); 
    }, ID_ExportConfig);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(this, this); }, ID_VerifyConfig);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxFileDialog fileDialog{this, _("Select Injection File"), wxEmptyString, wxEmptyString, "C Header (*.h)|*.h", wxFD_FILE_MUST_EXIST | wxFD_OPEN};
        if (fileDialog.ShowModal() == wxCANCEL) return;

        auto copyPath{Paths::injections() / fileDialog.GetFilename().ToStdWstring()};
        const auto copyOptions{fs::copy_options::overwrite_existing};
        std::error_code err;
        if (not fs::copy_file(fileDialog.GetPath().ToStdWstring(), copyPath, copyOptions, err)) {
            PCUI::showMessage(err.message(), _("Injection file could not be added."));
            return;
        }

        mConfig->presetArrays.addInjection(fileDialog.GetFilename().ToStdString());
    }, ID_AddInjection);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { 
        wxLaunchDefaultBrowser("http://profezzorn.github.io/ProffieOS-StyleEditor/style_editor.html");
    }, ID_StyleEditor);
    Bind(wxEVT_CHOICE, [this](wxCommandEvent& evt) {
        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        generalPage->GetStaticBox()->Show(evt.GetInt() == 0);
        propsPage->GetStaticBox()->Show(evt.GetInt() == 1);
        bladesPage->GetStaticBox()->Show(evt.GetInt() == 2);
        presetsPage->GetStaticBox()->Show(evt.GetInt() == 3);

        Layout();
        SetSizerAndFit(GetSizer());
    }, ID_WindowSelect);
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

void EditorWindow::createPages(wxSizer *sizer) {
    auto* optionsSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *windowSelect{new wxChoice(
        this,
        ID_WindowSelect,
        wxDefaultPosition,
        wxDefaultSize,
        {
            _("General"),
            _("Prop File"),
            _("Blade Arrays"),
            _("Presets And Styles"),
        }
    )};

    optionsSizer->Add(
        windowSelect,
        wxSizerFlags(0).Border(wxALL, 10)
    );

    generalPage = new GeneralPage(this);
    propsPage = new PropsPage(this);
    presetsPage = new PresetsPage(this);
    bladesPage = new BladesPage(this);

    propsPage->GetStaticBox()->Show(false);
    bladesPage->GetStaticBox()->Show(false);
    presetsPage->GetStaticBox()->Show(false);

    sizer->Add(optionsSizer, wxSizerFlags(0).Expand());
    sizer->Add(generalPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(propsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(presetsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(bladesPage, wxSizerFlags(1).Border(wxALL, 10).Expand());

    SetSizerAndFit(sizer);
}

bool EditorWindow::save() {
    return mConfig->save();
}

std::shared_ptr<Config::Config> EditorWindow::getOpenConfig() const { return mConfig; }

