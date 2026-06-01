#include "notebook.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/notebook.cpp
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

#include <wx/notebook.h>

#include "ui/build.hpp"
#include "ui/layout/detail/panel.hpp"
#include "ui/detail/window.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<wxNotebook> {
    Layout(const detail::Scaffold& scaffold, const Notebook& desc) {
        Create(scaffold.childParent_, desc.win_.id_);
        postCreation(scaffold, desc.win_);

        for (const auto& page : desc.pages_) {
            auto *panel{new detail::Panel(this)};

            build(panel, page.content_);

            AddPage(panel, page.title_);
        }

        activate();
    }
};

} // namespace

DescriptorPtr Notebook::operator()() {
    return std::make_unique<Notebook::Desc>(std::move(*this));
}

Notebook::Desc::Desc(Notebook&& data) :
    Notebook{std::move(data)} {}

wxSizerItem *Notebook::Desc::build(const detail::Scaffold& scaffold) const {
    auto *split{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(split)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Notebook::Desc::clone() const {
    return new Desc(*this);
}

