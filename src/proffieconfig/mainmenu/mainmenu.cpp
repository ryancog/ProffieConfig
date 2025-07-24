#include "mainmenu.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024-2025 Ryan Ogurek

#include <fstream>

#include <wx/aboutdlg.h>
#include <wx/cursor.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/toplevel.h>
#include <wx/utils.h>

#include "config/config.h"
#include "ui/message.h"
#include "ui/plaque.h"
#include "ui/frame.h"
#include "utils/defer.h"
#include "paths/paths.h"
#include "utils/image.h"
#include "utils/string.h"
#include "dialogs/addconfig.h"
#include "dialogs/manifest.h"

#include "../core/defines.h"
#include "../core/appstate.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"
#include "../editor/editorwindow.h"
#include "../onboard/onboard.h"
#include "../tools/arduino.h"
#include "../tools/serialmonitor.h"
#include "../mainmenu/dialogs/props.h"


MainMenu* MainMenu::instance{nullptr};
MainMenu::MainMenu(wxWindow* parent) : 
    PCUI::Frame(
        parent,
        wxID_ANY,
        "ProffieConfig",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)
    ) {
    Notifier::create(this, mNotifyData);

    createUI();
    createMenuBar();
    bindEvents();

    auto configChoices{configSelection.choices()};
    {
        auto configList{Config::fetchListFromDisk()};
        configChoices.insert(
            configChoices.end(),
            configList.begin(),
            configList.end()
        );
    }
    configSelection.setChoices(std::move(configChoices));

    initializeNotifier();
    Show();
}

void MainMenu::bindEvents() {
    auto promptClose{[this]() -> bool {
        for (auto& [config, editor]: mEditors) {
            if (not config->isSaved()) {
                auto res{PCUI::showMessage(
                    _("There is at least one editor open with unsaved changes, are you sure you want to exit?") +
                    "\n\n"+
                    _("All unsaved changes will be lost!"),
                    _("Open Editor(s)"),
                    wxYES_NO | wxNO_DEFAULT | wxCENTER | wxICON_EXCLAMATION
                )};
                if (res == wxNO) return false;
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
    Bind(Progress::EVT_UPDATE, [&](ProgressEvent& event) { 
        Progress::handleEvent(&event); 
    });
    Bind(Misc::EVT_MSGBOX, [&](Misc::MessageBoxEvent& event) {
        PCUI::showMessage(event.message, event.caption, event.style, this);
    });
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
        aboutInfo.SetVersion(wxSTRINGIZE(BIN_VERSION));
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
        if (not mEditors.empty()) {
            PCUI::showMessage(_("All Editors must be closed to continue."), _("Open Editors"));
            return;
        }

        Props(this).ShowModal();
    }, ID_AddProp);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        ManifestDialog(this).ShowModal();
    }, ID_UpdateManifest);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/blob/master/docs"); }, ID_Docs);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/issues/new"); }, ID_Issue);

    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { Arduino::refreshBoards(this); }, ID_RefreshDev);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        // if (activeEditor == nullptr) {
        //     activeEditor = generateEditor(configSelection);
        //     if (activeEditor == nullptr) return;
        //     editors.emplace_back(activeEditor);
        // }
        // Arduino::applyToBoard(this, activeEditor);
    }, ID_ApplyChanges);
# 	if defined(__WINDOWS__)
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        if (not SerialMonitor::instance) SerialMonitor::instance = new SerialMonitor(this);
    }, ID_OpenSerial);
#	else
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        if (SerialMonitor::instance != nullptr) SerialMonitor::instance->Raise();
        else SerialMonitor::instance = new SerialMonitor(this, boardSelection);
    }, ID_OpenSerial);
#	endif

    configSelection.setUpdateHandler([this](uint32 id) {
        if (id != configSelection.ID_SELECTION) return;
        mNotifyData.notify(ID_ConfigSelection);
    });
    boardSelection.setUpdateHandler([this](uint32 id) { 
        if (id != boardSelection.ID_SELECTION) return;
        mNotifyData.notify(ID_BoardSelection);
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);
        Defer cursorDefer{[]() { wxSetCursor(wxNullCursor); }};
        auto config{Config::open(configSelection)};
        auto editorIter{mEditors.find(config)};
        if (editorIter != mEditors.end()) {
            editorIter->second->Show();
            editorIter->second->Raise();
            return;
        } 

        auto editor{mEditors.emplace(config, new EditorWindow(this, config)).first->second};
        editor->Show();
        editor->Raise();
    }, ID_EditConfig);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        auto addDialog{AddConfig{this}};
        if (OnboardFrame::instance != nullptr) {
            static_cast<wxToggleButton*>(addDialog.FindWindow(AddConfig::ID_ImportExisting))->Disable();
        }
        if (addDialog.ShowModal() != wxID_OK) return;

        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        auto configPath{Paths::configs() / (static_cast<string>(addDialog.configName) + ".h")};
        if (not static_cast<filepath>(addDialog.importPath).empty()) {
            fs::copy(addDialog.importPath, configPath);
        } else {
            std::ofstream{configPath}.flush();
        }

        auto choices{configSelection.choices()};
        choices.push_back(addDialog.configName);
        configSelection.setChoices(std::move(choices));
        configSelection = configSelection.choices().size() - 1;
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
            for (auto iter{mEditors.begin()}; iter != mEditors.end(); ++iter) {
                if (static_cast<string>(iter->first->name) == static_cast<string>(configSelection)) {
                    iter->first->close();
                    iter->second->Destroy();
                    mEditors.erase(iter);
                    break;
                }
            }
            Config::remove(static_cast<string>(configSelection));

            auto configChoices{configSelection.choices()};
            configChoices.erase(std::next(configChoices.begin(), configSelection));
            configSelection.setChoices(std::move(configChoices));
            configSelection = 0;
        }
    }, ID_RemoveConfig);
}

void MainMenu::handleNotification(uint32 id) {
    bool rebound{id == ID_REBOUND};
    if (rebound or id == ID_ConfigSelection) {
        FindWindow(ID_EditConfig)->Enable(configSelection != 0);
        FindWindow(ID_RemoveConfig)->Enable(configSelection != 0);
        FindWindow(ID_ApplyChanges)->Enable(configSelection != 0 and boardSelection != 0);
    } 
    if (rebound or id == ID_BoardSelection) {
        FindWindow(ID_ApplyChanges)->Enable(configSelection != 0 and boardSelection != 0);
        FindWindow(ID_OpenSerial)->Enable(boardSelection != 0);
    }
}

void MainMenu::createMenuBar() {
    auto *file{new wxMenu};
    file->Append(ID_ReRunSetup, _("Re-Run First-Time Setup..."), _("Install Proffieboard Dependencies and View Tutorial"));
    file->Append(ID_AddProp, _("Prop Files..."));
    file->Append(ID_UpdateManifest, _("Update Channel..."));
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
#   if defined(__WXGTK__) || defined(__WINDOWS__)
    titleFont.SetPointSize(20);
#   elif defined (__WXOSX__)
    titleFont.SetPointSize(30);
#   endif
    title->SetFont(titleFont);
    auto *subTitle{new wxStaticText(this, wxID_ANY, _("Created by Ryryog25"))};
    titleSection->Add(title);
    titleSection->Add(subTitle);

    auto *appIcon{PCUI::createStaticImage(this, wxID_ANY, Image::loadPNG("icon"))};
    appIcon->SetMaxSize(wxSize{64, 64});

    headerSection->AddSpacer(10);
    headerSection->Add(titleSection);
    headerSection->AddSpacer(20);
    headerSection->AddStretchSpacer(1);
    headerSection->Add(appIcon);
    headerSection->AddSpacer(10);

    auto *configSelectSection{new wxBoxSizer(wxHORIZONTAL)};
    configSelection.setChoices(Utils::createEntries({
        _("Select Config..."),
    }));
    configSelection = 0;
    auto *configSelect{new PCUI::Choice(this, configSelection)};

    auto *addConfig{new wxButton(this, ID_AddConfig, _("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)};
    auto *removeConfig{new wxButton(
        this,
        ID_RemoveConfig,
        _("Remove"),
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};

    configSelectSection->AddSpacer(10);
    configSelectSection->Add(configSelect, wxSizerFlags{1}.Expand());
    configSelectSection->AddSpacer(5);
    configSelectSection->Add(addConfig, wxSizerFlags{}.Expand());
    configSelectSection->AddSpacer(5);
    configSelectSection->Add(removeConfig, wxSizerFlags{}.Expand());
    configSelectSection->AddSpacer(10);

    auto *editConfig{new wxButton(
        this,
        ID_EditConfig,
        _("Edit Selected Configuration")
    )};

    auto *boardControls{new wxBoxSizer(wxHORIZONTAL)};
    auto boardEntries{Utils::createEntries({_("Select Board...")})};
#   ifdef __WINDOWS__
    boardEntries.emplace_back(_("BOOTLOADER RECOVERY"));
#   endif
    boardSelection.setChoices(std::move(boardEntries));
    boardSelection = 0;
    auto *boardSelect{new PCUI::Choice(this, boardSelection)};
    boardSelect->SetToolTip(_("Select the Proffieboard to connect to.\nThese IDs are assigned by the OS, and can vary."));
    auto *refreshButton{new wxButton(this, ID_RefreshDev, _("Refresh Boards"))};
    refreshButton->SetToolTip(_("Generate an up-to-date list of connected boards."));

    boardControls->AddSpacer(10);
    boardControls->Add(refreshButton);
    boardControls->AddSpacer(5);
    boardControls->Add(boardSelect, wxSizerFlags{1});
    boardControls->AddSpacer(10);

    auto *applyButton{new wxButton(
        this,
        ID_ApplyChanges,
        _("Apply Selected Configuration to Board")
    )};
    applyButton->SetToolTip(_("Apply the current configuration to the selected Proffieboard."));
    auto *openSerial{new wxButton(this, ID_OpenSerial, _("Open Serial Monitor"))};

    sizer->AddSpacer(20);
    sizer->Add(headerSection, wxSizerFlags().Expand());
    sizer->AddSpacer(20);
    sizer->Add(configSelectSection, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(
        editConfig,
        wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 10)
    );
    sizer->AddSpacer(20);
    sizer->Add(boardControls, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(
        applyButton,
        wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 10)
    );
    sizer->AddSpacer(10);
    sizer->Add(
        openSerial,
        wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 10)
    );
    sizer->AddSpacer(20); // There's a sizing issue I need to figure out... for now we give it a chin

    SetSizerAndFit(sizer);
}

void MainMenu::removeEditor(EditorWindow *editor) {
    for (auto it{mEditors.begin()}; it != mEditors.end(); ++it) {
        if (it->second == editor) {
            mEditors.erase(it);
            break;
        }
    }
}
