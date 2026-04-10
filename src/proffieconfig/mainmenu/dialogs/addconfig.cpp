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
#include "data/helpers/exclusive.hpp"
#include "data/logic/adapter.hpp"
#include "data/logic/operators.hpp"
#include "data/string.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/filepicker.hpp"
#include "ui/controls/segmented.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/static/image.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/parent.hpp"

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
}

AddConfigDialog::~AddConfigDialog() {
    pcui::teardown(this);
}

AddConfigDialog::Result AddConfigDialog::getResult() {
    data::String::ROContext name{mConfigName};
    data::String::ROContext path{mImportPath};

    return {
        .mode_=static_cast<Result::Mode>(mMode.selected()),
        .path_=path.val(),
        .name_=name.val(),
    };
}

void AddConfigDialog::bindEvents() {
    mConfigName.responder().onChange_ = [](
        const data::String::ROContext& ctxt
    ) {
        auto& self{utils::parent<&AddConfigDialog::mConfigName>(
            const_cast<data::String&>(ctxt.model<data::String>())
        )};

        bool dupName{false};
        data::Vector::ROContext list{config::list()};
        for (const auto& model : list.children()) {
            auto& exist{static_cast<config::Info&>(*model)};
            data::String::ROContext existName{exist.name()};
            if (existName.val() != ctxt.val()) continue;

            dupName = true;
            break;
        }
        data::Bool::Context{self.mDupName}.set(dupName);

        bool nameEmpty{ctxt.val().empty()};
        bool nameBadChars{
            ctxt.val().find_first_of(".\\,/!#$%^&*|?<>\"'") != std::string::npos
        };

        data::Bool::Context{self.mNameValid}.set(
            not nameEmpty and
            not dupName and
            not nameBadChars
        );
    };
    mConfigName.responder().onChange_(mConfigName);

    mImportPath.responder().onChange_ = [](
        const data::String::ROContext& ctxt
    ) {
        auto& self{utils::parent<&AddConfigDialog::mImportPath>(
            const_cast<data::String&>(ctxt.model<data::String>())
        )};

        data::Bool::Context{self.mNeedImportPath}.set(ctxt.val().empty());

        fs::path path{ctxt.val()};
        if (path.has_stem()) {
            data::String::Context configName{self.mConfigName};
            configName.change(path.stem().string());
        }
    };
    mImportPath.responder().onChange_(mImportPath);
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
            .show_=mMode[eMode_Import] | data::logic::IsSet{}
          },
          .label_=_("Configuration to Import"),
        }(),
        pcui::FilePicker{
          .win_={
            .base_{.expand_=true},
            .show_=mMode[eMode_Import] | data::logic::IsSet{}
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
              (mMode[eMode_Import] | data::logic::IsSet{})
            }
          },
          .label_=_("Please choose a configuration file to import"),
        }(),
        pcui::StretchSpacer{}(),
        pcui::DialogButtons{
          .base_={.expand_=true},
          .ok_=pcui::Button{
            .win_{
              .enable_={
                mNameValid | data::logic::IsSet{} and
                (not (mMode[eMode_Import] | data::logic::IsSet{}) or
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

