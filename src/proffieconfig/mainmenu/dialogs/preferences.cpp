#include "preferences.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/preferences.cpp
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

#include <utility>

#include <wx/preferences.h>
#include <wx/settings.h>

#include "data/receiver.hpp"
#include "data/primitive/models/choice.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/build.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/data_context.hpp"
#include "ui/layout/detail/panel.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

#include "../../core/state.hpp"
#include "utils/color.hpp"

using namespace state::prefs;

namespace {

struct GeneralPanel : pcui::detail::Panel
#   ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    , data::Receiver
#   endif
    {
    GeneralPanel(wxWindow *parent) : Panel(parent) {
        mPresetInsertion.update(static_cast<size>(
            enums::AddPresetInsertion::Max
        ));

#       ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
        static const auto insertionTable{[] {
            data::base::Choice::RecvTable table;
            table.onChoice_ = data::map<&GeneralPanel::onPresetInsertion>();
            return table;
        }()};
        observeWith(mPresetInsertion, insertionTable);

        static const auto styleTable{[] {
            data::base::String::RecvTable table;
            table.onChange_ = data::map<&GeneralPanel::onStyleEditor>();
            return table;
        }()};
        observeWith(mStyleEditor, styleTable);

        activate();
#       endif

        pcui::build(this, ui());
    }

    ~GeneralPanel() override {
        pcui::cripple(this);
    }

    bool TransferDataToWindow() override {
        mPresetInsertion.choose(static_cast<int32>(
            get<Enum::Add_Preset_Insertion>()
        ));
        mStyleEditor.change(get(Str::Style_Editor_Link));

        return true;
    }

#   ifndef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    bool TransferDataFromWindow() override {
        setPresetInsertion();
        setStyleEditor();
        state::saveState();

        return true;
    }
#   endif

private:
    void setPresetInsertion() {
        auto insertCtxt{pcui::guiDataContext(mPresetInsertion)};
        set<Enum::Add_Preset_Insertion>(
            static_cast<enums::AddPresetInsertion>(insertCtxt.idx())
        );
    }

    void setStyleEditor() {
        auto editorCtxt{pcui::guiDataContext(mStyleEditor)};
        set(Str::Style_Editor_Link, editorCtxt.val());
    }

#   ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    void onPresetInsertion() {
        setPresetInsertion();
        state::saveState();
    }

    void onStyleEditor() {
        setStyleEditor();
        state::saveState();
    }
#   endif

    pcui::DescriptorPtr ui() {
        return pcui::Stack{
          .base_={
            .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
          },
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=_("New presets are added:"),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Choice{
              .win_={
                .base_={
                  .border_={
                    .size_=pcui::interGroupSpacing() * 2,
                    .dirs_=wxLEFT
                  },
                },
              },
              .data_=mPresetInsertion,
              .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                  switch (static_cast<enums::AddPresetInsertion>(idx)) {
                      case enums::AddPresetInsertion::Begin:
                          return _("At Beginning");
                      case enums::AddPresetInsertion::End:
                          return _("At End");
                      case enums::AddPresetInsertion::Before_Selected:
                          return _("Before Selected (or Begin)");
                      case enums::AddPresetInsertion::After_Selected:
                          return _("After Selected (or End)");
                      case enums::AddPresetInsertion::Max:
                          break;
                  }

                  std::unreachable();
              },
            }(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
            pcui::Label{
              .label_=_("Use the style editor at this URL:"),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Text{
              .win_={
                .base_={
                  .minSize_={450, -1},
                  .expand_=true,
                  .border_={
                    .size_=pcui::interGroupSpacing() * 2,
                    .dirs_=wxLEFT
                  },
                },
              },
              .data_=mStyleEditor,
              .style_=pcui::Text::SingleLine{
                .hint_=_("Style Editor URL"),
              },
            }(),
            pcui::Spacer{.size_=2}(),
            pcui::Label{
              .win_={
                .base_={
                  .border_={
                    .size_=pcui::interGroupSpacing() * 2,
                    .dirs_=wxLEFT
                  },
                },
              },
              .label_=_("Use \"{}\" to represent where to place the style contents in the link."),
              .font_=pcui::Font::Caption,
              .color_=color::Special::Caption,
            }(),
          }
        }();
    }

    data::prim::Choice mPresetInsertion;
    data::prim::String mStyleEditor;
};

struct GeneralPage : wxStockPreferencesPage {
    GeneralPage() :
        wxStockPreferencesPage(wxStockPreferencesPage::Kind_General) {}

    wxWindow *CreateWindow(wxWindow *parent) override {
        return new GeneralPanel(parent);
    }
};

struct AdvancedPanel : pcui::detail::Panel
#   ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    , data::Receiver
#   endif
    {
    AdvancedPanel(wxWindow *parent) : Panel(parent) {
        mHandleKeepSavefiles.update(static_cast<size>(
            enums::HandleKeepSaveFiles::Max
        ));

#       ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
        static const auto savefilesTable{[] {
            data::base::Choice::RecvTable table;
            table.onChoice_ = data::map<&AdvancedPanel::onSavefiles>();
            return table;
        }()};
        observeWith(mHandleKeepSavefiles, savefilesTable);

        activate();
#       endif

        pcui::build(this, ui());
    }

    ~AdvancedPanel() override {
        pcui::cripple(this);
    }

    bool TransferDataToWindow() override {
        mHandleKeepSavefiles.choose(static_cast<int32>(
            get<Enum::Handle_Keep_Save_Files>()
        ));
        return true;
    }

#   ifndef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    bool TransferDataFromWindow() override {
        setHandleKeepSavefiles();
        state::saveState();

        return true;
    }
#endif

private:
    void setHandleKeepSavefiles() {
        auto handleKeepSavefiles{pcui::guiDataContext(mHandleKeepSavefiles)};
        set<Enum::Handle_Keep_Save_Files>(
            static_cast<enums::HandleKeepSaveFiles>(handleKeepSavefiles.idx())
        );
    }

#   ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    void onSavefiles() {
        setHandleKeepSavefiles();
        state::saveState();
    }
#   endif

    pcui::DescriptorPtr ui() {
        return pcui::Stack{
          .base_={
            .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
          },
          .orient_=wxVERTICAL,
          .children_={
            pcui::Label{
              .label_=_("Keep Savefiles When Programming is:"),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Choice{
              .win_={
                .base_={
                  .border_={
                    .size_=pcui::interGroupSpacing() * 2,
                    .dirs_=wxLEFT
                  },
                },
              },
              .data_=mHandleKeepSavefiles,
              .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                  switch (static_cast<enums::HandleKeepSaveFiles>(idx)) {
                      using enum enums::HandleKeepSaveFiles;
                      case Ignore_Alert:
                        return _("Ignored, and an alert is shown if used");
                      case Ignore:
                        return _("Ignored");
                      case Allow_Hide_Unused:
                        return _("Allowed, but normally hidden");
                      case Allow:
                        return _("Allowed");
                      case Max:
                        break;
                  }

                  std::unreachable();
              },
            }(),
          }
        }();
    }

    data::prim::Choice mHandleKeepSavefiles;
};

struct AdvancedPage : wxStockPreferencesPage {
    AdvancedPage() :
        wxStockPreferencesPage(wxStockPreferencesPage::Kind_Advanced) {}

    wxWindow *CreateWindow(wxWindow *parent) override {
        return new AdvancedPanel(parent);
    }
};

} // namespace

PreferencesDlg::PreferencesDlg() {
    mEditor.AddPage(new GeneralPage);
    mEditor.AddPage(new AdvancedPage);
}

void PreferencesDlg::show(wxWindow *parent) {
    mEditor.Show(parent);
}

