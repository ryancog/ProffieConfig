#include "divider.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/static/divider.cpp
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

#include <wx/statline.h>

#include "ui/priv/helpers.hpp"

using namespace pcui;

std::unique_ptr<detail::Descriptor> Divider::operator()() {
    return std::make_unique<Divider::Desc>(std::move(*this));
}

Divider::Desc::Desc(Divider&& div) :
    Divider(std::move(div)) {}

wxSizerItem *Divider::Desc::build(const detail::Scaffold& scaffold) const {
    auto *text{new wxStaticLine(scaffold.childParent_, wxID_ANY)};
    auto *item{new wxSizerItem(text)};
    priv::apply(base_, item);
    return item;
}

