#include "mainmenu.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <fstream>

#include <wx/aboutdlg.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/toplevel.h>
#include <wx/utils.h>

#include "ui/controls.h"
#include "ui/message.h"
#include "ui/plaque.h"
#include "ui/frame.h"
#include "utils/defer.h"
#include "utils/paths.h"
#include "utils/image.h"
#include "dialogs/addconfig.h"

#include "../core/defines.h"
#include "../core/appstate.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"
#include "../core/config/configuration.h"
#include "../editor/editorwindow.h"
#include "../onboard/onboard.h"
#include "../tools/arduino.h"
#include "../tools/serialmonitor.h"
#include "../mainmenu/dialogs/props.h"


MainMenu* MainMenu::instance{nullptr};
MainMenu::MainMenu(wxWindow* parent) : 
    PCUI::Frame(parent, wxID_ANY, "ProffieConfig", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    createUI();
    createMenuBar();
    createTooltips();
    bindEvents();
    update();

    Show(true);
}

void MainMenu::bindEvents() {
    auto promptClose{[this]() -> bool {
        for (auto *editor : editors) {
            if (not editor->isSaved()) {
                if (PCUI::showMessage(
                            _("There is at least one editor open with unsaved changes, are you sure you want to exit?") +
                            "\n\n" +
                            _("All unsaved changes will be lost!"),
                            _("Open Editor(s)"),
                            wxYES_NO | wxNO_DEFAULT | wxCENTER | wxICON_EXCLAMATION) == wxNO) {
                    return false;
                }

                break;
            }
        }

        return true;
    }};

    Bind(wxEVT_CLOSE_WINDOW, [promptClose](wxCloseEvent& event) {
        AppState::saveState();
        if (event.CanVeto() and not promptClose()) {
            event.Veto();
        }
        event.Skip();
    });
    Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent &event) {
        const auto& msgEvent{static_cast<Misc::MessageBoxEvent&>(event)};
        PCUI::showMessage(msgEvent.message, msgEvent.caption, msgEvent.style, this);
    }, wxID_ANY);
    Bind(wxEVT_MENU, [this, promptClose](wxCommandEvent&) { 
        if (not promptClose()) return;
        Close(true); 
        OnboardFrame::instance = new OnboardFrame(); 
    }, ID_ReRunSetup);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxAboutDialogInfo aboutInfo;
        aboutInfo.SetDescription(
                _("All-in-one Proffieboard Management Utility") +
                "\n\n"
                "ProffieOS v" wxSTRINGIZE(PROFFIEOS_VERSION) " | Arduino CLI v" wxSTRINGIZE(ARDUINO_CLI_VERSION)
                );
        aboutInfo.SetVersion(wxSTRINGIZE(EXEC_VERSION));
        aboutInfo.SetWebSite("https://proffieconfig.kafrenetrading.com");
        aboutInfo.SetCopyright("Copyright (C) 2023-2025 Ryan Ogurek");
        aboutInfo.SetName("ProffieConfig");
        wxAboutBox(aboutInfo, this);
    }, wxID_ABOUT);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxLaunchDefaultApplication(Paths::logs().native());
    }, ID_Logs);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        PCUI::showMessage(COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION);
    }, ID_Copyright);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        if (not editors.empty()) {
            PCUI::showMessage(_("All Editors must be closed to continue."), _("Open Editors"));
            return;
        }

        Props(this).ShowModal();
    }, ID_AddProp);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/blob/master/docs"); }, ID_Docs);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/issues/new"); }, ID_Issue);

    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(this); }, ID_RefreshDev);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (activeEditor == nullptr) {
            activeEditor = generateEditor(configSelect->entry()->GetStringSelection().ToStdString());
            if (activeEditor == nullptr) return;
            editors.emplace_back(activeEditor);
        }
        Arduino::applyToBoard(this, activeEditor);
    }, ID_ApplyChanges);
# 	if defined(__WINDOWS__)
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        if (not SerialMonitor::instance) SerialMonitor::instance = new SerialMonitor(this);
    }, ID_OpenSerial);
#	else
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise();
        else SerialMonitor::instance = new SerialMonitor(this);
    }, ID_OpenSerial);
#	endif
    Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        activeEditor = nullptr;
        if (configSelect->entry()->GetSelection() == 0) {
            update();
            return;
        }

        for (auto *editor : editors) {
            if (configSelect->entry()->GetStringSelection() == wxString{editor->getOpenConfig()}) {
                activeEditor = editor;
                break;
            }
        }

        update();
    }, ID_ConfigSelect);
    Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { update(); }, ID_DeviceSelect);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        if (activeEditor == nullptr) {
            activeEditor = generateEditor(configSelect->entry()->GetStringSelection().ToStdString());
            if (activeEditor == nullptr) return;
            editors.emplace_back(activeEditor);
        }
        activeEditor->Show();
        activeEditor->Raise();
    }, ID_EditConfig);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        auto addDialog{AddConfig{this}};
        if (OnboardFrame::instance != nullptr) {
            static_cast<wxToggleButton*>(addDialog.FindWindow(AddConfig::ID_ImportExisting))->Disable();
        }
        if (addDialog.ShowModal() != wxID_OK) return;

        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        auto configPath{Paths::configs() / (addDialog.configName + ".h").ToStdWstring()};
        if (not addDialog.existingPath.empty()) {
            fs::copy(addDialog.existingPath.ToStdWstring(), configPath);
        } else {
            std::ofstream{configPath}.flush();
        }

        update();
        configSelect->entry()->SetStringSelection(addDialog.configName);
        wxPostEvent(this, wxCommandEvent{wxEVT_CHOICE, ID_ConfigSelect});
    }, ID_AddConfig);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent &) {
        if (PCUI::showMessage(
                _("Are you sure you want to deleted the selected configuration?") + 
                "\n\n" +
                _("This action cannot be undone!"),
                _("Delete Config"),
                wxYES_NO | wxNO_DEFAULT | wxCENTER,
                this) == wxYES
           ) {
            if (activeEditor) activeEditor->Close(true);
            fs::remove(Paths::configs() / (configSelect->entry()->GetStringSelection().ToStdString() + ".h"));
            activeEditor = nullptr;
            update();
        }
    }, ID_RemoveConfig);
    Bind(Arduino::EVT_CLEAR_BLIST, [this](Arduino::Event&) {
        boardSelect->entry()->Clear();
    });
    Bind(Arduino::EVT_APPEND_BLIST, [this](Arduino::Event& evt) {
        boardSelect->entry()->Append(evt.str);
    });
    Bind(Arduino::EVT_REFRESH_DONE, [this](Arduino::Event& evt) {
        boardSelect->entry()->SetStringSelection(evt.str);
        if (boardSelect->entry()->GetSelection() == -1) boardSelect->entry()->SetSelection(0);
        update();
    });
}

void MainMenu::createTooltips() const {
  TIP(applyButton, _("Apply the current configuration to the selected Proffieboard."));
  TIP(boardSelect, _("Select the Proffieboard to connect to.\nThese IDs are assigned by the OS, and can vary."));
  TIP(refreshButton, _("Generate an up-to-date list of connected boards."));
}

void MainMenu::createMenuBar() {
  auto *file{new wxMenu};
  file->Append(ID_ReRunSetup, _("Re-Run First-Time Setup..."), _("Install Proffieboard Dependencies and View Tutorial"));
  file->Append(ID_AddProp, _("Prop Files..."));
  file->AppendSeparator();
  file->Append(ID_Logs, _("Show Logs..."));
  file->Append(wxID_ABOUT);
  file->Append(ID_Copyright, _("Licensing Information"));
  file->Append(wxID_EXIT);

  auto* help{new wxMenu};
  help->Append(ID_Docs, _("Documentation...\tCtrl+H"), _("Open the ProffieConfig docs in your web browser"));
  help->Append(ID_Issue, _("Help/Bug Report..."), _("Open GitHub to submit issue"));

  auto* menuBar{new wxMenuBar};
  menuBar->Append(file, _("&File"));
  menuBar->Append(help, _("&Help"));
  SetMenuBar(menuBar);
}

void MainMenu::createUI() {
  auto *sizer{new wxBoxSizer(wxVERTICAL)};

  auto *headerSection{new wxBoxSizer(wxHORIZONTAL)};
  auto *titleSection{new wxBoxSizer(wxVERTICAL)};
  auto *title{new wxStaticText(this, wxID_ANY, "ProffieConfig")};
  auto titleFont{title->GetFont()};
  titleFont.MakeBold();
# if defined(__WXGTK__) || defined(__WINDOWS__)
  titleFont.SetPointSize(20);
# elif defined (__WXOSX__)
  titleFont.SetPointSize(30);
#endif
  title->SetFont(titleFont);
  auto *subTitle{new wxStaticText(this, wxID_ANY, _("Created by Ryryog25"))};
  titleSection->Add(title);
  titleSection->Add(subTitle);
  headerSection->Add(titleSection);
  headerSection->AddSpacer(20);
  headerSection->AddStretchSpacer(1);
  auto *appIcon{PCUI::createStaticImage(this, wxID_ANY, Image::loadPNG("icon"))};
  appIcon->SetMaxSize(wxSize{64, 64});
  headerSection->Add(appIcon);

  auto *configSelectSection{new wxBoxSizer(wxHORIZONTAL)};
  configSelect = new PCUI::Choice(this, ID_ConfigSelect, { _("Select Config...") });
  addConfig = new wxButton(this, ID_AddConfig, _("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  removeConfig = new wxButton(this, ID_RemoveConfig, _("Remove"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  removeConfig->Disable();
  configSelectSection->Add(configSelect, wxSizerFlags{1}.Expand());
  configSelectSection->AddSpacer(10);
  configSelectSection->Add(addConfig, wxSizerFlags{}.Expand());
  configSelectSection->AddSpacer(10);
  configSelectSection->Add(removeConfig, wxSizerFlags{}.Expand());

  editConfig = new wxButton(this, ID_EditConfig, _("Edit Selected Configuration"));
  editConfig->Disable();

  auto *boardControls{new wxBoxSizer(wxHORIZONTAL)};
  auto boardEntries{Misc::createEntries({_("Select Board...")})};
# ifdef __WINDOWS__
  boardEntries.emplace_back(_("BOOTLOADER RECOVERY"));
# endif
  boardSelect = new PCUI::Choice(this, ID_DeviceSelect, boardEntries);
  refreshButton = new wxButton(this, ID_RefreshDev, _("Refresh Boards"));
  boardControls->Add(refreshButton);
  boardControls->AddSpacer(10);
  boardControls->Add(boardSelect, wxSizerFlags{1});

  applyButton = new wxButton(this, ID_ApplyChanges, _("Apply Selected Configuration to Board"));
  applyButton->Disable();
  openSerial = new wxButton(this, ID_OpenSerial, _("Open Serial Monitor"));
  openSerial->Disable();

  sizer->AddSpacer(20);
  sizer->Add(headerSection, wxSizerFlags{}.Border(wxLEFT | wxRIGHT, 10).Expand());
  sizer->AddSpacer(20);
  sizer->Add(configSelectSection, wxSizerFlags{}.Border(wxLEFT | wxRIGHT, 10).Expand());
  sizer->AddSpacer(10);
  sizer->Add(editConfig, wxSizerFlags{}.Border(wxLEFT | wxRIGHT, 10).Expand());
  sizer->AddSpacer(20);
  sizer->Add(boardControls, wxSizerFlags{}.Border(wxLEFT | wxRIGHT, 10).Expand());
  sizer->AddSpacer(10);
  sizer->Add(applyButton, wxSizerFlags{}.Border(wxLEFT | wxRIGHT, 10).Expand());
  sizer->AddSpacer(10);
  sizer->Add(openSerial, wxSizerFlags{}.Border(wxLEFT | wxRIGHT, 10).Expand());
  sizer->AddSpacer(20); // There's a sizing issue I need to figure out... for now we give it a chin

  SetSizerAndFit(sizer);
}

void MainMenu::update() const {
    auto lastConfig = configSelect->entry()->GetStringSelection();
    auto selectEntry{configSelect->entry()->GetString(0)};
    configSelect->entry()->Clear();
    configSelect->entry()->Append(selectEntry);

    fs::directory_iterator configsIterator{Paths::configs()};
    for (const auto& configFile : configsIterator) {
        if (not configFile.is_regular_file()) continue;
        if (configFile.path().extension() != ".h") continue;

        configSelect->entry()->Append(configFile.path().stem().native());
    }

    configSelect->entry()->SetStringSelection(lastConfig);
    if (configSelect->entry()->GetSelection() == -1) configSelect->entry()->SetSelection(0);

    auto configSelected = configSelect->entry()->GetSelection() != 0;
    auto boardSelected = boardSelect->entry()->GetSelection() != 0;
    auto recoverySelected = boardSelect->entry()->GetStringSelection().find(_("BOOTLOADER")) != string::npos;

    applyButton->Enable(configSelected && boardSelected);
    editConfig->Enable(configSelected);
    removeConfig->Enable(configSelected);
    openSerial->Enable(boardSelected && !recoverySelected);
}

void MainMenu::removeEditor(EditorWindow *editor) {
    for (auto it{editors.begin()}; it != editors.end(); ++it) {
        if (*it == editor) {
            editors.erase(it);
            if (activeEditor == editor) activeEditor = nullptr;
            break;
        }
    }
}

EditorWindow *MainMenu::generateEditor(const string& configName) {
    auto *newEditor{new EditorWindow(configName, this)};
    if (not Configuration::readConfig(Paths::configs() / (configName + ".h"), newEditor)) {
        PCUI::showMessage(_("Error while reading configuration file!"), _("Config Read Error"), wxOK | wxCENTER, this);
        newEditor->Destroy();
        return nullptr;
    }
    return newEditor;
}
