#include "injections.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/injections.cpp
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

#include <wx/event.h>
#include <wx/filedlg.h>
#include <wx/gdicmn.h>

#include "config/config.hpp"
#include "config/misc/injection.hpp"
#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "ui/build.hpp"
#include "ui/builders/vecstack.hpp"
#include "ui/controls/button.hpp"
#include "ui/dialog.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/label.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"

namespace {

data::logic::Element operator|(
    const config::Injection&, data::logic::CanMoveUp
);

data::logic::Element operator|(
    const config::Injection&, data::logic::CanMoveDown
);

} // namespace

InjectionsDlg::InjectionsDlg(wxWindow *parent, config::Config& config) :
    pcui::Dialog(
        parent,
        wxID_ANY,
        _("Injections"),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    mConfig{config} {
    pcui::build(this, ui());
}

pcui::DescriptorPtr InjectionsDlg::ui() {
    return pcui::Stack{
      .base_={
        .minSize_={400, 500},
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Group{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .padded_=false,
          .children_={
            pcui::Scrolled{
              .win_={.base_={.expand_=true, .proportion_=1}},
              .scrollRate_={.y_=4},
              .child_=pcui::builders::VecStack{
                .base_={
                  .expand_=true,
                  .proportion_=1,
                  .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
                },
                .orient_=wxVERTICAL,
                .data_=mConfig.injections_,
                .builder_=[this](data::base::Model& model) {
                    return injection(model);
                },
                .separator_=pcui::Stack{
                  .base_={.expand_=true},
                  .children_={
                    pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                    pcui::Divider{
                      .base_={.expand_=true},
                      .orient_=wxHORIZONTAL,
                    }(),
                    pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                  }
                }(),
                .empty_=pcui::Stack{
                  .base_={.expand_=true, .proportion_=1},
                  .children_={
                    pcui::StretchSpacer{}(),
                    pcui::Label{
                      .win_={
                        .base_={
                          .border_={
                            .size_=pcui::interGroupSpacing(),
                            .dirs_=wxALL
                          },
                          .align_=wxALIGN_CENTER,
                        },
                      },
                      .label_=_("No Injections"),
                      .color_=wxSYS_COLOUR_GRAYTEXT,
                    }(),
                    pcui::StretchSpacer{}(),
                  }
                }(),
              }(),
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={.base_={.align_=wxALIGN_RIGHT}},
          .label_=_("Add Injection"),
          .func_=[this] { onAddButton(); },
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Group{
          .win_={.base_={.expand_=true}},
          .children_=pcui::Label{
            .label_=_(
              "Injection files are an advanced and niche feature which allow you to add a\n"
              "C header file into the CONFIG_PRESETS section of the config file.\n"
              "\n"
              "If you don't know what this is or why you'd need it, you probably don't :)"
            )
          }(),
        }(),
      }
    }();
}

pcui::DescriptorPtr InjectionsDlg::injection(data::base::Model& model) {
    auto& injection{dynamic_cast<config::Injection&>(model)};

    return pcui::Stack{
      .base_={.expand_=true},
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .base_={.proportion_=1},
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=injection.filename_,
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Button{
                  .label_=_("Remove"),
                  .func_=[&injection](pcui::CallbackContext ctxt) {
                      ctxt.topLevel_->CallAfter([&injection] {
                          onRemoveButton(injection);
                      });
                  },
                }(),
                pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                pcui::Button{
                  .label_=_("Edit"),
                  .func_=[&injection] { onEditButton(injection); },
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::Stack{
          .orient_=wxVERTICAL,
          .children_={
            pcui::Button{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize()},
                .enable_=injection | data::logic::CanMoveUp{},
              },
              .label_=pcui::syms::UP_ARROW,
              .exactFit_=true,
              .func_=[&injection] { onUpButton(injection); },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize()},
                .enable_=injection | data::logic::CanMoveDown{},
              },
              .label_=pcui::syms::DOWN_ARROW,
              .exactFit_=true,
              .func_=[&injection] { onDownButton(injection); },
            }(),
          }
        }(),
      }
    }();
}

void InjectionsDlg::onAddButton() {
    wxFileDialog fileDialog{
        this,
        _("Select Injection File"),
        wxEmptyString,
        wxEmptyString,
        _("C Header") + " (*.h)|*.h",
        wxFD_FILE_MUST_EXIST | wxFD_OPEN
    };
    if (fileDialog.ShowModal() == wxID_CANCEL)
        return;

    const auto src{fileDialog.GetPath().utf8_string()};
    auto dst{paths::injectionDir() / fileDialog.GetFilename().utf8_string()};

    std::error_code ec;
    if (not files::copyOverwrite(src, dst, ec)) {
        pcui::showMessage(
            ec.message(),
            {.caption_=_("Injection file could not be added.")}
        );
        return;
    }

    mConfig.injections_.append(std::make_unique<config::Injection>(
        mConfig, fileDialog.GetFilename().utf8_string()
    ));
}

void InjectionsDlg::onUpButton(config::Injection& injection) {
    auto& config{injection.root<config::Config>()};
    auto ctxt{data::context(config.injections_)};
    ctxt.moveUp(*ctxt.find(injection));
}

void InjectionsDlg::onDownButton(config::Injection& injection) {
    auto& config{injection.root<config::Config>()};
    auto ctxt{data::context(config.injections_)};
    ctxt.moveDown(*ctxt.find(injection));
}

void InjectionsDlg::onEditButton(config::Injection& injection) {
    auto path{paths::injectionDir() / injection.filename_};
    wxLaunchDefaultApplication(path.native());
}

void InjectionsDlg::onRemoveButton(config::Injection& injection) {
    auto res{pcui::showMessage(
        _("This action cannot be undone!"),
        {
            .caption_=_("Delete Injection"),
            .style_=wxYES_NO | wxNO_DEFAULT
        }
    )};
    if (res != wxYES)
        return;

    auto& config{injection.root<config::Config>()};
    auto ctxt{data::context(config.injections_)};
    ctxt.remove(injection);
}

namespace {

data::logic::Element operator|(
    const config::Injection& injection, data::logic::CanMoveUp
) {
    struct Adapter : data::logic::detail::Base, data::Receiver {
        Adapter(const config::Injection& injection) :
            injection_{injection},
            config_{injection_.root<config::Config>()} {
            static const auto table{[] {
                data::base::Vector::RecvTable table;
                table.preRemove_ = data::map(&Adapter::onVecChange);
                table.onInsert_ = data::map(&Adapter::onVecChange);
                table.onSwap_ = data::map(&Adapter::onVecChange);
                return table;
            }()};
            observeWith(config_.injections_, table);
        }

        ~Adapter() override { deactivate(); }

        bool tryLock() override {
            return injection_.tryLock();
        }

        void unlock() override {
            injection_.unlock();
        }

        bool doActivate() override {
            auto ctxt{data::context(config_.injections_)};
            Receiver::activate();
            return ctxt.canMoveUp(*ctxt.find(injection_));
        }

        void onVecChange(size) {
            std::lock_guard scopeLock(*pLock);
            auto ctxt{data::context(config_.injections_)};
            onChange(ctxt.canMoveUp(*ctxt.find(injection_)));
        }

        const config::Injection& injection_;
        const config::Config& config_;
    };

    return std::make_unique<Adapter>(injection);
}

data::logic::Element operator|(
    const config::Injection& injection, data::logic::CanMoveDown
) {
    struct Adapter : data::logic::detail::Base, data::Receiver {
        Adapter(const config::Injection& injection) :
            injection_{injection},
            config_{injection_.root<config::Config>()} {
            static const auto table{[] {
                data::base::Vector::RecvTable table;
                table.preRemove_ = data::map(&Adapter::onVecChange);
                table.onInsert_ = data::map(&Adapter::onVecChange);
                table.onSwap_ = data::map(&Adapter::onVecChange);
                return table;
            }()};
            observeWith(config_.injections_, table);
        }

        ~Adapter() override { deactivate(); }

        bool tryLock() override {
            return injection_.tryLock();
        }

        void unlock() override {
            injection_.unlock();
        }

        bool doActivate() override {
            auto ctxt{data::context(config_.injections_)};
            Receiver::activate();
            return ctxt.canMoveDown(*ctxt.find(injection_));
        }

        void onVecChange(size) {
            std::lock_guard scopeLock(*pLock);
            auto ctxt{data::context(config_.injections_)};
            onChange(ctxt.canMoveDown(*ctxt.find(injection_)));
        }

        const config::Injection& injection_;
        const config::Config& config_;
    };

    return std::make_unique<Adapter>(injection);
}

} // namespace

