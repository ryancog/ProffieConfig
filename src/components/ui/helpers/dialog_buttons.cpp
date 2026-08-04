#include "dialog_buttons.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/dialog_buttons.cpp
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

#include <wx/sizer.h>
#include <wx/statbox.h>

#include "ui/types.hpp"
#include "ui/values.hpp"

using namespace pcui;

DescriptorPtr DialogButtons::operator()() {
    // Always expand
    base_.expand_ = true;

    return std::make_unique<DialogButtons::Desc>(std::move(*this));
}

DialogButtons::Desc::Desc(DialogButtons&& data) :
    DialogButtons{std::move(data)} {}

wxSizerItem *DialogButtons::Desc::build(const detail::Scaffold& scaffold) const {
    detail::Scaffold childScaffold{scaffold};
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    // The fact that the spacers don't go away more smart is a little
    // unfortunate, but not a practical concern right now.

    const auto addSpacingOnLeft{[](wxSizerItem *item) {
        item->SetBorder(interGroupSpacing());
        const auto nonDirMask{item->GetFlag() & ~wxDIRECTION_MASK};
        item->SetFlag(nonDirMask | wxLEFT);
    }};

    const auto maybeSetId{[](wxWindow *win, wxWindowID id) {
        // Check if current ID is an auto ID, and change it if it is.
        auto cur{win->GetId()};
        if (wxID_AUTO_LOWEST <= cur and cur <= wxID_AUTO_HIGHEST)
            win->SetId(id);
    }};

    const auto setupOK{[&](wxSizerItem *item) {
        if (auto *win{item->GetWindow()})
            maybeSetId(win, wxID_OK);
    }};

    const auto setupCancel{[&](wxSizerItem *item) {
        if (auto *win{item->GetWindow()})
            maybeSetId(win, wxID_CANCEL);
    }};

    const auto setupApply{[&](wxSizerItem *item) {
        if (auto *win{item->GetWindow()})
            maybeSetId(win, wxID_APPLY);
    }};

#   ifdef _WIN32
    // Ok then Cancel then Apply on right
    sizer->AddStretchSpacer();

    if (ok_) {
        auto *item{ok_->build(childScaffold)};
        setupOK(item);
        sizer->Add(item);
    }

    if (cancel_) {
        auto *item{cancel_->build(childScaffold)};
        setupCancel(item);
        addSpacingOnLeft(item);
        sizer->Add(item);
    }

    if (apply_) {
        auto *item{apply_->build(childScaffold)};
        setupApply(item);
        addSpacingOnLeft(item);
        sizer->Add(item);
    }
#   else // macOS, GTK
    // Apply Far left, Cancel then Ok on right
    if (apply_) {
        auto *item{apply_->build(childScaffold)};
        setupApply(item);
        sizer->Add(item);
    }

    sizer->AddStretchSpacer();

    if (cancel_) {
        auto *item{cancel_->build(childScaffold)};
        setupCancel(item);
        addSpacingOnLeft(item);
        sizer->Add(item);
    }

    if (ok_) {
        auto *item{ok_->build(childScaffold)};
        setupOK(item);
        addSpacingOnLeft(item);
        sizer->Add(item);
    }
#   endif

    auto *item{new wxSizerItem(sizer)};
    detail::apply(base_, item);
    return item;
}

detail::Descriptor *DialogButtons::Desc::clone() const {
    return new Desc(*this);
}

