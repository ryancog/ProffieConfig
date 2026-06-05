#include "addconfig.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/addconfig.cpp
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

#include <wx/dialog.h>
#include <wx/sysopt.h>
#include <wx/filedlg.h>

#include "config/config.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/filepicker.hpp"
#include "ui/controls/segmented.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

#include "../mainmenu.hpp"

AddConfigDialog::AddConfigDialog(MainMenu *parent) :
    Dialog(
        parent,
        wxID_ANY,
        _("Add New Config")
    ) {
#   ifdef __WXOSX__
    // TODO: This should really go elsewhere... but where?
    wxSystemOptions::SetOption(wxOSX_FILEDIALOG_ALWAYS_SHOW_TYPES, true);
#   endif

    pcui::build(this, ui());
    bindEvents();

    activate();
}

AddConfigDialog::~AddConfigDialog() {
    pcui::cripple(this);
}

void AddConfigDialog::onActivate() {
    onName();
    onPath();
}

AddConfigDialog::Result AddConfigDialog::getResult() {
    auto name{data::context(mConfigName)};
    auto path{data::context(mImportPath)};
    auto mode{data::context(mMode)};

    return {
        .mode_=static_cast<Result::Mode>(mode.selected()),
        .path_=path.val(),
        .name_=name.val(),
    };
}

void AddConfigDialog::bindEvents() {
    static const auto nameTable{[] {
        data::prim::String::RecvTable table;
        table.onChange_ = data::map<&AddConfigDialog::onName>();
        return table;
    }()};
    observeWith(mConfigName, nameTable);
    
    static const auto pathTable{[] {
        data::prim::String::RecvTable table;
        table.onChange_ = data::map<&AddConfigDialog::onPath>();
        return table;
    }()};
    observeWith(mImportPath, pathTable);
}

pcui::DescriptorPtr AddConfigDialog::ui() {
    return pcui::Stack{
      .base_={
        .minSize_={400, -1},
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .children_={
        pcui::Segmented{
          .win_={
            .base_={.align_=wxALIGN_CENTER},
          },
          .data_=mMode,
          .labels_={
            pcui::Segmented::Label{
              .text_=_("Create New Config"),
            },
            pcui::Segmented::Label{
              .text_=_("Import Existing Config"),
            },
          },
        }(),
        pcui::Label{
          .win_={
            .base_={.border_={.dirs_=wxTOP}},
            .show_=mMode | data::logic::HasSelection{{eMode_Import}},
          },
          .label_=_("Configuration to Import"),
        }(),
        pcui::FilePicker{
          .win_={
            .base_{.expand_=true},
            .show_=mMode | data::logic::HasSelection{{eMode_Import}},
          },
          .data_=mImportPath,
          .message_=_("Choose Configuration File to Import"),
          .wildcard_=_("ProffieOS Configuration") + " (*.h)|*.h",
          .mode_=pcui::FilePicker::Open{},
        }(),
        pcui::Label{
          .win_={.base_={.border_={.dirs_=wxTOP}}},
          .label_=_("Configuration Name"),
        }(),
        pcui::Text{
          .win_={.base_{.expand_=true}},
          .data_=mConfigName,
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Label{
          .win_={
            .base_={.align_=wxALIGN_RIGHT},
            .show_=not (mNameValid | data::logic::IsSet{})
          },
          .label_=_("Please enter a valid name"),
        }(),
        pcui::Label{
          .win_={
            .base_={.align_=wxALIGN_RIGHT},
            .show_=mDupName | data::logic::IsSet{}
          },
          .label_=_("Configuration with same name already exists"),
        }(),
        pcui::Label{
          .win_={
            .base_={.align_=wxALIGN_RIGHT},
            .show_={
              (mNeedImportPath | data::logic::IsSet{}) and
              mMode | data::logic::HasSelection{{eMode_Import}},
            }
          },
          .label_=_("Please choose a configuration file to import"),
        }(),
        pcui::StretchSpacer{}(),
        pcui::DialogButtons{
          .ok_=pcui::Button{
            .win_{
              .enable_={
                mNameValid | data::logic::IsSet{} and
                (not (mMode | data::logic::HasSelection{{eMode_Import}}) or
                 not (mNeedImportPath | data::logic::IsSet{}))
              },
            },
            .label_=_("OK"),
            .default_=true,
            .func_=[this] { EndModal(wxID_OK); }
          }(),
          .cancel_=pcui::Button{
            .label_=_("Cancel"),
            .func_=[this] { EndModal(wxID_CANCEL); }
          }(),
        }(),
      },
    }();
}

void AddConfigDialog::onName() {
    auto name{data::context(mConfigName)};

    bool dupName{false};
    auto list{data::context(config::list())};
    for (const auto& model : list.children()) {
        auto& exist{dynamic_cast<config::Info&>(*model)};
        auto existName{data::context(exist.name())};
        if (existName.val() != name.val()) continue;

        dupName = true;
        break;
    }
    mDupName.set(dupName);

    bool nameEmpty{name.val().empty()};
    bool nameBadChars{
        name.val().find_first_of(".\\,/!#$%^&*|?<>\"'") != std::string::npos
    };

    mNameValid.set(
        not nameEmpty and
        not dupName and
        not nameBadChars
    );
}

void AddConfigDialog::onPath() {
    auto path{data::context(mImportPath)};

    mNeedImportPath.set(path.val().empty());

    fs::path fsPath{path.val()};
    if (fsPath.has_stem()) {
        auto configName{data::context(mConfigName)};
        configName.change(fsPath.stem().string());
    }
}

