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

#include "log/context.h"
#include "log/logger.h"
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

    sizer->SetMinSize(450, -1);
}

void EditorWindow::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event ) {
        if (!event.CanVeto()) {
            event.Skip();
            return;
        }

        if (not isSaved()) {
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
                    not Config::save(mConfig)
                ) {
                event.Veto();
                return;
            }
        }

        reinterpret_cast<MainMenu *>(GetParent())->removeEditor(this);
        event.Skip();
    });
    Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent& event) {
        auto& msgEvent{static_cast<Misc::MessageBoxEvent&>(event)};
        PCUI::showMessage(msgEvent.message, msgEvent.caption, msgEvent.style, this);
    }, wxID_ANY);

    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        Config::save(mConfig);
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

        generalPage->Show(evt.GetInt() == 0);
        propsPage->Show(evt.GetInt() == 1);
        bladesPage->Show(evt.GetInt() == 2);
        presetsPage->Show(evt.GetInt() == 3);

        if (bladesPage->AreAnyItemsShown()) {
            bladesPage->Fit(bladesPage->GetContainingWindow());
            bladesPage->Layout();
        }
        if (propsPage->AreAnyItemsShown()) propsPage->Layout();
        if (presetsPage->AreAnyItemsShown()) presetsPage->Layout();

        GetSizer()->Fit(this);

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

    propsPage->Show(false);
    bladesPage->Show(false);
    presetsPage->Show(false);

    sizer->Add(optionsSizer, wxSizerFlags(0).Expand());
    sizer->Add(generalPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(propsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(presetsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(bladesPage, wxSizerFlags(1).Border(wxALL, 10).Expand());

    SetSizerAndFit(sizer);
}


std::shared_ptr<Config::Config> EditorWindow::getOpenConfig() const { return mConfig; }

bool EditorWindow::isSaved() {
    auto& logger{Log::Context::getGlobal().createLogger("EditorWindow::isSaved()")};

    const auto currentPath{
        Paths::configs() / (static_cast<string>(mConfig->name) + ".h")
    };
    const auto validatePath{
        fs::temp_directory_path() / (static_cast<string>(mConfig->name) + "-validate")
    };

    auto dummyMessageHandler{[](wxCommandEvent&) {}};
    Bind(Misc::EVT_MSGBOX, dummyMessageHandler);
    auto res{Config::save(mConfig, validatePath)};
    wxYield();
    Unbind(Misc::EVT_MSGBOX, dummyMessageHandler);

    if (not res) {
        logger.warn("Config output failed");
        return false;
    }

    std::error_code err;
    const auto currentSize{fs::file_size(currentPath, err)};
    const auto validateSize{fs::file_size(validatePath, err)};
    if (currentSize != validateSize) {
        logger.warn(
            "File sizes do not match (" + 
            std::to_string(currentSize) + '/' + 
            std::to_string(validateSize) + ')'
        );
        return false;
    }

    std::ifstream current{currentPath};
    std::ifstream validate{validatePath};

    bool saved{true};
    while (current.good() && !current.eof() && validate.good() && !validate.eof()) {
        std::array<char, 4096> currentBuffer;
        std::array<char, currentBuffer.size()> validateBuffer;
        currentBuffer.fill(0);
        validateBuffer.fill(0);

        current.read(currentBuffer.data(), currentBuffer.size());
        validate.read(validateBuffer.data(), validateBuffer.size());

        if (0 != std::memcmp(currentBuffer.data(), validateBuffer.data(), validateBuffer.size())) {
            saved = false;
            break;
        }
    }

    current.close();
    validate.close();
    // fs::remove(validatePath);
    if (not saved) {
        logger.warn("File contents do not match");
    }
    return saved;
}

