#include "panel.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/panel.cpp
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

#if __WXOSX__
#import <AppKit/AppKit.h>
#endif

#include <wx/panel.h>
#include <wx/settings.h>

#include "ui/detail/window.hpp"
#include "ui/layout/detail/panel.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Layout : detail::Window<detail::Panel> {
    Layout(const detail::Scaffold& scaffold, const pcui::Panel& desc) {
        create(scaffold.childParent_, desc.win_.id_);
        postCreation(scaffold, desc.win_);

        if (desc.child_) {
            auto *sizer{new wxBoxSizer(wxVERTICAL)};

            auto child_scaffold{scaffold};
            child_scaffold.childParent_ = this;
            child_scaffold.sizer_ = sizer;

            sizer->Add(desc.child_->build(child_scaffold));
            SetSizer(sizer);
        }

        if (desc.tint_ == pcui::Panel::Tint::Content) {
#           if __WXOSX__
            auto view{GetHandle()};
            auto effect_view{[NSVisualEffectView new]};
            effect_view.material = NSVisualEffectMaterialContentBackground;

            [view addSubview:effect_view positioned:NSWindowBelow relativeTo:nil];
            [effect_view release];

            effect_view.translatesAutoresizingMaskIntoConstraints = false;
            [NSLayoutConstraint activateConstraints:@[
                [view.topAnchor constraintEqualToAnchor:effect_view.topAnchor],
                [view.bottomAnchor constraintEqualToAnchor:effect_view.bottomAnchor],
                [view.leadingAnchor constraintEqualToAnchor:effect_view.leadingAnchor],
                [view.trailingAnchor constraintEqualToAnchor:effect_view.trailingAnchor],
            ]];
#           else
            Bind(wxEVT_SYS_COLOUR_CHANGED, on_sys_color_changed);
            SetOwnBackgroundColour(get_background_color());
#           endif
        }

        activate();
    }

#   if not __WXOSX__
    static wxColour get_background_color() {
        auto dark{wxSystemSettings::GetAppearance().AreAppsDark()};
#       if __WXMSW__
        // Probably some weird historical thing: while the panel is colored as
        // wxSYS_COLOUR_WINDOW in dark mode, in light mode, it's
        // wxSYS_COLOUR_BTNFACE/wxSYS_COLOUR_FRAMEBK, and buttons are actually
        // colored a bit lighter.
        if (not dark) {
            auto color{wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)};
            return color.ChangeLightness(140);
        }

        auto color{wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)};
        return color.ChangeLightness(70);
#       else
        auto color{wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)};
        return color.ChangeLightness(dark ? 70 : 140);
#       endif
    }
    
    static void on_sys_color_changed(wxSysColourChangedEvent& evt) {
        evt.Skip();
        auto *panel{static_cast<Layout *>(evt.GetEventObject())};
        panel->SetOwnBackgroundColour(get_background_color());
    }
#   endif
};

} // namespace

DescriptorPtr Panel::operator()() {
    return std::make_unique<Panel::Desc>(std::move(*this));
}

Panel::Desc::Desc(Panel&& data) :
    Panel{std::move(data)} {}

wxSizerItem *Panel::Desc::build(const detail::Scaffold& scaffold) const {
    auto *panel{new Layout(scaffold, *this)};

    auto *item{new wxSizerItem(panel)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Panel::Desc::clone() const {
    return new Desc(*this);
}

