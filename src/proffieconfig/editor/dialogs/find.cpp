#include "find.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/find.cpp
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

#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/list.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/data_context.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/values.hpp"

#include "../editorwindow.hpp"

FindDlg::FindDlg(EditorWindow *parent) :
    pcui::Dialog(
        parent,
        wxID_ANY,
        _("Find"),
        wxFRAME_TOOL_WINDOW |
        wxFRAME_FLOAT_ON_PARENT |
        wxRESIZE_BORDER |
        wxCLOSE_BOX
    ) {

    pcui::build(this, ui());
    bind_events();
}

FindDlg::~FindDlg() {
    pcui::cripple(this);
}

pcui::DescriptorPtr FindDlg::ui() {
    return pcui::Stack{
      .base_{
        .minSize_={400, 300},
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Text{
              .win_={
                .base_={.proportion_=1},
                .id_=eID_Input,
                .focus_=true,
              },
              .data_=m_find_text,
              .font_=pcui::Font::Monospace,
              .style_=pcui::Text::SingleLine{
                .hint_=_("Search For..."),
                .onEnter_=[this] { on_find(); },
              },
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Button{
              .label_=_("Find"),
              .func_=[this] { on_find(); },
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::List{
          .win_={
            .base_={.expand_=true, .proportion_=1},
            .id_=eID_List,
          },
          .columns_=std::vector{
            _("Location"),
            _("Found Text"),
          },
          .rows_=m_num_found,
          .multi_selection_=false,
          .labeler_=[this](size row, size col) -> pcui::List::Label {
              const auto& found{*m_found[row]};

              if (col == 0) {
                  const auto& path{found.path()};
                  wxString ret;

                  for (auto iter{path.begin()}; iter != path.end(); ++iter) {
                      ret += *iter;

                      if (std::next(iter) != path.end())
                        ret += " > ";
                  }

                  return ret;
              }

              if (col == 1)
                  return found.snippet();

              return {};
          },
        }(),
      }
    }();
}

void FindDlg::bind_events() {
    auto *list{FindWindow(eID_List)};
    list->Bind(
        wxEVT_LIST_ITEM_ACTIVATED,
        &FindDlg::on_entry,
        this
    );
}

struct FindDlg::FoundStyle : FindDlg::Found {
    [[nodiscard]] const std::vector<std::string>& path() const final {
        return path_;
    }

    [[nodiscard]] const std::string& snippet() const final {
        return snippet_;
    }

    void reveal() const final {
        auto& page{editor_->mPresetsPage};
        auto array_sel{pcui::guiDataContext(page.mArraySel)};
        auto array_choice{pcui::guiDataContext(page.mArraySel.choice())};
        auto preset_sel{pcui::guiDataContext(page.mPresetSel)};
        auto preset_choice{pcui::guiDataContext(page.mPresetSel.choice())};
        auto style_sel{pcui::guiDataContext(page.mStyleSel)};
        auto style_choice{pcui::guiDataContext(page.mStyleSel.choice())};

        // Show presets page
        wxCommandEvent evt(wxEVT_MENU, EditorWindow::ePage_Presets);
        editor_->ProcessEvent(evt);

        { 
            size array_idx{0};
            auto arrays{pcui::guiDataContext(*array_sel.bound())};
            auto arrays_children{arrays.children()};
            for (; array_idx < arrays_children.size(); ++array_idx) {
                if (arrays_children[array_idx].get() == array_)
                    break;
            }

            if (array_idx == arrays_children.size())
                return;

            array_choice.choose(static_cast<int32>(array_idx));
        }

        {
            size preset_idx{0};
            auto presets{pcui::guiDataContext(*preset_sel.bound())};
            auto presets_children{presets.children()};
            for (; preset_idx < presets_children.size(); ++preset_idx) {
                if (presets_children[preset_idx].get() == preset_)
                    break;
            }

            if (preset_idx == presets_children.size())
                return;

            preset_choice.choose(static_cast<int32>(preset_idx));
        }

        {
            size style_idx{0};
            auto styles{pcui::guiDataContext(*style_sel.bound())};
            auto styles_children{styles.children()};
            for (; style_idx < styles_children.size(); ++style_idx) {
                if (styles_children[style_idx].get() == style_)
                    break;
            }

            if (style_idx == styles_children.size())
                return;

            style_choice.choose(static_cast<int32>(style_idx));
            auto& style{*style_sel.selected<config::presets::Style>()};
            auto content{pcui::guiDataContext(style.content_)};
            if (matched_.first < content.val().size())
                content.move(matched_.first);
        }
    }

    std::vector<std::string> path_;
    std::string snippet_;
    std::pair<size, size> matched_;

    EditorWindow *editor_;
    const config::presets::Array *array_;
    const config::presets::Preset *preset_;
    const config::presets::Style *style_;
};

void FindDlg::on_entry(wxListEvent& evt) {
    auto& found{*m_found[evt.GetIndex()]};
    found.reveal();
}

void FindDlg::on_find() {
    auto *editor{static_cast<EditorWindow *>(GetParent())};
    auto& config{*editor->mInfo.config()};

    // Clear before new search.
    auto num_found{pcui::guiDataContext(m_num_found)};
    num_found.set(0);
    m_found.clear();

    auto find_text{pcui::guiDataContext(m_find_text).val()};
    auto preset_arrays{pcui::guiDataContext(config.presetArrays_)};
    for (const auto& model : preset_arrays.children()) {
        const auto& array{dynamic_cast<config::presets::Array&>(*model)};
        auto presets{pcui::guiDataContext(array.presets_)};

        for (const auto& model : presets.children()) {
            auto& preset{dynamic_cast<config::presets::Preset&>(*model)};
            auto styles{pcui::guiDataContext(preset.styles_)};

            auto styles_children{styles.children()};
            for (size idx{0}; idx < styles_children.size(); ++idx) {
                const auto& model{styles_children[idx]};
                auto& style{dynamic_cast<config::presets::Style&>(*model)};
                auto content{pcui::guiDataContext(style.content_)};

                const auto& str{content.val()};
                auto find_pos{str.find(find_text)};
                if (find_pos == std::string::npos)
                    continue;

                // Blade strings might not exist depending on presets page
                // selections, so have a fallback.
                auto& blade_strings{editor->mPresetsPage.mBladeStrings};
                std::string blade_string;
                if (idx < blade_strings.size()) {
                    blade_string = pcui::guiDataContext(
                        *blade_strings[idx]
                    ).val();
                } else {
                    blade_string = wxString::Format(
                        _("Style %zu"),
                        idx
                    ).utf8_string();
                }

                auto found_style{std::make_unique<FindDlg::FoundStyle>()};
                found_style->editor_ = editor;
                found_style->array_ = &array;
                found_style->preset_ = &preset;
                found_style->style_ = &style;

                found_style->path_ = {
                    pcui::guiDataContext(array.name_).val(),
                    pcui::guiDataContext(preset.name_).val(),
                    blade_string
                };

                found_style->snippet_ = str.substr(
                    find_pos,
                    find_text.size() + 100
                );

                if (str.size() - find_pos > found_style->snippet_.length())
                    found_style->snippet_ += "...";

                if (find_pos != 0)
                    found_style->snippet_.insert(0, "...");

                found_style->matched_ = {find_pos, find_text.size()};

                m_found.push_back(std::move(found_style));
            }
        }
    }

    num_found.set(static_cast<int32>(m_found.size()));
}

FindDlg::Found::~Found() = default;

