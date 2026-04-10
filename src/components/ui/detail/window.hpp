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

#include "ui/detail/general.hpp"
#include "ui/detail/helpers.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datadriven.hpp"

namespace pcui::detail {

/**
 * Common utilities for a window.
 */
template <typename Base>
struct Window : IDataDriven, Base {
    void Fit() override {
        updateSizes();
    }

    void preDestroyCripple() override {
        mShowReceiver.reset();
        mShow.reset();
        mEnableReceiver.reset();
        mEnable.reset();
    }

    /**
     * Freeze the and fetch the current enable state, preventing anything from
     * modifying it.
     *
     * Unlike just the usual GUI enable state, which may become slightly
     * desynchronized with the data which dictates whether or not the window
     * may be enabled, this provides a reliable indicator.
     */
    bool freezeGetRealEnable() {
        if (not mEnable) return true;

        mEnable->lock();
        return mEnable->val();
    }

    void thawRealEnable() {
        if (mEnable) mEnable->unlock();
    }

    void updateVisualEnable() {
        safeCall([this]() {
            // Don't worry about locking here. If this happens to see something
            // a bit early that's fine.
            const auto winEn{not mEnable or mEnable->val()};
            const auto en{winEn and visualEnableOverride()};
            Base::Enable(en);
        });
    }

    /**
     * @return visual override from a higher level
     */
    virtual bool visualEnableOverride() { return true; }

protected:
    Window() = default;

    /**
     * Post window creation, prior to receiver attachment.
     */
    void postCreation(
        const detail::Scaffold& scaffold, const detail::ChildWindowBase& desc
    ) {
        mMinSize = desc.base_.minSize_;
        mMaxSize = desc.maxSize_;

        mShow = desc.show_;
        if (mShow) mShowReceiver = std::make_unique<ShowReceiver>(this);

        mEnable = desc.enable_;
        if (mEnable) mEnableReceiver = std::make_unique<EnableReceiver>(this);

        if (desc.tooltip_.empty() and this->GetParent()) {
            this->SetToolTip(this->GetParent()->GetToolTip());
        } else this->SetToolTip(desc.tooltip_);

        if (scaffold.scrolled_) {
            const auto onWheel{[scrolled=scaffold.scrolled_](
                wxMouseEvent& evt
            ) {
                scrolled->HandleOnMouseWheel(evt);
            }};
            this->Bind(wxEVT_MOUSEWHEEL, onWheel);
        }

        updateSizes();
    }

    void safeCall(std::function<void()>&& func) {
        if (wxIsMainThread()) func();
        else Base::CallAfter(std::move(func));
    }

    virtual void updateSizes() {
        Base::SetMinSize({-1, -1});

        auto minSize{mMinSize};
        minSize.IncTo(Base::GetBestSize());

        if (mMaxSize.IsFullySpecified()) {
            minSize.DecTo(mMaxSize);
            Base::SetMaxSize(mMaxSize);
        }

        Base::SetMinSize(minSize);
    }

private:
    template <typename, typename>
    friend struct DataWindow;

    struct ShowReceiver : data::logic::Receiver {
        ShowReceiver(Window *win) : win_{win} {
            attach(*win_->mShow);
        }

        ~ShowReceiver() override {
            detach();
        }

        void onChange() override {
            win_->safeCall([this, val=win_->mShow->val()]() {
                detail::queueShow(win_, val);
                detail::layoutAndFitFor(win_);
            });
        }

        Window *win_;
    };
    data::logic::Holder mShow;
    std::unique_ptr<ShowReceiver> mShowReceiver;

    struct EnableReceiver : data::logic::Receiver {
        EnableReceiver(Window *win) : win_{win} {
            attach(*win_->mEnable);
        }

        ~EnableReceiver() override {
            detach();
        }

        void onChange() override {
            win_->updateVisualEnable();
        }

        Window *win_;
    };
    data::logic::Holder mEnable;
    std::unique_ptr<EnableReceiver> mEnableReceiver;

    wxSize mMinSize;
    wxSize mMaxSize;
};

} // namespace pcui::detail

