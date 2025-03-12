#include "mainmenu.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../core/defines.h"
#include "../core/appstate.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"
#include "../core/config/configuration.h"
#include "../editor/editorwindow.h"
#include "../onboard/onboard.h"
#include "dialogs/addconfig.h"
#include "../tools/arduino.h"
#include "../tools/serialmonitor.h"
#include "../mainmenu/dialogs/props.h"

#include "ui/controls.h"
#include "utils/paths.h"
#include "utils/image.h"
#include "wx/utils.h"

#include <wx/event.h>
#include <wx/menu.h>
#include <wx/aboutdlg.h>
#include <wx/settings.h>

#ifdef __WINDOWS__
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif

#include <wx/statbmp.h>

MainMenu* MainMenu::instance{nullptr};
MainMenu::MainMenu(wxWindow* parent) : wxFrame(parent, wxID_ANY, "ProffieConfig") {
  createUI();
  createMenuBar();
  createTooltips();
  bindEvents();
  update();

# ifdef __WINDOWS__
  SetIcon( wxICON(IDI_ICON1) );
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
# endif

  Show(true);
}

void MainMenu::bindEvents() {
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
        AppState::instance->saveState();
        for (auto *editor : editors) {
            if (not editor->isSaved() && event.CanVeto()) {
                if (wxMessageDialog(
                            this,
                            "There is at least one editor open, are you sure you want to exit?\n\n"
                            "Any unsaved changes will be lost!",
                            "Open Editor(s)",
                            wxYES_NO | wxNO_DEFAULT | wxCENTER | wxICON_EXCLAMATION)
                        .ShowModal() == wxID_NO) {
                    event.Veto();
                    return;
                } else {
                    break;
                }
            }
        }
        event.Skip();
    });
    Bind(Progress::EVT_UPDATE, [&](wxCommandEvent& event) { Progress::handleEvent((Progress::ProgressEvent*)&event); }, wxID_ANY);
    Bind(Misc::EVT_MSGBOX, [&](wxCommandEvent &event) {
        wxMessageDialog(this, ((Misc ::MessageBoxEvent *)&event)->message, ((Misc ::MessageBoxEvent *)&event)->caption, ((Misc ::MessageBoxEvent *)&event)->style).ShowModal();
    }, wxID_ANY);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(); OnboardFrame::instance = new OnboardFrame(); }, ID_ReRunSetup);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxAboutDialogInfo aboutInfo;
        aboutInfo.SetDescription(
                                "All-in-one Proffieboard Management Utility\n"
                                "\n"
                                "ProffieOS v" wxSTRINGIZE(PROFFIEOS_VERSION) " | Arduino CLI v" wxSTRINGIZE(ARDUINO_CLI_VERSION)
                                );
        aboutInfo.SetVersion(wxSTRINGIZE(EXEC_VERSION));
        aboutInfo.SetWebSite("https://proffieconfig.kafrenetrading.com");
        aboutInfo.SetCopyright("Copyright (C) 2023-2025 Ryan Ogurek");
        aboutInfo.SetName("ProffieConfig");
        wxAboutBox(aboutInfo);
    }, wxID_ABOUT);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxLaunchDefaultApplication(Paths::logs().string());
    }, ID_Logs);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxMessageDialog(this, COPYRIGHT_NOTICE, "ProffieConfig Copyright Notice", wxOK | wxICON_INFORMATION).ShowModal();
    }, ID_Copyright);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        if (not editors.empty()) {
            wxMessageBox("All Editors must be closed to continue.", "Editors Open");
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
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { SerialMonitor::instance = new SerialMonitor(this); SerialMonitor::instance->Close(true); }, ID_OpenSerial);
#	else
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise(); else SerialMonitor::instance = new SerialMonitor(this); }, ID_OpenSerial);
#	endif
    Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        activeEditor = nullptr;
        if (configSelect->entry()->GetStringSelection() == "Select Config...") {
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

        auto configPath{Paths::configs() / (addDialog.configName + ".h")};
        if (not addDialog.existingPath.empty()) {
            fs::copy(addDialog.existingPath, configPath);
        } else {
            std::ofstream{configPath}.flush();
        }

        update();
        configSelect->entry()->SetStringSelection(addDialog.configName);
        wxPostEvent(this, wxCommandEvent{wxEVT_CHOICE, ID_ConfigSelect});
    }, ID_AddConfig);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent &) {
        if (wxMessageDialog(this, "Are you sure you want to deleted the selected configuration?\n\nThis action cannot be undone!", "Delete Config", wxYES_NO | wxNO_DEFAULT | wxCENTER).ShowModal() == wxID_YES) {
            if (activeEditor) activeEditor->Close(true);
            remove((Paths::configs() / (configSelect->entry()->GetStringSelection().ToStdString() + ".h")).c_str());
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

void MainMenu::createTooltips() {
  TIP(applyButton, "Apply the current configuration to the selected Proffieboard.");
  TIP(boardSelect, "Select the Proffieboard to connect to.\nThis will be an unrecognizable device identifier, but chances are there's only one which will show up.");
  TIP(refreshButton, "Refresh the detected boards.");
}

void MainMenu::createMenuBar() {
  wxMenu *file = new wxMenu;
  file->Append(ID_ReRunSetup, "Re-Run First-Time Setup...", "Install Proffieboard Dependencies and View Tutorial");
  file->Append(ID_AddProp, "Props...");
  file->AppendSeparator();
  file->Append(ID_Logs, "Show Logs...");
  file->Append(wxID_ABOUT);
  file->Append(ID_Copyright, "Copyright Notice");
  file->Append(wxID_EXIT);

  wxMenu* help = new wxMenu;
  help->Append(ID_Docs, "Documentation...\tCtrl+H", "Open the ProffieConfig docs in your web browser");
  help->Append(ID_Issue, "Help/Bug Report...", "Open GitHub to submit issue");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(file, "&File");
  menuBar->Append(help, "&Help");
  SetMenuBar(menuBar);
}

void MainMenu::createUI() {
  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto headerSection = new wxBoxSizer(wxHORIZONTAL);
  auto titleSection = new wxBoxSizer(wxVERTICAL);
  auto title = new wxStaticText(this, wxID_ANY, "ProffieConfig");
  auto titleFont = title->GetFont();
  titleFont.MakeBold();
# if defined(__WXGTK__) || defined(__WINDOWS__)
  titleFont.SetPointSize(20);
# elif defined (__WXOSX__)
  titleFont.SetPointSize(30);
#endif
  title->SetFont(titleFont);
  auto subTitle = new wxStaticText(this, wxID_ANY, "Created by Ryryog25\n\n");
  titleSection->Add(title, wxSizerFlags(0).Border(wxLEFT | wxRIGHT | wxTOP, 10));
  titleSection->Add(subTitle, wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 10));
  headerSection->Add(titleSection, wxSizerFlags(0));
  headerSection->AddStretchSpacer(1);
  auto *appIcon{new wxStaticBitmap(this, wxID_ANY, Image::loadPNG("icon"))};
  appIcon->SetMaxSize(wxSize{64, 64});
  headerSection->Add(appIcon, wxSizerFlags(0).Border(wxALL, 10));

  auto configSelectSection = new wxBoxSizer(wxHORIZONTAL);
  configSelect = new PCUI::Choice(this, ID_ConfigSelect, Misc::createEntries({"Select Config..."}));
  addConfig = new wxButton(this, ID_AddConfig, "Add", wxDefaultPosition, wxSize(50, -1), wxBU_EXACTFIT);
  removeConfig = new wxButton(this, ID_RemoveConfig, "Remove", wxDefaultPosition, wxSize(75, -1), wxBU_EXACTFIT);
  removeConfig->Disable();
  configSelectSection->Add(configSelect, wxSizerFlags(1).Border(wxALL, 5).Expand());
  configSelectSection->Add(addConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());
  configSelectSection->Add(removeConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());

  auto boardControls = new wxBoxSizer(wxHORIZONTAL);
# ifdef __WINDOWS__
  auto boardEntries = Misc::createEntries({"Select Board...", "BOOTLOADER RECOVERY"});
# else
  auto boardEntries = Misc::createEntries({"Select Board..."});
# endif
  boardSelect = new PCUI::Choice(this, ID_DeviceSelect, boardEntries);
  refreshButton = new wxButton(this, ID_RefreshDev, "Refresh Boards");
  boardControls->Add(refreshButton, wxSizerFlags(0).Border(wxALL, 5));
  boardControls->Add(boardSelect, wxSizerFlags(1).Border(wxALL, 5));

  auto options = new wxBoxSizer(wxVERTICAL);
  applyButton = new wxButton(this, ID_ApplyChanges, "Apply Selected Configuration to Board");
  applyButton->Disable();
  editConfig = new wxButton(this, ID_EditConfig, "Edit Selected Configuration");
  editConfig->Disable();
  openSerial = new wxButton(this, ID_OpenSerial, "Open Serial Monitor");
  openSerial->Disable();
  options->Add(applyButton, wxSizerFlags(0).Border(wxALL, 5).Expand());
  options->Add(editConfig, wxSizerFlags(0).Border(wxALL, 5).Expand());
  options->Add(openSerial, wxSizerFlags(0).Border(wxALL, 5).Expand());

  sizer->Add(headerSection, wxSizerFlags(0).Expand());
  sizer->Add(configSelectSection, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->Add(boardControls, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->Add(options, wxSizerFlags(0).Border(wxALL, 5).Expand());
  sizer->AddSpacer(20); // There's a sizing issue I need to figure out... for now we give it a chin

  SetSizerAndFit(sizer);
}

void MainMenu::update() {
    auto lastConfig = configSelect->entry()->GetStringSelection();
    configSelect->entry()->Clear();
    configSelect->entry()->Append("Select Config...");

    fs::directory_iterator configsIterator{Paths::configs()};
    for (const auto configFile : configsIterator) {
        if (not configFile.is_regular_file()) continue;
        if (configFile.path().extension() != ".h") continue;

        configSelect->entry()->Append(configFile.path().stem().string());
    }

    configSelect->entry()->SetStringSelection(lastConfig);
    if (configSelect->entry()->GetSelection() == -1) configSelect->entry()->SetSelection(0);

    auto configSelected = configSelect->entry()->GetStringSelection() != "Select Config...";
    auto boardSelected = boardSelect->entry()->GetStringSelection() != "Select Board...";
    auto recoverySelected = boardSelect->entry()->GetStringSelection().find("BOOTLOADER") != std::string::npos;

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

EditorWindow *MainMenu::generateEditor(const std::string& configName) {
    auto newEditor = new EditorWindow(configName, this);
    if (not Configuration::readConfig(Paths::configs() / (configName + ".h"), newEditor)) {
        wxMessageDialog(this, "Error reading configuration file!", "Config Error", wxOK | wxCENTER).ShowModal();
        newEditor->Destroy();
        return nullptr;
    }
    return newEditor;
}
