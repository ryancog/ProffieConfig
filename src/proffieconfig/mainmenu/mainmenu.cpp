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

#include <wx/app.h>
#include <wx/gdicmn.h>
#include <wx/menu.h>
#include <wx/utils.h>

#if __WXOSX__
#import <AppKit/AppKit.h>
#endif

#include "config/config.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "dialogs/preferences.hpp"
#include "ui/bitmap.hpp"
#include "ui/builders/vecstack.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/dialogs/progress.hpp"
#include "ui/helpers/busy.hpp"
#include "ui/helpers/data_context.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/panel.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/image.hpp"
#include "ui/static/label.hpp"
#include "ui/utils.hpp"
#include "ui/values.hpp"
#include "utils/paths.hpp"

#include "../core/state.hpp"
#include "../onboard/onboard.hpp"
#include "../editor/editorwindow.hpp"
#include "../tools/arduino.hpp"
#include "../tools/config.hpp"
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

    createMenuBar();
    bindEvents();

    mConfigSel.bind(&config::list());

    config::update();

    pcui::build(this, ui());

    // I don't have a great way with the current layout system to constrain this
    // otherwise :/
    { 
        auto *port_choice{FindWindow(eID_Port_Choice)};
        port_choice->SetMaxSize({port_choice->GetSize().x, -1});
    }

#   if __WXOSX__
    auto view{GetHandle()};
    auto window{view.window};
    window.titleVisibility = NSWindowTitleHidden;
    window.titlebarAppearsTransparent = true;
    window.styleMask |= NSWindowStyleMaskFullSizeContentView;
#   endif

    activate();

#   if _WIN32
    CM_NOTIFY_FILTER filter;
    filter.cbSize = sizeof filter;
    filter.Flags = CM_NOTIFY_FILTER_FLAG_ALL_INTERFACE_CLASSES;
    filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
    std::memset(
        &filter.u.DeviceInterface.ClassGuid,
        0,
        sizeof filter.u.DeviceInterface.ClassGuid
    );
    CM_Register_Notification(
        &filter,
        this,
        pnp_notify_cb,
        &m_pnp_notify_handle
    );
    refresh_boards();
#   elif __APPLE__
    m_io_notify_port = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        IONotificationPortGetRunLoopSource(m_io_notify_port),
        kCFRunLoopDefaultMode
    );
    IOServiceAddMatchingNotification(
        m_io_notify_port,
        kIOFirstMatchNotification,
        IOServiceMatching("IOUSBHostDevice"),
        io_notify_cb,
        this,
        &m_io_device_iter
    );
    // It may take a moment for the serial object to get setup, so an extra
    // notification needs to be setup for that, in addition to the usual usb
    // matching for bootloader.
    IOServiceAddMatchingNotification(
        m_io_notify_port,
        kIOFirstMatchNotification,
        IOServiceMatching("IOSerialBSDClient"),
        io_notify_cb,
        this,
        &m_io_serial_iter
    );
    IOServiceAddMatchingNotification(
        m_io_notify_port,
        kIOTerminatedNotification,
        IOServiceMatching("IOUSBHostDevice"),
        io_notify_cb,
        this,
        &m_io_removed_iter
    );
    io_notify_cb(this, m_io_device_iter);
    io_notify_cb(this, m_io_serial_iter);
    io_notify_cb(this, m_io_removed_iter);
#   endif

    Show();
}

MainMenu::~MainMenu() {
#   if _WIN32
    CM_Unregister_Notification(m_pnp_notify_handle);
#   elif __APPLE__
    IOObjectRelease(m_io_device_iter);
    IOObjectRelease(m_io_serial_iter);
    IOObjectRelease(m_io_removed_iter);

    CFRunLoopRemoveSource(
        CFRunLoopGetCurrent(),
        IONotificationPortGetRunLoopSource(m_io_notify_port),
        kCFRunLoopDefaultMode
    );

    IONotificationPortDestroy(m_io_notify_port);
#   endif

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
    auto button_font{- pcui::Font::Normal};
    button_font.SetWeight(wxFONTWEIGHT_BOLD);
#   if __WXMSW__
    button_font.SetPointSize(12);
#   else
    button_font.SetPointSize(14);
#   endif

    const auto button_bitmap{[](cstring str) {
        pcui::Bitmap bmp(str);
        bmp.color(color::UserAccent{});
        bmp.scaleTo(24);
        return bmp.realize();
    }};

    return pcui::Stack{
      .base_={
        .minSize_={800, 550},
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .base_={
            .expand_=true,
            .border_={
              .size_=pcui::winEdgeSpacing(),
              .dirs_=wxALL
            },
          },
          .orient_=wxVERTICAL,
          .children_={
#           if __WXOSX__
            // Spacer past the traffic light region.
            // Combined with the usual edge border should give a proper result.
            pcui::Spacer{
              .size_=[this] {
                  auto view{GetHandle()};
                  auto window{view.window};
                  // The contentLayoutRect is in non-flipped coordinates. That
                  // is, they originate from the bottom left, so to figure out
                  // the spacing, compute against frame height.
                  auto content_height{window.contentLayoutRect.size.height};
                  auto frame_height{window.frame.size.height};
                  auto safe_inset{frame_height - content_height};
                  // It looks a little too far spaced out though, so also
                  // subtract just a little.
                  return static_cast<int32>(safe_inset) - 4;
              }()
            }(),
#           endif
            pcui::Stack{
              .base_={.expand_=true},
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Stack{
                  .orient_=wxVERTICAL,
                  .children_={
                    pcui::Label{
                      .label_="ProffieConfig",
#                     if defined(__WXGTK__) or defined(__WXMSW__)
                      .font_=wxFontInfo{20}.Bold(),
#                     elif defined (__WXOSX__)
                      .font_=wxFontInfo{30}.Bold(),
#                     endif
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
            pcui::StretchSpacer{.size_=20, .proportion_=1}(),
            pcui::Group{
              .win_={
                .base_={.expand_=true},
              },
              .label_=_("Connected Boards"),
              .children_={
                pcui::Choice{
                  .win_={
                    .base_={.expand_=true},
                    .id_=eID_Port_Choice,
                    .tooltip_=_("Select the Proffieboard to connect to.\nThese IDs are assigned by the OS, and can vary."),
                  },
                  .data_=mBoardChoice,
                  .style_=pcui::Choice::PopUp{
                    .unselected_=_("Select Port..."),
                  },
                  .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                      auto& board{mBoards[idx]};
                      switch (board.type_) {
                          using enum arduino::Port::Type;
                          case Bootloader:
                              return _("Bootloader");
                          case Proffie:
                              return wxString::Format(
                                    _("Proffieboard (%s)"),
                                    board.serial_path_
                              );
                      }

                      std::unreachable();
                  }
                }(),
              }
            }(),
            pcui::StretchSpacer{.size_=20, .proportion_=3}(),
            pcui::Button{
              .win_={
                .base_={.expand_=true},
                .enable_={
                  mConfigSel.choice() | data::logic::HasSelection{} and
                  mBoardChoice | data::logic::HasSelection{}
                },
                .tooltip_=_("Compile and upload the selected configuration to the selected Proffieboard."),
              },
              .label_=_("Apply Config"),
              .bitmap_={
                .src_=button_bitmap("chip"),
                .mode_=pcui::Button::BitmapMode::Maintain,
              },
              .font_=button_font,
              .func_=[this] { onApplyConfig(); },
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.expand_=true},
                .enable_=mBoardChoice | data::logic::HasSelection{{},
                  [this](uint32 idx) {
                      return mBoards[idx].type_ == arduino::Port::Type::Proffie;
                  }
                },
              },
              .label_=_("Serial Monitor"),
              .bitmap_={
                .src_=button_bitmap("serial_monitor"),
                .mode_=pcui::Button::BitmapMode::Maintain,
              },
              .font_=button_font,
              .func_=[this] { onOpenSerial(); },
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Button{
              .win_={.base_={.expand_=true}},
              .label_=_("Version Management"),
              .bitmap_={
                .src_=button_bitmap("management"),
                .mode_=pcui::Button::BitmapMode::Maintain,
              },
              .font_=button_font,
              .func_=[this] { on_manage_versions(); },
            }(),
          },
        }(),
        pcui::Divider{}(),
        pcui::Panel{
          .win_={
            .base_={
              .expand_=true,
              .proportion_=1,
            },
          },
          .tint_=pcui::Panel::Tint::Content,
          .child_=pcui::Stack{
            .base_={
              .expand_=true,
              .proportion_=1,
              .border_={
                .size_=pcui::winEdgeSpacing(),
                .dirs_=wxBOTTOM
              },
            },
            .orient_=wxVERTICAL,
            .children_={
              pcui::Scrolled{
                .win_={
                  .base_={.expand_=true, .proportion_=1},
                  .on_activate_=[this](wxWindow *win) {
                      // Unselect when clicking background
                      const auto on_click{[this](wxMouseEvent&) {
                          mConfigSel.choice().unchoose();
                      }};
                      win->Bind(wxEVT_LEFT_UP, on_click);
                  }
                },
                .scrollRate_{.y_=4},
                .child_=pcui::builders::VecStack{
                  .base_={
                    .expand_=true,
                    .proportion_=1,
                    .border_{
                     .size_=pcui::winEdgeSpacing(),
                     .dirs_=wxALL,
                    },
                  },
                  .data_=config::list(),
                  .builder_=[this](data::base::Model& model) {
                      return config_entry(model);
                  },
                  .separator_=pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                }(),
              }(),
              pcui::Divider{
                .base_={
                  .border_={
                    .size_=pcui::winEdgeSpacing(),
                    .dirs_=wxLEFT | wxRIGHT
                  },
                },
              }(),
              pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
              pcui::Stack{
                .base_={
                  .expand_=true,
                  .border_={
                    .size_=pcui::winEdgeSpacing(),
                    .dirs_=wxLEFT | wxRIGHT,
                  },
                },
                .orient_=wxHORIZONTAL,
                .children_={
                  pcui::StretchSpacer{}(),
                  pcui::Button{
                    .label_=_("Add Config"),
                    .bitmap_={
                      .src_=[] {
                          pcui::Bitmap bmp("new");
                          bmp.color(color::UserAccent{});
                          bmp.scaleTo(20);
                          return bmp.realize();
                      }(),
                      .mode_=pcui::Button::BitmapMode::Maintain,
                    },
                    .func_=[this] { on_add_config(); },
                  }(),
                }
              }(),
            },
          }(),
        }(),
      }
    }();
}

void MainMenu::createMenuBar() {
    auto *file{new wxMenu};
#   ifdef __WXOSX__
    // TODO: The wxWidgets labeling is broken and, if it did work, outdated
    file->Append(wxID_PREFERENCES, _("Settings...") + "\tCtrl+,");
#   else
    file->Append(wxID_PREFERENCES);
#   endif
    file->Append(eID_Update_Manifest, _("Update Channel..."));
    file->AppendSeparator();
    file->Append(eID_Logs, _("Show Logs..."));
#   ifdef __WXOSX__
    // TODO: The wxWidgets labeling is broken
    file->Append(wxID_ABOUT, _("About ProffieConfig"));
#   else
    file->Append(wxID_ABOUT);
#   endif
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
        eID_Guides,
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
                    pcui::guiDataContext(info->config()->isSaved()).val()
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

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        if (not mPrefsDlg)
            mPrefsDlg = new PreferencesDlg;

        mPrefsDlg->show(this);
    }, wxID_PREFERENCES);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        wxLaunchDefaultApplication(paths::logDir().string());
    }, eID_Logs);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        LicenseDialog dlg(this);
        dlg.ShowModal();
    }, eID_Licenses);

    Bind(wxEVT_MENU, [&](wxCommandEvent &) {
        ManifestDialog(this).ShowModal();
    }, eID_Update_Manifest);

    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser(paths::website() + "/guides");
    }, eID_Guides);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        wxLaunchDefaultBrowser("https://github.com/ryancog/ProffieConfig/issues/new");
    }, eID_Issue);
    Bind(wxEVT_MENU, [&](wxCommandEvent&) {
        if (Close()) {
            onboard::Frame::instance = new onboard::Frame;
        }
    }, eID_Run_Setup);

    static const auto board_choice_table{[] {
        data::prim::Choice::RecvTable table;
        table.onChoice_ = data::map<&MainMenu::on_board_choice>();
        return table;
    }()};
    observeWith(mBoardChoice, board_choice_table);
}

void MainMenu::refresh_boards() {
    std::thread{[this]() {
        auto choice{data::context(mBoardChoice)};

        // Besides just resetting the count to zero while doing the update,
        // this is necessary in the case where the same number of boards is
        // reported. If the choice wasn't cleared here, it'll never re-fetch
        // the string(s).
        choice.update(0);

        mBoards = arduino::getBoards();

        int32 new_idx{-1};
        for (size idx{0}; idx < mBoards.size(); ++idx) {
            if (mBoards[idx].serial_path_ == m_last_serial_selected) {
                new_idx = static_cast<int32>(idx);
                break;
            }
        }

        if (new_idx == -1 and not mBoards.empty())
            new_idx = 0;

        choice.update(mBoards.size(), new_idx);
    }}.detach();
}

void MainMenu::on_board_choice() {
    auto board_choice{pcui::guiDataContext(mBoardChoice)};
    if (board_choice.idx() < 0)
        return;

    const auto& port{mBoards[board_choice.idx()]};
    if (port.type_ != arduino::Port::Type::Proffie)
        return;

    m_last_serial_selected = port.serial_path_;
}

void MainMenu::on_add_config() {
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

        auto configSel{pcui::guiDataContext(mConfigSel)};
        auto vec{pcui::guiDataContext(*configSel.bound())};
        auto choice{pcui::guiDataContext(mConfigSel.choice())};

        for (size idx{0}; idx < vec.children().size(); ++idx) {
            auto& info{dynamic_cast<config::Info&>(*vec.children()[idx])};
            auto nameCtxt{pcui::guiDataContext(info.name())};

            if (nameCtxt.val() != result.name_) continue;

            choice.choose(static_cast<int32>(idx));
            break;
        }
    }}.detach();
}

bool MainMenu::check_config_valid(
    const data::base::Vector::ROContext& configs,
    config::Info& info
) {
    // In case the UI desyncs, makes sure info is actually an object in the list
    // and isn't dead memory. Worst case the data was reallocated and we just
    // end up dealing with some other (valid) config.
    for (const auto& model : configs.children()) {
        if (model.get() == &info)
            return true;
    }

    return false;
}

void MainMenu::on_remove_config(config::Info& info) {
    auto configs{pcui::guiDataContext(config::list())};
    if (not check_config_valid(configs, info))
        return;

    const auto message{wxString::Format(
        _("Are you sure you want to delete the configuration \"%s\"?") +
        "\n\n" +
        _("This action cannot be undone!"),
        pcui::guiDataContext(info.name()).val()
    )};
    pcui::dialogs::message::Args args{
        .caption_=_("Delete Config"),
        .style_=wxYES_NO | wxNO_DEFAULT | wxCENTER,
        .parent_=this,
    };

    if (pcui::showMessage(message, args) != wxYES)
        return;

    auto iter{mEditors.find(&info)};
    if (iter != mEditors.end())
        iter->second->Close(true);

    auto path{info.path()};
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
    info.unload();
}

void MainMenu::on_edit_config(config::Info& info) {
    auto configs{pcui::guiDataContext(config::list())};
    if (not check_config_valid(configs, info))
        return;

    auto err{info.load()};
    if (err) {
        pcui::showMessage(*err);
        return;
    }

    auto iter{mEditors.find(&info)};
    if (iter == mEditors.end()) {
        auto *editor{new EditorWindow(info)};

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

void MainMenu::onApplyConfig() {
    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Applying Changes"),
        true
    )};

    bool clean{wxGetKeyState(WXK_CONTROL)};

    std::thread{[this, prog, busy, clean]() {
        prog->set(1, _("Opening Config..."));

        auto cfgSel{pcui::guiDataContext(mConfigSel)};
        auto boardChoice{pcui::guiDataContext(mBoardChoice)};

        auto *info{cfgSel.selected<config::Info>()};
        if (info == nullptr) {
            prog->finish(false, _("Cancelled"));
            return;
        }

        if (auto err{info->load()}) {
            prog->finish(true, *err);
            return;
        }

        auto& config{*info->config()};
        extraConfigOptionProcessing(config);
        auto& compInfo{arduino::getCacheInfo(config, clean)};

        const auto& port{mBoards[boardChoice.idx()]};

        // If there's an open serial monitor for this port, it needs to be
        // suspended so that the board can be accessed for reboot.
        SerialMonitorDlg *suspended_monitor{nullptr};
        for (auto *monitor : mMonitors) {
            if (monitor->device() == port.serial_path_) {
                suspended_monitor = monitor;
                suspended_monitor->disconnect();
                break;
            }
        }

        auto name{pcui::guiDataContext(info->name())};
        arduino::applyToBoard(
            name.val(),
            port,
            compInfo,
            *prog
        );

        if (suspended_monitor)
            suspended_monitor->reconnect();
    }}.detach();
}

void MainMenu::onOpenSerial() {
    auto boardChoice{pcui::guiDataContext(mBoardChoice)};
    auto& boardPort{mBoards[boardChoice.idx()]};

    for (auto *monitor : mMonitors) {
        if (monitor->device() != boardPort.serial_path_)
            continue;

        monitor->Show();
        monitor->Raise();
        return;
    }

    auto *dlg{new SerialMonitorDlg(this, boardPort.serial_path_)};

    const auto onClose{[dlg, this](wxCloseEvent& evt) {
        evt.Skip();

        if (evt.GetEventObject() != dlg)
            return;

        mMonitors.erase(dlg);
    }};
    dlg->Bind(wxEVT_CLOSE_WINDOW, onClose);

    mMonitors.insert(dlg);

    dlg->Show();
}

void MainMenu::on_manage_versions() {
    if (mVersionsDlg) {
        mVersionsDlg->Show();
        mVersionsDlg->Raise();
        return;
    }

    const auto warnPref{state::prefs::get(
        state::prefs::Bool::Hide_Editor_Manage_Versions_Warn
    )};
    if (
            not mEditors.empty() and
            not warnPref
        ) {
        auto res{pcui::showHideablePrompt(
            _(
                "Although version management can be done with editors open, the changes won't be visible\n"
                "until the editor is reloaded, and you might run into oddities."
            ),
            {
                .caption_=_("Please Close Editors"),
                .style_=wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxCENTER,
                .labels_={.ok_=_("Proceed")},
                .parent_=this,
            }
        )};
        state::prefs::set(
            state::prefs::Bool::Hide_Editor_Manage_Versions_Warn,
            res.wantsHide_
        );
        if (res.id_ != wxID_OK) return;
    }

    mVersionsDlg = new VersionsDlg(this);
    const auto onDestroy{[this](wxWindowDestroyEvent& evt) {
        if (evt.GetEventObject() == mVersionsDlg)
            mVersionsDlg = nullptr;
    }};
    mVersionsDlg->Bind(wxEVT_DESTROY, onDestroy);

    mVersionsDlg->Show();
}

pcui::DescriptorPtr MainMenu::config_entry(data::base::Model& model) {
    auto& info{dynamic_cast<config::Info&>(model)};

    const auto button_bitmap{[](cstring str) {
        pcui::Bitmap bmp(str);
        bmp.color(color::UserAccent{});
        bmp.scaleTo(20);
        return bmp.realize();
    }};

    return pcui::Group{
      .win_={
        .base_={.expand_=true},
        .on_activate_=[this, &info](wxWindow *win) {
            const auto on_click{[this, &info](wxMouseEvent&) {
                auto config_list{pcui::guiDataContext(config::list())};
                auto configs{config_list.children()};
                for (size idx{0}; idx < configs.size(); ++idx) {
                    if (&info == configs[idx].get()) {
                        mConfigSel.choice().choose(static_cast<int32>(idx));
                        break;
                    }
                }
            }};
            auto *label{win->FindWindow(eID_Config_Entry_Label)};
            auto *divider{win->FindWindow(eID_Config_Entry_Line)};

            // Select this item when clicked.
            label->Bind(wxEVT_LEFT_UP, on_click);
            divider->Bind(wxEVT_LEFT_UP, on_click);
            auto *parent{divider->GetParent()};
            while (not false) {
                parent->Bind(wxEVT_LEFT_UP, on_click);

                if (parent == win)
                    break;

                parent = parent->GetParent();
            }

            struct Receiver : wxSizerItem, data::Receiver {
                Receiver(
                    MainMenu *mainmenu,
                    wxWindow *window,
                    config::Info& info
                ) : wxSizerItem(0, 0),
                    m_mainmenu{mainmenu},
                    m_window{window},
                    m_info(info) {}

                void onActivate() override {
                    on_choice();
                }

                void on_choice() {
                    auto config_sel{pcui::guiDataContext(
                        m_mainmenu->mConfigSel
                    )};

                    const auto color{config_sel.selected() == &m_info
                        ? color::Dynamic(color::UserAccent{}).color()
                        : wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)
                    };
                    auto *divider{m_window->FindWindow(eID_Config_Entry_Line)};
                    // You'd think this would be foreground, but nope.
                    divider->SetBackgroundColour(color);
                    divider->Refresh();
                }

            private:
                MainMenu *m_mainmenu;
                wxWindow *m_window;
                config::Info& m_info;
            };

            // Handle selection.
            auto *receiver{new Receiver(this, win, info)};
            pcui::find_sizer(win)->Add(receiver);

            static const auto choice_table{[] {
                data::prim::Choice::RecvTable table;
                table.onChoice_ = data::map<&Receiver::on_choice>();
                return table;
            }()};
            receiver->observeWith(mConfigSel.choice(), choice_table);
            receiver->activate();

            // Receiver will be deactivated with normal crippling and destroyed
            // as part of the window hierarchy.
        },
      },
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .base_={.proportion_=1},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .win_={
                .base_={.expand_=true, .proportion_=1},
                .id_=eID_Config_Entry_Label,
                .maxSize_={300, -1},
              },
              .label_=info.name(),
              .font_=(- pcui::Font::Normal).MakeBold(),
              .ellipsize_=pcui::Label::Ellipsize::Middle,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            // Can't recolor Divider on MSW, so just keep things consistent,
            // and use an empty panel everywhere.
            pcui::Panel{
              .win_={
                .base_={
                  .minSize_={-1, 3},
                  .expand_=true
                },
                .id_=eID_Config_Entry_Line,
              },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Button{
          .win_={.base_={.expand_=true}},
          .bitmap_={
            .src_=button_bitmap("edit"),
            .mode_=pcui::Button::BitmapMode::Maintain,
          },
          .exactFit_=true,
          .func_=[this, &info] { on_edit_config(info); },
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Button{
          .win_={.base_={.expand_=true}},
          .bitmap_={
            .src_=button_bitmap("trash"),
            .mode_=pcui::Button::BitmapMode::Maintain,
          },
          .exactFit_=true,
          .func_=[this, &info] {
              // Must be deferred since it'll destroy this UI.
              CallAfter([this, &info] {
                  on_remove_config(info);
              });
          }
        }(),
      }
    }();
}

#if _WIN32
DWORD CALLBACK MainMenu::pnp_notify_cb(
    HCMNOTIFICATION,
    PVOID arg,
    CM_NOTIFY_ACTION,
    PCM_NOTIFY_EVENT_DATA,
    DWORD
) {
    reinterpret_cast<MainMenu *>(arg)->refresh_boards();
    return ERROR_SUCCESS;
}
#elif __APPLE__
void MainMenu::io_notify_cb(void *arg, io_iterator_t iter) {
    // Drain the iterator to re-arm the notification
    while (not false) {
        auto object{IOIteratorNext(iter)};
        if (object == IO_OBJECT_NULL)
            break;

        IOObjectRelease(object);
    }

    reinterpret_cast<MainMenu *>(arg)->refresh_boards();
}
#endif

