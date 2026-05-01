#include "window.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/window.cpp
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

#include "ui/detail/helpers.hpp"

using namespace pcui::detail;

void WindowImpl::onDeactivate() {
    mShowReceiver.reset();
    mShow.reset();
    mEnableReceiver.reset();
    mEnable.reset();
}

bool WindowImpl::freezeGetRealEnable() {
    if (not mEnable) return true;

    mEnable->lock();
    return mEnable->val();
}

void WindowImpl::thawRealEnable() {
    if (mEnable) mEnable->unlock();
}

void WindowImpl::updateVisualEnable() {
    safeCall([this]() {
        // Don't worry about locking here. If this happens to see something
        // a bit early that's fine.
        const auto winEn{not mEnable or mEnable->val()};
        const auto en{winEn and visualEnableOverride()};
        dynamic_cast<wxWindow *>(this)->Enable(en);
    });
}

void WindowImpl::postCreation(
    const Scaffold& scaffold, const ChildWindowBase& desc
) {
    auto *win{dynamic_cast<wxWindow *>(this)};

    mMinSize = desc.base_.minSize_;
    mMaxSize = desc.maxSize_;

    mShow = desc.show_;
    if (mShow)
        mShowReceiver = std::make_unique<ShowReceiver>(this);

    mEnable = desc.enable_;
    if (mEnable)
        mEnableReceiver = std::make_unique<EnableReceiver>(this);

    if (desc.tooltip_.empty() and win->GetParent()) {
        // GetToolTip returns a ptr, and these tool tips cannot be shared
        // across windows, so get the text and SetToolTip will create a new
        // tip from it.
        win->SetToolTip(win->GetParent()->GetToolTipText());
    } else {
        win->SetToolTip(desc.tooltip_);
    }

    if (scaffold.scrolled_) {
        const auto onWheel{[scrolled=scaffold.scrolled_](
            wxMouseEvent& evt
        ) {
            scrolled->HandleOnMouseWheel(evt);
        }};
        win->Bind(wxEVT_MOUSEWHEEL, onWheel);
    }

    updateSizes();

    if (desc.focus_)
        win->SetFocus();
}

void WindowImpl::safeCall(const std::function<void()>& func) {
    if (wxIsMainThread())
        func();
    else
        dynamic_cast<wxWindow *>(this)->CallAfter(func);
}

void WindowImpl::updateSizes() {
    auto *win{dynamic_cast<wxWindow *>(this)};

    win->SetMinSize({-1, -1});

    auto minSize{mMinSize};
    minSize.IncTo(win->GetBestSize());

    if (mMaxSize.IsFullySpecified()) {
        minSize.DecTo(mMaxSize);
        win->SetMaxSize(mMaxSize);
    }

    win->SetMinSize(minSize);
}

WindowImpl::ShowReceiver::ShowReceiver(WindowImpl *win) : mWin{win} {
    attach(*mWin->mShow);
}

WindowImpl::ShowReceiver::~ShowReceiver() {
    detach();
}

void WindowImpl::ShowReceiver::onChange() {
    mWin->safeCall([this, val=mWin->mShow->val()]() {
        auto *win{dynamic_cast<wxWindow *>(mWin)};
        queueShow(win, val);
        layoutAndFitFor(win);
    });
}

WindowImpl::EnableReceiver::EnableReceiver(WindowImpl *win) : mWin{win} {
    attach(*mWin->mEnable);
}

WindowImpl::EnableReceiver::~EnableReceiver() {
    detach();
}

void WindowImpl::EnableReceiver::onChange() {
    mWin->updateVisualEnable();
}

