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
#include "utils/paths.h"
#include "utils/image.h"
#include "utils/string.h"
#include "dialogs/addconfig.h"
#include "dialogs/manifest.h"

#include "config/info.h"
#include "log/info.h"
#include "pconf/info.h"
#include "ui/info.h"
#include "utils/info.h"
#include "versions/info.h"

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
    boardSelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_STRING);
    configSelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_STRING);

    createUI();
    createMenuBar();
    bindEvents();
    updateConfigChoices();

    initializeNotifier();
    Show();
}

void MainMenu::bindEvents() {
    auto promptClose{[this]() -> bool {
        for (auto *editor : mEditors) {
            if (not editor->getOpenConfig().isSaved()) {
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
            "Arduino CLI: v" + Arduino::version() + "\n"
            "Config: v" + Config::version() + "\n"
            "Log: v" + Log::version() + "\n"
            "PConf: v" + PConf::version() + "\n"
            "PCUI: v" + PCUI::version() + "\n"
            "Utils: v" + Utils::version() + "\n"
            "Versions: v" + Versions::version() + "\n"
        );
        aboutInfo.SetVersion(wxSTRINGIZE(VERSION));
        aboutInfo.SetWebSite("https://proffieconfig.kafrenetrading.com");
        aboutInfo.SetCopyright("Copyright (C) 2023-2025 Ryan Ogurek");
        aboutInfo.SetName("ProffieConfig");
        wxAboutBox(aboutInfo, this);
    }, wxID_ABOUT);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxLaunchDefaultApplication(Paths::logDir().native());
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

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/blob/master/docs");
    }, ID_Docs);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/issues/new");
    }, ID_Issue);

    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);

        auto *progDialog{new Progress(this)};
        progDialog->SetTitle(_("Board Refresh"));
        progDialog->Update(0, _("Initializing..."));

        std::thread{[this, progDialog]() {
            progDialog->emitEvent(10, _("Discovering Boards..."));
            const auto boards{Arduino::getBoards()};

            progDialog->emitEvent(90, _("Processing and Finalizing..."));
            auto choices{boardSelection.choices()};
#           ifdef __WXOSX__
            if (choices.size() > 1) choices.erase(std::next(choices.begin()));
            choices.reserve(boards.size() + 1);
#           else
            choices.erase(std::next(choices.begin()), std::prev(choices.end()));
            choices.reserve(boards.size() + 2);
#           endif
            choices.insert(std::next(choices.begin()), boards.begin(), boards.end());
            boardSelection.setChoices(std::move(choices));

            progDialog->emitEvent(100, _("Done"));
            mNotifyData.notify(ID_AsyncDone);
        }}.detach();
    }, ID_RefreshDev);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        wxSetCursor(wxCURSOR_WAIT);

        auto *config{Config::getIfOpen(configSelection)};
        const auto configWasOpen{static_cast<bool>(config)};
        if (not configWasOpen) {
            const auto res{Config::open(configSelection)};
            if (auto *err = std::get_if<string>(&res)) {
                PCUI::showMessage(*err, _("Cannot Open Config for Apply"));
                return;
            }
            config = std::get<Config::Config *>(res);
        }

        auto *progDialog{new Progress(this)};
        progDialog->SetTitle(_("Applying Changes"));
        progDialog->Update(0, _("Initializing..."));
        
        std::thread{[this, config, progDialog]() {
            Defer defer{[this]() { mNotifyData.notify(ID_AsyncDone); }};

            const auto res{
                Arduino::applyToBoard(boardSelection, *config, progDialog)
            };

            if (auto *err = std::get_if<string>(&res)) {
                auto *evt{new Misc::MessageBoxEvent(
                    Misc::EVT_MSGBOX, wxID_ANY, *err, _("Cannot Apply Changes")
                )};
                wxQueueEvent(this, evt);
                return;
            }
            const auto& result{std::get<Arduino::Result>(res)};

            wxString message{_("Changes Successfully Applied to ProffieBoard!")};
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
                Misc::EVT_MSGBOX,
                wxID_ANY,
                message,
                _("Apply Changes to Board"),
                wxOK | wxICON_INFORMATION
            )};
            wxQueueEvent(this, evt);
        }}.detach();
    }, ID_ApplyChanges);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        if (not SerialMonitor::instance) SerialMonitor::instance = new SerialMonitor(this, boardSelection);
        else {
            SerialMonitor::instance->Show();
            SerialMonitor::instance->Raise();
        }
    }, ID_OpenSerial);

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
        auto res{Config::open(configSelection)};
        if (auto *ptr = std::get_if<string>(&res)) {
            PCUI::showMessage(*ptr, _("Cannot Edit Config"));
            return;
        }

        auto& config{*std::get<Config::Config *>(res)};
        for (auto *editor : mEditors) {
            if (&editor->getOpenConfig() == &config) {
                editor->Show();
                editor->Raise();
                return;
            }
        }

        auto editor{mEditors.emplace_back(new EditorWindow(this, config))};
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

        if (static_cast<filepath>(addDialog.importPath).empty()) {
            auto res{Config::open(addDialog.configName)};
            if (auto *err = std::get_if<string>(&res)) {
                PCUI::showMessage(*err, _("Failed Creating Config"));
                return;
            }
            auto& config{*std::get<Config::Config *>(res)};
            config.save();
            config.close();
        } else {
            auto err{Config::import(addDialog.configName, addDialog.importPath)};
            if (err) {
                PCUI::showMessage(*err, _("Cannot Import Config"));
                return;
            }
        }

        updateConfigChoices();
        configSelection = addDialog.configName;
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
            auto *config{Config::getIfOpen(configSelection)};
            for (auto iter{mEditors.begin()}; iter != mEditors.end(); ++iter) {
                auto editor{*iter};
                if (&editor->getOpenConfig() == config) {
                    editor->Destroy();
                    mEditors.erase(iter);
                    break;
                }
            }
            Config::remove(static_cast<string>(configSelection));

            updateConfigChoices();
        }
    }, ID_RemoveConfig);
}

void MainMenu::updateConfigChoices() {
    vector<string> choices{_("Select Config...").ToStdString()};
    const auto configList{Config::fetchListFromDisk()};
    choices.insert(choices.end(), configList.begin(), configList.end());
    configSelection.setChoices(std::move(choices));
    if (configSelection == -1) configSelection = 0;
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
        bool canOpenSerial{boardSelection != 0};
#       ifdef __WXMSW__
        canOpenSerial &= boardSelection != boardSelection.choices().size() - 1;
#       endif
        FindWindow(ID_OpenSerial)->Enable(canOpenSerial);
    }
    if (rebound or id == ID_AsyncDone) {
        wxSetCursor(wxNullCursor);
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
    boardEntries.emplace_back(_("BOOTLOADER RECOVERY").ToStdString());
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
#   ifdef wxMSW__
    sizer->AddSpacer(20); // There's a sizing issue I need to figure out... for now we give it a chin
#   else
    sizer->AddSpacer(10);
#   endif

    SetSizerAndFit(sizer);
}

void MainMenu::removeEditor(EditorWindow *editor) {
    for (auto it{mEditors.begin()}; it != mEditors.end(); ++it) {
        if (*it == editor) {
            mEditors.erase(it);
            break;
        }
    }
}
