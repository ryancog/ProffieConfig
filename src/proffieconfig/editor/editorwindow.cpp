#include "editorwindow.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

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

#include "utils/paths.h"
#include "ui/message.h"
#include "utils/defer.h"
#include "../core/config/settings.h"
#include "../core/config/configuration.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"

#include "../tools/arduino.h"

EditorWindow::EditorWindow(const string& _configName, wxWindow* parent) : PCUI::Frame(parent, wxID_ANY, "ProffieConfig Editor - " + _configName, wxDefaultPosition, wxDefaultSize), mOpenConfig(_configName) {
    createMenuBar();
    createPages();
    bindEvents();
    createToolTips();
    settings = new Settings(this);

# ifdef __WINDOWS__
    SetIcon( wxICON(IDI_ICON1) );
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif
    sizer->SetMinSize(450, -1);
}

EditorWindow::~EditorWindow() {
    delete settings;
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
                "You currently have unsaved changes which will be lost otherwise.",
                "Save Changes to \"" + mOpenConfig + "\"?",
                flags
            };
            saveDialog.SetYesNoCancelLabels("Save Changes", "Discard Changes", "Cancel");
            auto saveChoice{saveDialog.ShowModal()};

            if (saveChoice == wxID_YES) {
                if (not Configuration::outputConfig(Paths::configs() / (mOpenConfig + ".h"), this)) {
                    event.Veto();
                    return;
                }
            } else if (saveChoice == wxID_CANCEL) {
                event.Veto();
                return;
            }
        }

        reinterpret_cast<MainMenu *>(GetParent())->removeEditor(this);
        event.Skip();
    });
    Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent &event) {
            PCUI::showMessage(((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style, this);
    }, wxID_ANY);

    Bind(wxEVT_MENU, [this](wxCommandEvent&) { Configuration::outputConfig(Paths::configs() / (mOpenConfig + ".h"), this); }, wxID_SAVE); 

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::exportConfig(this); }, ID_ExportConfig);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(this, this); }, ID_VerifyConfig);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxFileDialog fileDialog{this, "Select Injection File", wxEmptyString, wxEmptyString, "C Header (*.h)|*.h", wxFD_FILE_MUST_EXIST | wxFD_OPEN};
        if (fileDialog.ShowModal() == wxCANCEL) return;

        auto copyPath{Paths::injections() / fileDialog.GetFilename().ToStdWstring()};
        const auto copyOptions{fs::copy_options::overwrite_existing};
        std::error_code err;
        if (not fs::copy_file(fileDialog.GetPath().ToStdWstring(), copyPath, copyOptions, err)) {
            PCUI::showMessage(err.message(), "Injection file could not be added.");
            return;
        }

        presetsPage->injections.push_back(fileDialog.GetFilename().ToStdString());
        presetsPage->update();
    }, ID_AddInjection);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("http://profezzorn.github.io/ProffieOS-StyleEditor/style_editor.html"); }, ID_StyleEditor);

    Bind(wxEVT_CHOICE, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        generalPage->Show(windowSelect->entry()->GetStringSelection() == "General");
        propsPage->Show(windowSelect->entry()->GetStringSelection() == "Prop File");
        bladesPage->Show(windowSelect->entry()->GetStringSelection() == "Blade Arrays");
        presetsPage->Show(windowSelect->entry()->GetStringSelection() == "Presets And Styles");

        //generalPage->update();
        bladesPage->update();
        propsPage->update();
        presetsPage->update();

        if (bladesPage->AreAnyItemsShown()) {
            bladesPage->Fit(bladesPage->GetContainingWindow());
            bladesPage->Layout();
        }
        if (propsPage->AreAnyItemsShown()) propsPage->Layout();
        if (presetsPage->AreAnyItemsShown()) presetsPage->Layout();

        sizer->Fit(this);
    }, ID_WindowSelect);
}
void EditorWindow::createToolTips() {
}

void EditorWindow::createMenuBar() {
    auto *file{new wxMenu};
    file->Append(ID_VerifyConfig, "Verify Config\tCtrl+R");
    file->AppendSeparator();
    file->Append(wxID_SAVE, "Save Config\tCtrl+S");
    file->Append(ID_ExportConfig, "Export Config...\t");
    file->AppendSeparator();
    file->Append(ID_AddInjection, "Add Injection...\t", "Add a header file to be injected into CONFIG_PRESETS during compilation.");

    auto *tools{new wxMenu};
    tools->Append(ID_StyleEditor, "Style Editor...", "Open the ProffieOS style editor");

    auto *menuBar{new wxMenuBar};
    menuBar->Append(file, "&File");
    menuBar->Append(tools, "&Tools");
    SetMenuBar(menuBar);
}

void EditorWindow::createPages() {
    sizer = new wxBoxSizer(wxVERTICAL);

    auto* optionsSizer{new wxBoxSizer(wxHORIZONTAL)};
    windowSelect = new PCUI::Choice(this, ID_WindowSelect, Misc::createEntries({"General", "Prop File", "Blade Arrays", "Presets And Styles"}));
    optionsSizer->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));

    generalPage = new GeneralPage(this);
    propsPage = new PropsPage(this);
    presetsPage = new PresetsPage(this);
    bladesPage = new BladesPage(this);

    //generalPage->update();
    propsPage->update();
    presetsPage->update();
    bladesPage->update();

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


string_view EditorWindow::getOpenConfig() const { return mOpenConfig; }

void EditorWindow::renameConfig(const string& name) {
    fs::rename(Paths::configs() / (mOpenConfig + ".h"), Paths::configs() / (name + ".h"));
    mOpenConfig = name;
    // TODO: 
}

bool EditorWindow::isSaved() {
    const auto currentPath{Paths::configs() / (mOpenConfig + ".h")};
    const auto validatePath{fs::temp_directory_path() / (mOpenConfig + "-validate")};
    if (not Configuration::outputConfig(validatePath, this)) {
        return false;
    }

    std::error_code err;
    if (fs::file_size(currentPath, err) != fs::file_size(validatePath, err)) {
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
    fs::remove(validatePath);
    return saved;
}

