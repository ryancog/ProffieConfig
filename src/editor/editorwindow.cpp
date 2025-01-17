// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "editor/editorwindow.h"

#include "editor/pages/bladespage.h"
#include "editor/pages/generalpage.h"
#include "editor/pages/presetspage.h"
#include "editor/pages/propspage.h"

#include "core/config/settings.h"
#include "core/config/configuration.h"
#include "core/defines.h"
#include "core/utilities/misc.h"
#include "core/utilities/progress.h"

#include "tools/arduino.h"

#include <wx/event.h>
#include <wx/combobox.h>
#include <wx/arrstr.h>
#ifdef __WINDOWS__
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif
#include <wx/statbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/tooltip.h>
#include <wx/menu.h>

EditorWindow::EditorWindow(const std::string& _configName, wxWindow* parent) : wxFrame(parent, wxID_ANY, "ProffieConfig Editor - " + _configName, wxDefaultPosition, wxDefaultSize), openConfig(_configName) {
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

        auto saved{isSaved()};
        if (saved or wxMessageDialog(
                    this,
                    "Are you sure you want to close the editor?\n\n"
                    "You currently have unsaved changes which will be lost!",
                    "Close ProffieConfig Editor",
                    wxICON_WARNING | wxYES_NO | wxNO_DEFAULT)
                .ShowModal() == wxID_YES) {
            reinterpret_cast<MainMenu *>(GetParent())->removeEditor(this);
            event.Skip();
            return;
        }
        event.Veto();
    });
    Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent &event) {
        wxMessageDialog(this, ((Misc::MessageBoxEvent*)&event)->message, ((Misc::MessageBoxEvent*)&event)->caption, ((Misc::MessageBoxEvent*)&event)->style).ShowModal();
    }, wxID_ANY);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::outputConfig(CONFIG_DIR + openConfig + ".h", this); }, ID_SaveConfig);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Configuration::exportConfig(this); }, ID_ExportConfig);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Arduino::verifyConfig(this, this); }, ID_VerifyConfig);

# if defined(__WXOSX__)
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(wxGetCwd() + std::string("/" STYLEEDIT_PATH)); }, ID_StyleEditor);
# else
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser(STYLEEDIT_PATH); }, ID_StyleEditor);
#endif

    Bind(wxEVT_CHOICE, [&](wxCommandEvent&) {
        generalPage->Show(windowSelect->entry()->GetStringSelection() == "General");
        propsPage->Show(windowSelect->entry()->GetStringSelection() == "Prop File");
        bladesPage->Show(windowSelect->entry()->GetStringSelection() == "Blade Arrays");
        presetsPage->Show(windowSelect->entry()->GetStringSelection() == "Presets And Styles");

        //generalPage->update();
        bladesPage->update();
        propsPage->update();
        propsPage->updateSizeAndLayout();
        presetsPage->update();

        if (bladesPage->AreAnyItemsShown()) {
            bladesPage->Fit(bladesPage->GetContainingWindow());
            bladesPage->Layout();
        }
        if (propsPage->AreAnyItemsShown()) propsPage->Layout();
        if (presetsPage->AreAnyItemsShown()) presetsPage->Layout();
        FULLUPDATEWINDOW(this);
    }, ID_WindowSelect);
}
void EditorWindow::createToolTips() {
}

void EditorWindow::createMenuBar() {
    wxMenu *file = new wxMenu;
    file->Append(ID_VerifyConfig, "Verify Config\tCtrl+R");
    file->AppendSeparator();
    file->Append(ID_SaveConfig, "Save Config\tCtrl+S");
    file->Append(ID_ExportConfig, "Export Config...\t");

    wxMenu* tools = new wxMenu;
    tools->Append(ID_StyleEditor, "Style Editor...", "Open the ProffieOS style editor");

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(file, "&File");
    menuBar->Append(tools, "&Tools");
    SetMenuBar(menuBar);
}

void EditorWindow::createPages() {
    sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* options = new wxBoxSizer(wxHORIZONTAL);
    windowSelect = new pcChoice(this, ID_WindowSelect, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"General", "Prop File", "Blade Arrays", "Presets And Styles"}));
    options->Add(windowSelect, wxSizerFlags(0).Border(wxALL, 10));

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

    sizer->Add(options, wxSizerFlags(0).Expand());
    sizer->Add(generalPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(propsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(presetsPage, wxSizerFlags(1).Border(wxALL, 10).Expand());
    sizer->Add(bladesPage, wxSizerFlags(1).Border(wxALL, 10).Expand());

    SetSizerAndFit(sizer);
}


std::string_view EditorWindow::getOpenConfig() const { return openConfig; }

bool EditorWindow::isSaved() const {
    const auto currentPath{CONFIG_DIR + openConfig + ".h"};
    const auto validatePath{CONFIG_DIR + openConfig + "-validate"};
    Configuration::outputConfig(validatePath, this);
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
    wxRemove(validatePath);
    return saved;
}

