#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/window.hpp
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

#include <wx/scrolwin.h>

#include "data/receiver.hpp"
#include "ui/detail/general.hpp"
#include "ui/detail/scaffold.hpp"

#include "ui_export.h"

namespace pcui::detail {

struct UI_EXPORT WindowImpl : data::Receiver {
    void onDeactivate() override;

    /**
     * Freeze the and fetch the current enable state, preventing anything from
     * modifying it.
     *
     * Unlike just the usual GUI enable state, which may become slightly
     * desynchronized with the data which dictates whether or not the window
     * may be enabled, this provides a reliable indicator.
     */
    virtual bool freezeGetRealEnable();

    virtual void thawRealEnable();

    void updateVisualEnable();

    virtual bool visualEnableOverride() { return true; }

protected:
    WindowImpl() = default;

    /**
     * Post window creation, prior to receiver attachment.
     */
    void postCreation(
        const Scaffold& scaffold, const ChildWindowBase& desc
    );

    void safeCall(const std::function<void()>& func);

    virtual void updateSizes();

private:
    struct ShowReceiver : data::logic::Receiver {
        ShowReceiver(WindowImpl *win);
        ~ShowReceiver() override;
        void onChange() override;
    private:
        WindowImpl *mWin;
    };
    data::logic::Holder mShow;
    std::unique_ptr<ShowReceiver> mShowReceiver;

    struct EnableReceiver : data::logic::Receiver {
        EnableReceiver(WindowImpl *win);
        ~EnableReceiver() override;
        void onChange() override;
    private:
        WindowImpl *mWin;
    };
    data::logic::Holder mEnable;
    std::unique_ptr<EnableReceiver> mEnableReceiver;

    wxSize mMinSize;
    wxSize mMaxSize;
};

/**
 * Common utilities for a window.
 */
template <typename Base>
struct Window : Base, virtual WindowImpl {
    void Fit() override {
        updateSizes();
    }
};

} // namespace pcui::detail

