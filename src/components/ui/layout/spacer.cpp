#include "spacer.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/spacer.cpp
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

using namespace pcui;

std::unique_ptr<detail::Descriptor> Spacer::operator()() {
    // NOLINTNEXTLINE(performance-move-const-arg)
    return std::make_unique<Spacer::Desc>(std::move(*this));
}

Spacer::Desc::Desc(Spacer&& data) :
    // NOLINTNEXTLINE(performance-move-const-arg)
    Spacer{std::move(data)} {}

wxSizerItem *Spacer::Desc::build(const detail::Scaffold&) const {
    return new wxSizerItem(size_, size_);
}

std::unique_ptr<detail::Descriptor> StretchSpacer::operator()() {
    // NOLINTNEXTLINE(performance-move-const-arg)
    return std::make_unique<StretchSpacer::Desc>(std::move(*this));
}

StretchSpacer::Desc::Desc(StretchSpacer&& data) :
    // NOLINTNEXTLINE(performance-move-const-arg)
    StretchSpacer{std::move(data)} {}

wxSizerItem *StretchSpacer::Desc::build(const detail::Scaffold&) const {
    return new wxSizerItem(size_, size_, proportion_);
}

