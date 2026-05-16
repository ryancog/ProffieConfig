#include "list.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/list.cpp
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

#include <variant>

#include <wx/listctrl.h>
#include <wx/settings.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/window.hpp"
#include "ui/font.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Control : detail::Window<wxListCtrl> {
    Control(const detail::Scaffold& scaffold, const List& desc) :
        labeler_{desc.labeler_},
        rows_{desc.rows_} {
        itemAttr_.SetFont(- pcui::Font::Monospace);

        auto style{wxLC_REPORT | wxLC_VIRTUAL};

        if (std::holds_alternative<size>(desc.columns_))
            style |= wxLC_NO_HEADER;

        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );

        postCreation(scaffold, desc.win_);

        if (auto *ptr{std::get_if<1>(&rows_)}) {
            static const auto rowTable{[] {
                data::base::Integer::RecvTable table;
                table.onSet_ = data::map(&Control::onRows);
                return table;
            }()};
            amend(ptr->get(), rowTable);
        }

        // Only handle one for now.
        if (auto *ptr{std::get_if<1>(&desc.columns_)}) {
            for (auto& col : *ptr)
                AppendColumn(col);
        } else {
            auto num{std::get<0>(desc.columns_)};
            for (size idx{0}; idx < num; ++idx)
                AppendColumn({});
        }

        Bind(wxEVT_SIZE, &Control::onSizeEvent, this);

        activate();
    }

    void onActivate() override {
        Window::onActivate();

        Bind(wxEVT_LIST_CACHE_HINT, &Control::onCacheEvent, this);

        if (auto *ptr{std::get_if<1>(&rows_)}) {
            SetItemCount(data::context(ptr->get()).val());
        } else {
            SetItemCount(static_cast<long>(std::get<size>(rows_)));
        }
    }

    wxString OnGetItemText(long row, long col) const override {
        std::lock_guard scopeLock(pMutex);

        const auto rowIdx{row - labelStart_};

        // If there's momentary desync, make sure things don't go oob.
        if (rowIdx < 0 or rowIdx >= labels_.size())
            return {};

        auto& label{labels_[rowIdx][col]};

        if (auto *ptr{std::get_if<1>(&label)}) {
            return data::context(ptr->get()).val();
        }

        return std::get<0>(label);
    }

    wxItemAttr *OnGetItemAttr(long) const override {
        return &itemAttr_;
    }

    void onCacheEvent(wxListEvent& evt) {
        std::lock_guard scopeLock(pMutex);

        auto start{evt.GetCacheFrom()};
        auto num{evt.GetCacheTo() - start + 1};

        if (start == labelStart_ and num == labels_.size())
            return;

        std::vector<std::vector<List::Label>> newLabels;
        const auto oldLabelEnd{labelStart_ + labels_.size()};

        for (size idx{0}; idx < num; ++idx) {
            auto ctrlIdx{start + idx};

            if (ctrlIdx >= labelStart_ and ctrlIdx < oldLabelEnd) {
                const auto oldIdx{ctrlIdx - labelStart_};
                newLabels.push_back(std::move(labels_[oldIdx]));
            } else {
                std::vector<List::Label> colLabels;
                
                for (size colIdx{0}; colIdx < GetColumnCount(); ++colIdx) {
                    auto label{labeler_(ctrlIdx, colIdx)};

                    if (auto *ptr{std::get_if<1>(&label)}) {
                        static const auto labelTable{[] {
                            data::base::String::RecvTable table;
                            table.onChange_ =
                                data::map(&Control::onLabelChange);
                            return table;
                        }()};
                        amend(*ptr, labelTable);
                    }

                    colLabels.push_back(std::move(label));
                }

                newLabels.push_back(std::move(colLabels));
            }
        }

        for (auto& oldColLabels : labels_) {
            for (auto& oldColLabel : oldColLabels) {
                if (auto *ptr{std::get_if<1>(&oldColLabel)})
                    repeal(*ptr);
            }
        }

        labels_ = std::move(newLabels);
        labelStart_ = start;

        recomputeColumns();
    }

    void onSizeEvent(wxSizeEvent& evt) {
        recomputeColumns();
        evt.Skip();
    }

    void onRows() {
        auto ctxt{data::context(std::get<1>(rows_).get())};

        // Row backing data might be about to be destroyed.
        // Invalidate everything.
        labelStart_ = 0;
        for (auto& oldColLabels : labels_) {
            for (auto& oldColLabel : oldColLabels) {
                if (auto *ptr{std::get_if<1>(&oldColLabel)})
                    repeal(*ptr);
            }
        }
        labels_.clear();

        safeCall([this, num=ctxt.val()] {
            SetItemCount(num);
        });
    }

    void onLabelChange(const data::base::Model& model) {
        safeCall([this, &model] {
            std::lock_guard scopeLock(pMutex);

            for (size idx{0}; idx < labels_.size(); ++idx) {
                auto& rowLabels{labels_[idx]};
                for (size col{0}; col < rowLabels.size(); ++col) {
                    auto& label{rowLabels[col]};
                    if (auto *ptr{std::get_if<1>(&label)}) {
                        RefreshItem(static_cast<long>(labelStart_ + idx));
                        return;
                    }
                }
            }
        });
    }
    
    void recomputeColumns() {
        auto numRows{labels_.size()};
        auto numColumns{GetColumnCount()};

        auto widthLeft{GetClientSize().GetWidth()};
        for (int col{0}; col < numColumns; ++col) {
            int bestWidth{0};
            for (size row{0}; row < numRows; ++row) {
                const auto text{OnGetItemText(
                    static_cast<long>(labelStart_ + row),
                    col
                )};

                int width{};
                GetTextExtent(
                    text,
                    &width,
                    nullptr,
                    nullptr,
                    nullptr,
                    &itemAttr_.GetFont()
                );

                bestWidth = std::max(bestWidth, width);
            }

            if (col == numColumns - 1)
                bestWidth = std::max(bestWidth, widthLeft);
            else
                // wxWidgets uses AUTOSIZE_COL_MARGIN = 10
                bestWidth += 10;
            
            if (bestWidth != GetColumnWidth(col))
                SetColumnWidth(col, bestWidth);

            widthLeft -= bestWidth;
        }
    }

    ssize labelStart_{0};
    std::vector<std::vector<List::Label>> labels_;

    List::Labeler labeler_;

    List::Rows rows_;

    mutable wxItemAttr itemAttr_;
};

} // namespace

DescriptorPtr List::operator()() {
    return std::make_unique<List::Desc>(std::move(*this));
}

List::Desc::Desc(List&& data) :
    List{std::move(data)} {}

wxSizerItem *List::Desc::build(const detail::Scaffold& scaffold) const {
    auto *ctrl{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(ctrl)};

    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *List::Desc::clone() const {
    return new Desc(*this);
}

