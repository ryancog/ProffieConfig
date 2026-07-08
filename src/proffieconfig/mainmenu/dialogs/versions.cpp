#include "versions.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/versions.cpp
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

#include <thread>

#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/settings.h>

#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "ui/build.hpp"
#include "ui/builders/selector.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/dialog.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/dialogs/progress.hpp"
#include "ui/helpers/busy.hpp"
#include "ui/helpers/foreach.hpp"
#include "ui/layout/notebook.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/values.hpp"
#include "utils/color.hpp"
#include "utils/paths.hpp"
#include "versions/os.hpp"
#include "versions/prop.hpp"
#include "versions/versions.hpp"

VersionsDlg::VersionsDlg(wxWindow *parent) :
    pcui::Dialog(
        parent,
        wxID_ANY,
        _("Versions Manager"),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ) {

    static const auto propInstalledTable{[] {
        data::base::Vector::RecvTable table;
        table.onInsert_ = data::map<&VersionsDlg::onPropInstalledChange>();
        table.onRemove_ = data::map<&VersionsDlg::onPropInstalledChange>();
        table.onSwap_ = data::map<&VersionsDlg::onPropInstalledChange>();
        return table;
    }()};
    observeWith(versions::props::list(), propInstalledTable);

    static const auto propAvailChoiceTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_ = data::map<&VersionsDlg::onPropAvailChoice>();
        return table;
    }()};
    observeWith(mAvailPropSel.choice(), propAvailChoiceTable);

    static const auto osInstalledTable{[] {
        data::base::Vector::RecvTable table;
        table.onInsert_ = data::map<&VersionsDlg::onOsInstalledChange>();
        table.onRemove_ = data::map<&VersionsDlg::onOsInstalledChange>();
        table.onSwap_ = data::map<&VersionsDlg::onOsInstalledChange>();
        return table;
    }()};
    observeWith(versions::os::list(), osInstalledTable);

    static const auto osAvailChoiceTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_ = data::map<&VersionsDlg::onOsAvailChoice>();
        return table;
    }()};
    observeWith(mAvailOsSel.choice(), osAvailChoiceTable);

    mPropSel.bind(&versions::props::list());
    mOsSel.bind(&versions::os::list());
    mAvailPropSel.bind(&versions::props::available());
    mAvailOsSel.bind(&versions::os::available());

    pcui::build(this, ui());

    activate();
}

VersionsDlg::~VersionsDlg() {
    pcui::cripple(this);
}

void VersionsDlg::onActivate() {
    updatePropInstall();
    updateOsInstall();
}

pcui::DescriptorPtr VersionsDlg::ui() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .children_={
        pcui::Notebook{
          .win_={
            .base_={
              .minSize_={-1, 300},
              .expand_=true,
              .proportion_=1,
            }
          },
          .pages_={
            pcui::Notebook::Page{
              .title_=_("Prop Files"),
              .content_=props(),
            },
            pcui::Notebook::Page{
              .title_=_("ProffieOS"),
              .content_=os(),
            }
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Button{
              .label_=_("Fetch Available"),
              .func_=[this] { onFetchButton(); },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .label_=_("Reset To Defaults"),
              .func_=[this] { onResetButton(); },
            }(),
            pcui::StretchSpacer{.size_=pcui::interGroupSpacing() * 3}(),
            pcui::Button{
              .label_=_("Show Local Folder"),
              .func_=[this] {
                  wxLaunchDefaultApplication(paths::versionDir().string());
              },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .label_=_("Reload Local"),
              .func_=[this] { versions::loadLocal(); },
            }(),
          }
        }(),
      },
    }();
}

pcui::DescriptorPtr VersionsDlg::props() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
      },
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=_("Available"),
            }(),
            pcui::Choice{
              .win_={
                .base_={
                  .expand_=true,
                  .proportion_=1,
                },
              },
              .data_=mAvailPropSel.choice(),
              .style_=pcui::Choice::List{},
              .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                  auto ctxt{data::context(mAvailPropSel)};
                  auto vecCtxt{data::context(*ctxt.bound())};

                  return dynamic_cast<versions::props::Available&>(
                      *vecCtxt.children()[idx]
                  ).name_;
              }
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={.base_={.expand_=true}},
              .label_=mPropInstall,
              .func_=[this] { onPropInstallButton(); },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Divider{}(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=_("Installed"),
            }(),
            pcui::Choice{
              .win_={
                .base_={
                  .expand_=true,
                  .proportion_=1,
                },
              },
              .data_=mPropSel.choice(),
              .style_=pcui::Choice::List{},
              .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                  auto ctxt{data::context(mPropSel)};
                  auto vecCtxt{data::context(*ctxt.bound())};
                  return dynamic_cast<versions::props::Versioned&>(
                      *vecCtxt.children()[idx]
                  ).name_;
              }
            }(),
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                /* TODO: For now someone can just drop the files in. A dialog
                 * to do that seems silly.
                pcui::Button{
                  .win_={
                    .base_={
                      .minSize_=pcui::iconButtonSize(),
                    },
                  },
                  .label_=pcui::syms::PLUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                }(),
                */
                pcui::Button{
                  .win_={
                    .base_={
                      .minSize_=pcui::iconButtonSize(),
                    },
                    .enable_=mPropSel.choice() | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::MINUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onPropRemoveButton(); },
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::builders::Selector{
          .data_=mPropSel,
          .builder_=propInfo
        }(),
      },
    }();
}

pcui::DescriptorPtr VersionsDlg::propInfo(data::base::Model *model) {
    const wxSize minSize(200, -1);

    if (model == nullptr) {
        return pcui::Stack{
          .base_={
            .expand_=true,
            .proportion_=1,
          },
          .children_={
            pcui::StretchSpacer{}(),
            pcui::Label{
              .win_={
                .base_={
                  .minSize_=minSize,
                  .proportion_=1,
                  .align_=wxALIGN_CENTER,
                },
              },
              .label_=_("No Prop Selected"),
              .font_=pcui::Font::Header,
              .color_=wxSYS_COLOUR_GRAYTEXT,
            }(),
            pcui::StretchSpacer{}(),
          }
        }();
    }

    auto& prop{dynamic_cast<versions::props::Versioned&>(*model)};
    const auto& data{prop.data_};

    return pcui::Stack{
      .base_={
        .minSize_=minSize,
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Label{
          .label_=prop.name_,
          .font_=pcui::Font::Header,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{}(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Label{
          .label_=wxString::Format(_("Display Name: %s"), data.name_),
        }(),
        pcui::Label{
          .label_=wxString::Format(_("Header File: %s"), data.filename_),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=wxString::Format(_("%zu Settings"), data.settings_.size()),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=[&data] {
              wxString numsString;

              for (const auto& [num, buttons] : data.buttons_) {
                  if (not numsString.empty())
                      numsString.append(", ");

                  numsString.append(std::to_string(num));
              }

              if (numsString.empty())
                  numsString = _("None");

              return wxString::Format(_("Supported Buttons: %s"), numsString);
          }(),
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=_("Supported Versions:"),
        }(),
        pcui::Scrolled{
          .win_={
            .base_={
              .minSize_={-1, 100},
              .expand_=true,
              .proportion_=1,
            },
          },
          .scrollRate_={.x_=-1, .y_=10},
          .child_=pcui::Stack{
            .children_={
              pcui::ForEach{
                .of_=prop.supportedVersions_,
                .do_=[](const utils::Version& ver) {
                    return pcui::Label{
                      .label_=ver.string()
                    }();
                }
              }(),
            },
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr VersionsDlg::os() {
    return pcui::Stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
      },
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=_("Available"),
            }(),
            pcui::Choice{
              .win_={
                .base_={
                  .expand_=true,
                  .proportion_=1,
                },
              },
              .data_=mAvailOsSel.choice(),
              .style_=pcui::Choice::List{},
              .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                  auto ctxt{data::context(mAvailOsSel)};
                  auto vecCtxt{data::context(*ctxt.bound())};

                  return dynamic_cast<versions::os::OS&>(
                      *vecCtxt.children()[idx]
                  ).version_.string();
              }
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={.base_={.expand_=true}},
              .label_=mOsInstall,
              .func_=[this] { onOsInstallButton(); },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Divider{}(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=_("Installed"),
            }(),
            pcui::Choice{
              .win_={
                .base_={
                  .expand_=true,
                  .proportion_=1,
                },
              },
              .data_=mOsSel.choice(),
              .style_=pcui::Choice::List{},
              .labeler_=[this](uint32 idx) -> pcui::Choice::Label {
                  auto ctxt{data::context(mOsSel)};
                  auto vecCtxt{data::context(*ctxt.bound())};
                  return dynamic_cast<versions::os::OS&>(
                      *vecCtxt.children()[idx]
                  ).version_.string();
              }
            }(),
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                /* TODO: For now someone can just drop the files in. A dialog
                 * to do that seems silly.
                pcui::Button{
                  .win_={
                    .base_={
                      .minSize_=pcui::iconButtonSize(),
                    },
                  },
                  .label_=pcui::syms::PLUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                }(),
                */
                pcui::Button{
                  .win_={
                    .base_={
                      .minSize_=pcui::iconButtonSize(),
                    },
                    .enable_=mOsSel.choice() | data::logic::HasSelection{},
                  },
                  .label_=pcui::syms::MINUS,
                  .style_=pcui::Button::Style::Companion,
                  .exactFit_=true,
                  .func_=[this] { onOsRemoveButton(); },
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::builders::Selector{
          .data_=mOsSel,
          .builder_=osInfo
        }(),
      },
    }();
}

pcui::DescriptorPtr VersionsDlg::osInfo(data::base::Model *model) {
    const wxSize minSize(200, -1);

    if (model == nullptr) {
        return pcui::Stack{
          .base_={
            .expand_=true,
            .proportion_=1,
          },
          .children_={
            pcui::StretchSpacer{}(),
            pcui::Label{
              .win_={
                .base_={
                  .minSize_=minSize,
                  .proportion_=1,
                  .align_=wxALIGN_CENTER,
                },
              },
              .label_=_("No OS Selected"),
              .font_=pcui::Font::Header,
              .color_=wxSYS_COLOUR_GRAYTEXT,
            }(),
            pcui::StretchSpacer{}(),
          }
        }();
    }

    auto& os{dynamic_cast<versions::os::OS&>(*model)};

    return pcui::Stack{
      .base_={
        .minSize_=minSize,
        .expand_=true,
        .proportion_=1,
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Label{
          .label_=wxString::Format(
              "ProffieOS %s",
              os.version_.string()
          ),
          .font_=pcui::Font::Header,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Divider{}(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Label{
          .label_=wxString::Format(
              _("Arduino Core: %s"),
              os.coreVersion_.string()
          ),
        }(),
        pcui::Label{
          .label_=os.coreUrl_,
          .font_=pcui::Font::Monospace,
          .color_=color::Special::Caption,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .label_=_("Supported Boards:"),
        }(),
        pcui::Spacer{.size_=4}(),
        pcui::Scrolled{
          .win_={
            .base_={
              .minSize_={-1, 100},
              .expand_=true,
              .proportion_=1,
            },
          },
          .scrollRate_={.x_=-1, .y_=10},
          .child_=pcui::Stack{
            .children_={
              pcui::ForEach{
                .of_=os.boards_,
                .do_=[](const auto& mapped) -> pcui::DynamicList {
                    const auto& [idx, board]{mapped};
                    return {
                        pcui::Label{.label_=board.name_}(),
                        pcui::Label{
                          .label_="    " + board.coreId_,
                          .font_=pcui::Font::Monospace,
                          .color_=color::Special::Caption,
                        }(),
                        pcui::Label{
                          .label_="    " + board.include_,
                          .font_=pcui::Font::Monospace,
                          .color_=color::Special::Caption,
                        }(),
                        pcui::Spacer{.size_=2}(),
                    };
                }
              }(),
            },
          }(),
        }(),
      }
    }();
}

void VersionsDlg::onFetchButton() {
    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Fetch Available"),
        true
    )};

    std::thread{[this, prog, busy] {
        auto err{versions::fetch(nullptr, prog)};
        if (err) {
            prog->finish(true, *err);
            return;
        }

        prog->finish(false);
    }}.detach();
}

void VersionsDlg::onResetButton() {
    constexpr auto FULL_RESET{wxYES};
    constexpr auto RESTORE_DEFAULTS{wxNO};
    auto res{pcui::showMessage(
        _(
            "These actions cannot be undone!\n"
            "\n"
            "\"Purge And Reset\" will remove any/all custom versions before restoring defaults.\n"
            "\n"
            "\"Restore Defaults\" will restore only default versions, preserving any custom versions."
        ),
        {
            .caption_=_("Restore Defaults"),
            .style_=wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT,
            .labels_={
                .yes_=_("Purge And Reset"),
                .no_=_("Restore Defaults")
            },
            .parent_=this
        }
    )};

    if (res == wxCANCEL)
        return;

    if (res == FULL_RESET) {
        auto res{pcui::showMessage(
            _("Are you sure?"),
            {
                .caption_=_("Purge And Reset"),
                .style_=wxYES_NO | wxNO_DEFAULT
            }
        )};

        if (res != wxYES)
            return;
    }

    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Restore Defaults"),
        true
    )};

    std::thread{[this, res, prog, busy] {
        prog->pulse(_("Restoring..."));

        auto err{versions::installDefault(res == FULL_RESET)};
        if (err) {
            prog->finish(true, *err);
            return;
        }

        prog->finish(false);
    }}.detach();
}

void VersionsDlg::onPropInstallButton() {
    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Install Prop")
    )};

    std::thread{[this, busy, prog] {
        auto ctxt{data::context(mAvailPropSel)};
        auto *avail{ctxt.selected<versions::props::Available>()};

        prog->pulse(_("Downloading files..."));

        auto err{versions::downloadProp(avail->name_)};
        if (err) {
            prog->finish(true, *err);
            return;
        }

        prog->finish(false);

        auto installedVec{data::context(versions::props::list())};

        size idx{0};
        auto children{installedVec.children()};
        for (; idx < children.size(); ++idx) {
            auto& installed{dynamic_cast<versions::props::Versioned&>(
                *children[idx]
            )};

            if (installed.name_ == avail->name_)
                break;
        }

        mPropSel.choice().choose(static_cast<int32>(idx));
    }}.detach();
}

void VersionsDlg::onPropRemoveButton() {
    pcui::BusyTracker busy(this);

    auto res{pcui::showMessage(
        _("This cannot be undone!"),
        {
            .caption_=_("Remove Prop"),
            .style_=wxYES_NO | wxNO_DEFAULT,
            .parent_=this
        }
    )};
    if (res != wxYES)
        return;

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Remove Prop")
    )};

    auto ctxt{data::context(mPropSel)};
    auto *versioned{ctxt.selected<versions::props::Versioned>()};

    prog->set(30, _("Purging files..."));

    std::error_code ec;
    fs::remove_all(paths::propDir() / versioned->name_, ec);
    if (ec) {
        prog->finish(true, ec.message());
        return;
    }

    prog->set(90, _("Reprocessing..."));

    versions::loadLocal();

    prog->finish(false);
}

void VersionsDlg::onPropInstalledChange(size) {
    updatePropInstall();
}

void VersionsDlg::onPropAvailChoice() {
    updatePropInstall();
}

void VersionsDlg::updatePropInstall() {
    auto sel{data::context(mAvailPropSel)};

    std::string label{_("Install").utf8_string()};
    bool enable{false};

    if (auto *avail{sel.selected<versions::props::Available>()}) {
        auto vec{data::context(versions::props::list())};

        bool found{false};
        for (const auto& model : vec.children()) {
            auto& installed{dynamic_cast<versions::props::Versioned&>(*model)};

            if (installed.name_ == avail->name_) {
                found = true;
                break;
            }
        }

        enable = not found;

        if (found)
            label = _("Installed").utf8_string();
    }

    mPropInstall.enable(enable);
    mPropInstall.change(std::move(label));
}

void VersionsDlg::onOsInstallButton() {
    pcui::BusyTracker busy(this);

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Install ProffieOS")
    )};

    std::thread{[this, busy, prog] {
        auto ctxt{data::context(mAvailOsSel)};
        auto *avail{ctxt.selected<versions::os::OS>()};

        prog->pulse(_("Downloading files..."));

        auto err{versions::downloadOS(avail->version_)};
        if (err) {
            prog->finish(true, *err);
            return;
        }

        prog->finish(false);

        auto installedVec{data::context(versions::os::list())};

        size idx{0};
        auto children{installedVec.children()};
        for (; idx < children.size(); ++idx) {
            auto& installed{dynamic_cast<versions::os::OS&>(*children[idx])};

            if (installed.version_.compare(avail->version_) == 0)
                break;
        }

        mOsSel.choice().choose(static_cast<int32>(idx));
    }}.detach();
}

void VersionsDlg::onOsRemoveButton() {
    pcui::BusyTracker busy(this);

    auto res{pcui::showMessage(
        _("This cannot be undone!"),
        {
            .caption_=_("Remove ProffieOS Version"),
            .style_=wxYES_NO | wxNO_DEFAULT,
            .parent_=this
        }
    )};
    if (res != wxYES)
        return;

    auto *prog{new pcui::ProgressDialog(
        this,
        _("Remove ProffieOS Version")
    )};

    auto ctxt{data::context(mOsSel)};
    auto *installed{ctxt.selected<versions::os::OS>()};

    prog->set(30, _("Purging files..."));

    std::error_code ec;
    fs::remove_all(paths::osDir() / installed->version_.string(), ec);
    if (ec) {
        prog->finish(true, ec.message());
        return;
    }

    prog->set(90, _("Reprocessing..."));

    versions::loadLocal();

    prog->finish(false);
}

void VersionsDlg::onOsInstalledChange(size) {
    updateOsInstall();
}

void VersionsDlg::onOsAvailChoice() {
    updateOsInstall();
}

void VersionsDlg::updateOsInstall() {
    auto sel{data::context(mAvailOsSel)};

    std::string label{_("Install").utf8_string()};
    bool enable{false};

    if (auto *avail{sel.selected<versions::os::OS>()}) {
        auto vec{data::context(versions::os::list())};

        bool found{false};
        for (const auto& model : vec.children()) {
            auto& installed{dynamic_cast<versions::os::OS&>(*model)};

            if (installed.version_.compare(avail->version_) == 0) {
                found = true;
                break;
            }
        }

        enable = not found;

        if (found)
            label = _("Installed").utf8_string();
    }

    mOsInstall.enable(enable);
    mOsInstall.change(std::move(label));
}

