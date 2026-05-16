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

#include <wx/gdicmn.h>
#include <wx/menu.h>
#include <wx/utils.h>

#include "config/config.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/bitmap.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/dialogs/progress.hpp"
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
#include "../tools/arduino.hpp"
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

    mConfigSel.bind(&config::list());

    config::update();

    pcui::build(this, ui());

    Show();
}

MainMenu::~MainMenu() {
    pcui::cripple(this);
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
                  .font_=wxFontInfo{20}.Bold(),
#                 elif defined (__WXOSX__)
                  .font_=wxFontInfo{30}.Bold(),
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
              .data_=mConfigSel,
              .style_=pcui::Choice::PopUp{
                .unselected_=_("Select Config"),
              },
              .labeler_=[this](uint32 sel) -> pcui::Choice::Label {
                  auto vec{data::context(config::list())};
                  if (sel >= vec.children().size()) return {};

                  auto& info{dynamic_cast<config::Info&>(*vec.children()[sel])};
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
              .func_=[this] { onAddConfig(); },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.expand_=true},
                .enable_=mConfigSel.choice() | data::logic::HasSelection{},
              },
              .label_=_("Remove"),
              .exactFit_=true,
              .func_=[this] { onRemoveConfig(); },
            }(),
          },
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .enable_=mConfigSel.choice() | data::logic::HasSelection{},
          },
          .label_=_("Edit Selected Configuration"),
          .func_=[this] { onEditConfig(); },
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
              .func_=[this] { onRefreshBoards(); },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Choice{
              .win_={
                .base_={.proportion_=1},
                .tooltip_=_("Select the Proffieboard to connect to.\nThese IDs are assigned by the OS, and can vary."),
              },
              .data_=mBoardChoice,
              .style_=pcui::Choice::PopUp{
                .unselected_=_("No Port (BOOTLOADER)"),
              },
              .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                  return mBoards[idx];
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .enable_=mConfigSel.choice() | data::logic::HasSelection{},
            .tooltip_=_("Compile and upload the selected configuration to the selected Proffieboard."),
          },
          .label_=_("Apply Selected Configuration to Board"),
          .func_=[this] { onApplyConfig(); },
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={
            .base_={.expand_=true},
            .enable_=mBoardChoice | data::logic::HasSelection{},
          },
          .label_=_("Open Serial Monitor"),
          .func_=[this] { onOpenSerial(); },
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
                    data::context(info->config()->isSaved()).val()
               ) {
                continue;
            }

            auto res{pcui::showMessage(
                _("There is at least one editor open with unsaved changes.") + '\n' +
                _("Are you sure you want to exit?") + "\n\n"+
                _("All unsaved changes will be lost!"),
                {
                    .caption_=_("Open Editor(s)"),
                    .style_=wxYES_NO | wxNO_DEFAULT | wxCENTER | wxICON_EXCLAMATION
                }
            )};
            if (res != wxYES) return false;
            break;
        }

        return true;
    }};

    Bind(wxEVT_CLOSE_WINDOW, [this, promptClose](wxCloseEvent& event) {
        if (event.CanVeto() and not promptClose()) {
            event.Veto();
            return;
        }

        state::saveState();

        for (auto [info, editor] : mEditors)
            editor->Close(true);

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
                {
                    .caption_=_("Please Close Editors"),
                    .style_=wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxCENTER,
                    .labels_={.ok_=_("Proceed")},
                    .parent_=this,
                }
            )};
            state::setPreference(
                state::ePreference_Hide_Editor_Manage_Versions_Warn,
                res.wantsHide_
            );
            if (res.id_ != wxID_OK) return;
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
}

void MainMenu::onAddConfig() {
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
                    {.caption_=_("Config Add Error")}
                );
                return;
            }

            config::update();
        } else {
            auto err{config::import(result.name_, result.path_)};
            if (err) {
                pcui::showMessage(*err, {.caption_=_("Import Error")});
            }
        }

        auto configSel{data::context(mConfigSel)};
        auto vec{data::context(*configSel.bound())};
        auto choice{data::context(mConfigSel.choice())};

        for (size idx{0}; idx < vec.children().size(); ++idx) {
            auto& info{dynamic_cast<config::Info&>(*vec.children()[idx])};
            auto nameCtxt{data::context(info.name())};

            if (nameCtxt.val() != result.name_) continue;

            choice.choose(static_cast<int32>(idx));
            break;
        }
    }}.detach();
}

void MainMenu::onRemoveConfig() {
    const auto message{
        _("Are you sure you want to deleted the selected configuration?") +
        "\n\n" +
        _("This action cannot be undone!"),
    };
    pcui::dialogs::message::Args args{
        .caption_=_("Delete Config"),
        .style_=wxYES_NO | wxNO_DEFAULT | wxCENTER,
        .parent_=this,
    };

    if (pcui::showMessage(message, args) != wxYES)
        return;

    auto sel{data::context(mConfigSel)};
    auto *info{sel.selected<config::Info>()};

    auto iter{mEditors.find(info)};
    if (iter != mEditors.end())
        iter->second->Close(true);

    auto path{info->path()};
    std::error_code ec;
    if (not fs::remove(path, ec)) {
        pcui::showMessage(
            wxString::Format(
                _("The configuration could not be deleted: %s"),
                ec.message()
            ),
            {
                .caption_=_("Delete Config"),
                .style_=wxOK | wxICON_ERROR
            }
        );
        return;
    }

    // Unloading will remove the config from the list once it's deleted from
    // disk.
    info->unload();
}

void MainMenu::onEditConfig() {
    auto sel{data::context(mConfigSel)};
    auto& info{*sel.selected<config::Info>()};

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

void MainMenu::onRefreshBoards() {
    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(this, _("Board Refresh"))};

    std::thread{[this, prog, busy]() {
        auto choice{data::context(mBoardChoice)};

        std::optional<std::string> last;
        if (choice.idx() >= 0)
            last = mBoards[choice.idx()];

        prog->pulse(_("Discovering Boards..."));
        mBoards = arduino::getBoards();

        prog->set(90, _("Processing and Finalizing..."));

        int32 newIdx{-1};
        for (size idx{0}; idx < mBoards.size(); ++idx) {
            if (mBoards[idx] == last) {
                newIdx = static_cast<int32>(idx);
                break;
            }
        }

        choice.update(mBoards.size(), newIdx);

        prog->finish(false);
    }}.detach();
}

void MainMenu::onApplyConfig() {
    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(this, _("Applying Changes"))};

    std::thread{[this, prog, busy]() {
        prog->set(1, _("Opening Config..."));

        auto cfgSel{data::context(mConfigSel)};
        auto boardChoice{data::context(mBoardChoice)};

        auto *info{cfgSel.selected<config::Info>()};
        if (info == nullptr or boardChoice.idx() == -1) {
            prog->finish(false, _("Cancelled"));
            return;
        }

        if (auto err{info->load()}) {
            prog->finish(true, *err);
            return;
        }

        arduino::applyToBoard(
            mBoards[boardChoice.idx()],
            *info->config(),
            *prog
        );
    }}.detach();
}

void MainMenu::onOpenSerial() {
}

