#include "mainmenu.hpp"
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

#include <fstream>
#include <thread>

#include <wx/menu.h>
#include <wx/utils.h>

#include "config/config.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/bitmap.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/helpers/busy.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/static/image.hpp"
#include "ui/static/label.hpp"
#include "ui/values.hpp"
#include "utils/paths.hpp"

#include "../core/state.hpp"
#include "../onboard/onboard.hpp"
#include "../editor/editorwindow.hpp"
#include "dialogs/about.hpp"
#include "dialogs/addconfig.hpp"
#include "dialogs/licenses.hpp"
#include "dialogs/manifest.hpp"

MainMenu *MainMenu::instance{nullptr};

MainMenu::MainMenu(wxWindow* parent) : 
    pcui::Frame(
        parent,
        state::eID_Main_Menu,
        "ProffieConfig",
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)
    ) {

    // REVIEW
    // boardSelection.setPersistence(pcui::ChoiceData::Persistence::String);
    // configSelection.setPersistence(pcui::ChoiceData::Persistence::String);

    createMenuBar();
    bindEvents();

    { data::Selector::Context ctxt{configSel_};
        ctxt.bind(&config::list());
    }

    config::update();

    pcui::build(this, ui());

    Show();
}

MainMenu::~MainMenu() {
    pcui::teardown(this);
}

void MainMenu::removeEditor(EditorWindow *editor) {
    for (auto it{mEditors.begin()}; it != mEditors.end(); ++it) {
        if (it->second == editor) {
            mEditors.erase(it);
            break;
        }
    }
}

pcui::DescriptorPtr MainMenu::ui() {
    return pcui::Stack{
      .base_={
        // This is such a small and dense window the usual spacing looks
        // weird here.
        .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
      },
      .children_={
        pcui::Stack{
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Stack{
              .children_={
                pcui::Label{
                  .label_="ProffieConfig",
#                 if defined(__WXGTK__) or defined(__WXMSW__)
                  .style_=wxFontInfo{20}.Bold(),
#                 elif defined (__WXOSX__)
                  .style_=wxFontInfo{30}.Bold(),
#                 endif
                }(),
                pcui::Label{
                  .label_=_("Created by Ryryog25"),
                }(),
              },
            }(),
            pcui::Spacer{.size_=20}(),
            pcui::StretchSpacer{}(),
            pcui::Image{
              .win_={.maxSize_={64, 64}},
              .src_=pcui::Bitmap("icon"),
            }()
          },
        }(),
        pcui::Spacer{.size_=20}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Choice{
              .win_={.base_={.proportion_=1}},
              .data_=configSel_.choice_,
              .style_=pcui::Choice::PopUp{
                .unselected_=_("Select Config"),
              },
              .labeler_=[this](uint32 sel) -> pcui::Choice::Label {
                  data::Vector::ROContext vec{config::list()};
                  if (sel >= vec.children().size()) return {};

                  auto& info{static_cast<config::Info&>(*vec.children()[sel])};
                  return info.name();
              }
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              // On GTK, the exactfit shrinks button height smaller than the
              // PopUp Choice
              .win_={.base_={.expand_=true}},
              .label_=_("Add"),
              .exactFit_=true,
              .func_=[this] { importConfig(); },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.expand_=true},
                .enable_=configSel_.choice_ | data::logic::HasSelection{},
              },
              .label_=_("Remove"),
              .exactFit_=true,
            }(),
          },
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .enable_=configSel_.choice_ | data::logic::HasSelection{},
          },
          .label_=_("Edit Selected Configuration"),
          .func_=[this] {
              data::Choice::ROContext choice{configSel_.choice_};
              data::Vector::ROContext vec{config::list()};
              if (choice.choice() >= vec.children().size()) return;

              auto& info{static_cast<config::Info&>(
                  *vec.children()[choice.choice()]
              )};

              auto err{info.load()};
              if (err) {
                  pcui::showMessage(*err);
                  return;
              }

              auto iter{mEditors.find(&info)};
              if (iter == mEditors.end()) {
                  auto *editor{new EditorWindow(this, info)};

                  const auto onDestroy{[this, editor](
                      wxWindowDestroyEvent& evt
                  ) {
                      evt.Skip();
                      if (evt.GetEventObject() != editor) return;

                      removeEditor(editor);
                  }};
                  editor->Bind(wxEVT_DESTROY, onDestroy);

                  mEditors[&info] = editor;

                  editor->Show();
              } else {
                  iter->second->Raise();
              }
          }
        }(),
        pcui::Spacer{.size_=20}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Button{
              .win_={
                .tooltip_=_("Generate an up-to-date list of connected boards."),
              },
              .label_=_("Refresh Boards"),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Choice{
              .win_={
                .base_={.proportion_=1},
                .tooltip_=_("Select the Proffieboard to connect to.\nThese IDs are assigned by the OS, and can vary."),
              },
              .data_=board_,
              .style_=pcui::Choice::PopUp{
                .unselected_=_("Select Board"),
              },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .enable_={
              configSel_.choice_ | data::logic::HasSelection{} and
              board_ | data::logic::HasSelection{}
            },
            .tooltip_=_("Compile and upload the selected configuration to the selected Proffieboard."),
          },
          .label_=_("Apply Selected Configuration to Board"),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .enable_=board_ | data::logic::HasSelection{},
          },
          .label_=_("Open Serial Monitor"),
        }(),
      }
    }();

#   if defined _WIN32 or defined __linux__
    // boardEntries.emplace_back(_("BOOTLOADER RECOVERY").ToStdString());
#   endif
}

void MainMenu::createMenuBar() {
    auto *file{new wxMenu};
    file->Append(eID_Manage_Versions, _("Manage Versions..."));
    file->Append(eID_Update_Manifest, _("Update Channel..."));
    file->AppendSeparator();
    file->Append(eID_Logs, _("Show Logs..."));
    file->Append(wxID_ABOUT);
    file->Append(eID_Licenses, _("Licensing Information"));
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
        eID_Docs,
        _("Guides...\tCtrl+H"),
        _("Open the ProffieConfig guides in your web browser")
    );
    // TODO: Make this a page that has my contact info and a button to go to
    // the issues page.
    help->Append(
        eID_Issue,
        _("Help/Bug Report..."),
        _("Open GitHub to submit issue")
    );
    help->AppendSeparator();
    help->Append(eID_Run_Setup, _("Re-Run Setup"));
    if (helpIdx == wxNOT_FOUND) menuBar->Append(help, helpStr);

    SetMenuBar(menuBar);
}

void MainMenu::bindEvents() {
    auto promptClose{[this]() -> bool {
        for (auto [info, editor] : mEditors) {
            if (
                    info->config() and
                    data::Bool::ROContext{info->config()->isSaved()}.val()
               ) {
                continue;
            }

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

        return true;
    }};

    Bind(wxEVT_CLOSE_WINDOW, [promptClose](wxCloseEvent& event) {
        state::saveState();

        if (event.CanVeto() and not promptClose()) {
            event.Veto();
            return;
        }
        event.Skip();
    });

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        Close(true);
    }, wxID_EXIT);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        showAbout(this);
    }, wxID_ABOUT);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxLaunchDefaultApplication(paths::logDir().native());
    }, eID_Logs);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        LicenseDialog dlg(this);
        dlg.ShowModal();
    }, eID_Licenses);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        const auto warnPref{state::getPreference(
            state::ePreference_Hide_Editor_Manage_Versions_Warn
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
            state::setPreference(
                state::ePreference_Hide_Editor_Manage_Versions_Warn,
                res.wantsToHide_
            );
            if (res.result_ != wxID_OK) return;
        }

        // REVIEW
        // versions_manager::open(this, state::eID_Versions_Manager);
    }, eID_Manage_Versions);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        ManifestDialog(this).ShowModal();
    }, eID_Update_Manifest);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/blob/master/docs");
    }, eID_Docs);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/issues/new");
    }, eID_Issue);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        if (Close()) {
            onboard::Frame::instance = new onboard::Frame;
        }
    }, eID_Run_Setup);

    /*
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
    }, eID_Refresh_Dev);

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

            auto *evt{new misc::MessageBoxEvent(
                misc::EVT_MSGBOX,
                wxID_ANY,
                message,
                _("Apply Changes to Board"),
                wxOK | wxICON_INFORMATION
            )};
            wxQueueEvent(this, evt);
        }}.detach();
    }, eID_Apply_Changes);
    */

    /*
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
    }, eID_Remove_Config);
    */
}

void MainMenu::importConfig() {
    AddConfigDialog dlg(this);
    if (dlg.ShowModal() != wxID_OK) return;

    pcui::BusyTracker busy(this);

    std::thread{[this, busy, result=dlg.getResult()] {
        using Result = AddConfigDialog::Result;

        if (result.mode_ == Result::Mode::Create) {
            const auto filename{result.name_ + config::RAW_FILE_EXTENSION};
            std::ofstream file{paths::configDir() / filename};

            if (file.fail()) {
                pcui::showMessage(
                    _("File could not be created."),
                    _("Config Add Error")
                );
                return;
            }

            config::update();
        } else {
            auto err{config::import(result.name_, result.path_)};
            if (err) {
                pcui::showMessage(*err, _("Import Error"));
            }
        }

        data::Selector::Context configSel{configSel_};
        data::Vector::ROContext vec{*configSel.bound()};
        data::Choice::Context choice{configSel_.choice_};

        for (size idx{0}; idx < vec.children().size(); ++idx) {
            auto& info{static_cast<config::Info&>(*vec.children()[idx])};
            data::String::ROContext nameCtxt{info.name()};

            if (nameCtxt.val() != result.name_) continue;

            choice.choose(static_cast<int32>(idx));
            break;
        }
    }}.detach();
}

