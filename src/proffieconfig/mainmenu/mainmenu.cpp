#include "mainmenu.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/mainmenu.cpp
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

#include <wx/aboutdlg.h>
#include <wx/cursor.h>
#include <wx/collpane.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/toplevel.h>
#include <wx/utils.h>

#include "app/info.h"
#include "config/config.h"
#include "ui/message.h"
#include "ui/plaque.h"
#include "ui/frame.hpp"
#include "utils/defer.h"
#include "utils/paths.h"
#include "utils/image.h"
#include "utils/string.h"
#include "dialogs/addconfig.h"
#include "dialogs/manifest.h"
#include "config/info.h"
#include "log/info.h"
#include "pconf/info.h"
#include "ui/info.hpp"
#include "utils/info.h"
#include "versions/info.h"
#include "versions_manager/info.h"
#include "versions_manager/manager.h"

#include "../core/licenses.h"
#include "../core/appstate.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"
#include "../editor/editorwindow.h"
#include "../tools/arduino.h"
#include "../tools/serialmonitor.h"
#include "../onboard/onboard.h"
#include "wx/font.h"

MainMenu *MainMenu::instance{nullptr};
MainMenu::MainMenu(wxWindow* parent) : 
    pcui::Frame(
        parent,
        AppState::ID_MainMenu,
        "ProffieConfig",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)
    ) {
    NotifyReceiver::create(this, mNotifyData);
    boardSelection.setPersistence(pcui::ChoiceData::Persistence::String);
    configSelection.setPersistence(pcui::ChoiceData::Persistence::String);

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
                auto res{pcui::showMessage(
                    _("There is at least one editor open with unsaved changes, are you sure you want to exit?") +
                    "\n\n"+
                    _("All unsaved changes will be lost!"),
                    _("Open Editor(s)"),
                    wxYES_NO | wxNO_DEFAULT | wxCENTER | wxICON_EXCLAMATION
                )};
                if (res != wxYES) return false;
                break;
            }
        }

        return true;
    }};

    Bind(wxEVT_CLOSE_WINDOW, [promptClose](wxCloseEvent& event) {
        AppState::saveState();
        if (event.CanVeto() and not promptClose()) {
            event.Veto();
            return;
        }
        event.Skip();
    });
    Bind(Progress::EVT_UPDATE, [&](ProgressEvent& event) { 
        Progress::handleEvent(&event); 
    });
    Bind(misc::EVT_MSGBOX, [&](misc::MessageBoxEvent& event) {
        pcui::showMessage(event.message_, event.caption_, event.style_, this);
    });
    Bind(wxEVT_MENU, [&](wxCommandEvent&) { Close(true); }, wxID_EXIT);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxAboutDialogInfo aboutInfo;
        const auto componentVersions{
            string{} +
            "App: v" + app::version() + "\n"
            "Config: v" + Config::version() + "\n"
            "Log: v" + Log::version() + "\n"
            "PConf: v" + PConf::version() + "\n"
            "pcui: v" + pcui::version() + "\n"
            "Utils: v" + Utils::version() + "\n"
            "Versions: v" + Versions::version() + "\n"
            "Versions Manager: v" + VersionsManager::version() + "\n"
            "Arduino CLI: v" + Arduino::version() + "\n"
        };
#       ifdef __WXOSX__
        aboutInfo.SetDescription(_("All-in-one Proffieboard Management Utility"));
        aboutInfo.SetVersion(
            wxSTRINGIZE(BIN_VERSION),
            "Core: v" + wxString{wxSTRINGIZE(BIN_VERSION)} + "\n"
            + componentVersions
        );
#       else
        aboutInfo.SetDescription(
            _("All-in-one Proffieboard Management Utility") + "\n\n"
            + componentVersions
        );
        aboutInfo.SetVersion(wxSTRINGIZE(BIN_VERSION));
#       endif
#       ifdef __WXGTK__
        aboutInfo.SetWebSite("https://proffieconfig.kafrenetrading.com");
#       endif
        aboutInfo.SetCopyright("Copyright (C) 2023-2026 Ryan Ogurek");
        aboutInfo.SetName("ProffieConfig");
        wxAboutBox(aboutInfo, this);
    }, wxID_ABOUT);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxLaunchDefaultApplication(paths::logDir().native());
    }, ID_Logs);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxDialog dialog{
            this,
            wxID_ANY,
            wxString::Format(_("ProffieConfig Copyright & License Info")),
            wxDefaultPosition,
            wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxCENTER | wxRESIZE_BORDER
        };

        auto *scrollWin{new wxScrolledWindow(&dialog)};
        auto *scrollWrapSizer{new wxBoxSizer(wxVERTICAL)};
        for (const auto& info : LICENSES) {
            auto *sizer{new wxBoxSizer(wxVERTICAL)};
            auto *nameText{new wxStaticText(scrollWin, wxID_ANY, info.name_)};
            auto font{nameText->GetFont()};
            font.SetFractionalPointSize(1.2 * font.GetFractionalPointSize());
            nameText->SetFont(font);
            sizer->Add(nameText);
            auto *copyrightText{new wxStaticText(
                scrollWin,
                wxID_ANY,
                wxString{L"Copyright \u00A9 "} + 
                info.date_ + ' ' + info.author_
            )};
            sizer->Add(copyrightText);
            auto *detailText{new wxStaticText(scrollWin, wxID_ANY, info.detail_)};
            sizer->Add(detailText);

            constexpr auto SHOW_STR{"Show License"};
            auto *pane{new wxCollapsiblePane(
                scrollWin,
                wxID_ANY,
                SHOW_STR,
                wxDefaultPosition,
                wxDefaultSize,
                wxCP_NO_TLW_RESIZE
            )};
            const auto onPaneChanged{[pane, scrollWin](
                wxCollapsiblePaneEvent& evt
            ) {
                pane->SetLabel(evt.GetCollapsed() ? SHOW_STR : "Hide License");
                scrollWin->Layout();
                scrollWin->FitInside();
            }};
            pane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, onPaneChanged);
            auto *licenseText{new wxTextCtrl(
                pane->GetPane(),
                wxID_ANY,
                info.license_,
                wxDefaultPosition,
                wxDefaultSize,
                wxTE_READONLY | wxTE_MULTILINE |
                wxTE_NO_VSCROLL | wxTE_WORDWRAP | wxTE_AUTO_URL
            )};
            auto licenseFont{licenseText->GetFont()};
            licenseFont.SetFamily(wxFONTFAMILY_TELETYPE);
            licenseText->SetFont(licenseFont);

            const auto onMouseWheel{[scrollWin](wxMouseEvent& evt) {
                // Have to forward it up the chain manually.
                scrollWin->HandleOnMouseWheel(evt);
            }};
            licenseText->Bind(wxEVT_MOUSEWHEEL, onMouseWheel);

            const auto textExtent{licenseText->GetTextExtent('M')};
            // Width needs to be set first so that the number of lines is
            // properly calculated
            const auto width{textExtent.x * 80};
            licenseText->SetMinClientSize({width, -1});
            licenseText->SetSize(licenseText->GetMinSize());

            const auto height{
                (textExtent.y * licenseText->GetNumberOfLines())
                + (textExtent.y / 2)
            };
            licenseText->SetMinClientSize({width, height});
            licenseText->SetSize(licenseText->GetMinSize());

            auto *paneSizer{new wxBoxSizer(wxVERTICAL)};
            paneSizer->Add(licenseText, 0, wxEXPAND);
            pane->GetPane()->SetSizer(paneSizer);
            pane->SetMinClientSize({
                pane->GetPane()->GetBestVirtualSize().x, -1
            });

            sizer->Add(pane, 0, wxEXPAND);
            scrollWrapSizer->AddSpacer(10);
            scrollWrapSizer->Add(
                sizer,
                wxSizerFlags()
                    .Expand()
                    .Border(wxLEFT | wxRIGHT, 10)
            );
        }
        scrollWrapSizer->AddSpacer(10);
        scrollWin->SetSizerAndFit(scrollWrapSizer);
        scrollWin->SetMinSize({
            scrollWin->GetBestVirtualSize().x, 600
        });
        scrollWin->SetScrollRate(-1, 10);

        auto *dlgSizer{new wxBoxSizer(wxVERTICAL)};
        dlgSizer->Add(scrollWin, 1, wxEXPAND);
        dialog.SetSizerAndFit(dlgSizer);

        dialog.ShowModal();
    }, ID_Licenses);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        const auto warnPref{AppState::getPreference(
            AppState::HIDE_EDITOR_MANAGE_VERSIONS_WARN
        )};
        if (
                not mEditors.empty() and
                not warnPref
            ) {
            auto res{pcui::showHideablePrompt(
                _("Although version management can be done with editors open, some information may be lost when adding/removing props."),
                _("Please Close Editors"),
                this,
                wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxCENTER,
                wxEmptyString,
                wxEmptyString,
                _("Proceed")
            )};
            AppState::setPreference(
                AppState::HIDE_EDITOR_MANAGE_VERSIONS_WARN,
                res.wantsToHide_
            );
            if (res.result_ != wxID_OK) return;
        }

        VersionsManager::open(this, AppState::ID_VersionsManager);
    }, ID_ManageVersions);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        ManifestDialog(this).ShowModal();
    }, ID_UpdateManifest);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/blob/master/docs");
    }, ID_Docs);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/issues/new");
    }, ID_Issue);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        if (Close()) {
            Onboard::Frame::instance = new Onboard::Frame;
        }
    }, ID_RunSetup);

    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        mNotifyData.notify(ID_AsyncStart);

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
            choices.insert(
                std::next(choices.begin()),
                boards.begin(),
                boards.end()
            );
            boardSelection.setChoices(std::move(choices));

            progDialog->emitEvent(100, _("Done"));
            mNotifyData.notify(ID_AsyncDone);
        }}.detach();
    }, ID_RefreshDev);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        mNotifyData.notify(ID_AsyncStart);

        auto *progDialog{new Progress(this)};
        progDialog->SetTitle(_("Applying Changes"));
        progDialog->Update(0, _("Initializing..."));

        std::thread{[this, progDialog]() {
            Defer defer{[this]() { mNotifyData.notify(ID_AsyncDone); }};

            progDialog->Update(1, _("Opening Config..."));

            auto *config{Config::getIfOpen(configSelection)};
            const auto configWasOpen{static_cast<bool>(config)};
            if (not configWasOpen) {
                const auto res{Config::open(configSelection)};
                if (const auto *err = std::get_if<string>(&res)) {
                    progDialog->Update(100, _("Error"));
                    auto *evt{new misc::MessageBoxEvent(
                        misc::EVT_MSGBOX, 
                        wxID_ANY, 
                        *err, 
                        _("Cannot Apply Changes")
                    )};
                    wxQueueEvent(this, evt);
                    return;
                }
                config = std::get<Config::Config *>(res);
            }

            const auto res{
                Arduino::applyToBoard(boardSelection, *config, progDialog)
            };

            if (const auto *err = std::get_if<string>(&res)) {
                auto *evt{new misc::MessageBoxEvent(
                    misc::EVT_MSGBOX,
                    wxID_ANY,
                    *err,
                    _("Cannot Apply Changes")
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

            auto *evt{new misc::MessageBoxEvent(
                misc::EVT_MSGBOX,
                wxID_ANY,
                message,
                _("Apply Changes to Board"),
                wxOK | wxICON_INFORMATION
            )};
            wxQueueEvent(this, evt);
        }}.detach();
    }, ID_ApplyChanges);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        if (not SerialMonitor::instance) {
            SerialMonitor::instance = new SerialMonitor(this, boardSelection);
        } else {
            SerialMonitor::instance->Show();
            SerialMonitor::instance->Raise();
        }
    }, ID_OpenSerial);

    configSelection.setUpdateHandler([this](uint32 id) {
        if (id != pcui::ChoiceData::eID_Selection) return;
        mNotifyData.notify(ID_ConfigSelection);
    });
    boardSelection.setUpdateHandler([this](uint32 id) { 
        if (id == pcui::ChoiceData::eID_Choices and boardSelection == -1) {
            boardSelection = 0;
            return;
        }
        if (id != pcui::ChoiceData::eID_Selection) return;

        mNotifyData.notify(ID_BoardSelection);
    });
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        mNotifyData.notify(ID_AsyncStart);

        std::thread{[this]() {
            Defer defer{[&]() { mNotifyData.notify(ID_AsyncDone); }};

            auto res{Config::open(configSelection)};
            if (auto *ptr = std::get_if<string>(&res)) {
                pcui::showMessage(*ptr, _("Cannot Edit Config"));
                return;
            }

            auto *config{std::get<Config::Config *>(res)};
            mConfigNeedShown = config;
        }}.detach();
    }, ID_EditConfig);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { 
        auto addDialog{AddConfig{this}};
        if (addDialog.ShowModal() != wxID_OK) return;

        mNotifyData.notify(ID_AsyncStart);

        auto importPath{static_cast<filepath>(addDialog.importPath_)};
        auto name{static_cast<string>(addDialog.configName_)};
        std::thread{[this, importPath, name]() {
            Defer defer{[&]() { mNotifyData.notify(ID_AsyncDone); }};

            if (importPath.empty()) {
                auto res{Config::open(name)};
                if (auto *err = std::get_if<string>(&res)) {
                    pcui::showMessage(*err, _("Failed Creating Config"));
                    return;
                }
                auto& config{*std::get<Config::Config *>(res)};
                config.save();
                config.close();
            } else {
                auto err{Config::import(name, importPath)};
                if (err) {
                    pcui::showMessage(*err, _("Cannot Import Config"));
                    return;
                }
            }

            updateConfigChoices();
            configSelection = name;
        }}.detach();
    }, ID_AddConfig);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent &) {
        if (pcui::showMessage(
                _("Are you sure you want to deleted the selected configuration?") +
                "\n\n" +
                _("This action cannot be undone!"),
                _("Delete Config"),
                wxYES_NO | wxNO_DEFAULT | wxCENTER,
                this) == wxYES
           ) {
            auto *config{Config::getIfOpen(configSelection)};
            for (auto iter{mEditors.begin()}; iter != mEditors.end(); ++iter) {
                auto *editor{*iter};
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
    bool rebound{id == pcui::Notifier::eID_Rebound};
    if (rebound or id == ID_ConfigSelection) {
        FindWindow(ID_EditConfig)->Enable(configSelection != 0);
        FindWindow(ID_RemoveConfig)->Enable(configSelection != 0);
        FindWindow(ID_ApplyChanges)->Enable(
            configSelection != 0 and boardSelection != 0
        );
    } 
    if (rebound or id == ID_BoardSelection) {
        FindWindow(ID_ApplyChanges)->Enable(
            configSelection != 0 and boardSelection != 0
        );

        bool canOpenSerial{boardSelection != 0};
#       if defined _WIN32 or defined __linux__
        const auto bootloaderIdx{boardSelection.choices().size() - 1};
        canOpenSerial &= boardSelection != bootloaderIdx;
#       endif
        FindWindow(ID_OpenSerial)->Enable(canOpenSerial);
    }

    if (id == ID_AsyncStart) {
        wxSetCursor(wxCURSOR_WAIT);
    }
    if (rebound or id == ID_AsyncDone) {
        if (mConfigNeedShown != nullptr) {
            EditorWindow *editor{nullptr};

            for (auto *listedEditor : mEditors) {
                if (&listedEditor->getOpenConfig() == mConfigNeedShown) {
                    editor = listedEditor;
                    break;
                }
            }

            if (editor == nullptr) {
                editor = new EditorWindow(this, *mConfigNeedShown);
                mEditors.push_back(editor);
            }

            editor->Show();
            editor->Raise();
            mConfigNeedShown = nullptr;
        }

        wxSetCursor(wxNullCursor);
    }
}

void MainMenu::createMenuBar() {
    auto *file{new wxMenu};
    file->Append(ID_ManageVersions, _("Manage Versions..."));
    file->Append(ID_UpdateManifest, _("Update Channel..."));
    file->AppendSeparator();
    file->Append(ID_Logs, _("Show Logs..."));
    file->Append(wxID_ABOUT);
    file->Append(ID_Licenses, _("Licensing Information"));
    file->Append(wxID_EXIT);

    auto* menuBar{new wxMenuBar};
    menuBar->Append(file, _("&File"));
    pcui::Frame::appendDefaultMenuItems(menuBar);

    const auto helpStr{_("&Help")};
    const auto helpIdx{menuBar->FindMenu(helpStr)};
    auto *help{
        helpIdx == wxNOT_FOUND
            ? new wxMenu 
            : menuBar->GetMenu(helpIdx)
    };
    help->Append(
        ID_Docs,
        _("Guides...\tCtrl+H"),
        _("Open the ProffieConfig guides in your web browser")
    );
    // TODO: Make this a page that has my contact info and a button to go to
    // the issues page.
    help->Append(
        ID_Issue,
        _("Help/Bug Report..."),
        _("Open GitHub to submit issue")
    );
    help->AppendSeparator();
    help->Append(ID_RunSetup, _("Re-Run Setup"));
    if (helpIdx == wxNOT_FOUND) menuBar->Append(help, helpStr);

    SetMenuBar(menuBar);
}

void MainMenu::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    auto *headerSection{new wxBoxSizer(wxHORIZONTAL)};

    auto *titleSection{new wxBoxSizer(wxVERTICAL)};
    auto *title{new wxStaticText(this, wxID_ANY, "ProffieConfig")};
    auto titleFont{title->GetFont()};
    titleFont.MakeBold();
#   if defined(__WXGTK__) or defined(__WXMSW__)
    titleFont.SetPointSize(20);
#   elif defined (__WXOSX__)
    titleFont.SetPointSize(30);
#   endif
    title->SetFont(titleFont);
    auto *subTitle{new wxStaticText(
        this, wxID_ANY, _("Created by Ryryog25")
    )};
    titleSection->Add(title);
    titleSection->Add(subTitle);

    auto *appIcon{pcui::createStaticImage(
        this, wxID_ANY, Image::loadPNG("icon")
    )};
    appIcon->SetMaxSize(wxSize{64, 64});

    headerSection->AddSpacer(10);
    headerSection->Add(titleSection);
    headerSection->AddSpacer(20);
    headerSection->AddStretchSpacer(1);
    headerSection->Add(appIcon);
    headerSection->AddSpacer(10);

    auto *configSelectSection{new wxBoxSizer(wxHORIZONTAL)};
    auto *configSelect{new pcui::Choice(this, configSelection)};

    auto *addConfig{new wxButton(
        this,
        ID_AddConfig,
#       ifdef __WXGTK__
        " " + _("Add") + " ",
#       else
        _("Add"),
#       endif
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *removeConfig{new wxButton(
        this,
        ID_RemoveConfig,
#       ifdef __WXGTK__
        " " + _("Remove") + " ",
#       else
        _("Remove"),
#       endif
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
#   if defined _WIN32 or defined __linux__
    boardEntries.emplace_back(_("BOOTLOADER RECOVERY").ToStdString());
#   endif

    boardSelection.setChoices(std::move(boardEntries));
    boardSelection = 0;
    auto *boardSelect{new pcui::Choice(this, boardSelection)};
    boardSelect->SetToolTip(_("Select the Proffieboard to connect to.\nThese IDs are assigned by the OS, and can vary."));

    auto *refreshButton{new wxButton(
        this, ID_RefreshDev, _("Refresh Boards")
    )};
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
    auto *openSerial{new wxButton(
        this, ID_OpenSerial, _("Open Serial Monitor")
    )};

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
#   ifdef __WXMSW__
    // There's a sizing issue I need to figure out... for now we give it a chin
    sizer->AddSpacer(FromDIP(30));
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

