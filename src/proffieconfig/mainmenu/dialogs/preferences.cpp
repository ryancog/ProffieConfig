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

#include <wx/preferences.h>
#include <wx/settings.h>

#include "data/context.hpp"
#include "data/receiver.hpp"
#include "data/primitive/models/choice.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/build.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/text.hpp"
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

        activate();
#       endif

        pcui::build(this, ui());
    }

    ~GeneralPanel() override {
        pcui::cripple(this);
    }

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

                  assert(0);
                  __builtin_unreachable();
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

    bool TransferDataToWindow() override {
        mPresetInsertion.choose(static_cast<int32>(
            get<Enum::Add_Preset_Insertion>()
        ));
        mStyleEditor.change(get(Str::Style_Editor_Link));

        return true;
    }

#   ifndef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    bool TransferDataFromWindow() override {
        auto insertCtxt{data::context(mPresetInsertion)};
        set<Enum::Add_Preset_Insertion>(
            static_cast<enums::AddPresetInsertion>(insertCtxt.idx())
        );

        auto editorCtxt{data::context(mStyleEditor)};
        set(Str::Style_Editor_Link, editorCtxt.val());

        state::saveState();

        return true;
    }
#   endif

private:
#   ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    void onPresetInsertion() {
        auto insertCtxt{data::context(mPresetInsertion)};
        set<Enum::Add_Preset_Insertion>(
            static_cast<enums::AddPresetInsertion>(insertCtxt.idx())
        );

        state::saveState();
    }

    void onStyleEditor() {
        auto editorCtxt{data::context(mStyleEditor)};
        set(Str::Style_Editor_Link, editorCtxt.val());

        state::saveState();
    }
#   endif

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

/*
struct AdvancedPanel : pcui::detail::Panel
#   ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    , data::Receiver
#   endif
    {
    AdvancedPanel(wxWindow *parent) : Panel(parent) {
#       ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
        activate();
#       endif

        pcui::build(this, ui());
    }

    ~AdvancedPanel() override {
        pcui::cripple(this);
    }

    pcui::DescriptorPtr ui() {
        return pcui::Stack{
          .base_={
            .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
          },
          .orient_=wxVERTICAL,
          .children_={
          }
        }();
    }

    bool TransferDataToWindow() override {
        return true;
    }

    bool TransferDataFromWindow() override {
        return true; }

private:
};

struct AdvancedPage : wxStockPreferencesPage {
    AdvancedPage() :
        wxStockPreferencesPage(wxStockPreferencesPage::Kind_Advanced) {}

    wxWindow *CreateWindow(wxWindow *parent) override {
        return new AdvancedPanel(parent);
    }
};
*/

} // namespace

PreferencesDlg::PreferencesDlg() {
    mEditor.AddPage(new GeneralPage);
    // mEditor.AddPage(new AdvancedPage);
}

void PreferencesDlg::show(wxWindow *parent) {
    mEditor.Show(parent);
}

