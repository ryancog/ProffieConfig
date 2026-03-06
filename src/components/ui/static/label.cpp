#include "label.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/static/label.cpp
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

#include <wx/font.h>
#include <wx/stattext.h>

#include "ui/priv/helpers.hpp"

using namespace pcui;

std::unique_ptr<detail::Descriptor> Label::operator()() {
    return std::make_unique<Label::Desc>(std::move(*this));
}

Label::Desc::Desc(Label&& label) :
    Label(std::move(label)) {}

wxSizerItem *Label::Desc::build(const detail::Scaffold& scaffold) const {
    auto *text{new wxStaticText(scaffold.childParent_, wxID_ANY, label_)};

    text->SetFont(style_.makeFont());

    auto *item{new wxSizerItem(text)};
    priv::apply(base_, item);
    priv::apply(win_, item);

    return item;
}

