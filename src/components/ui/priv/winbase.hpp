#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/priv/winbase.hpp
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

#include <wx/thread.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/general.hpp"
#include "data/logic/logic.hpp"

class wxWindow;

namespace pcui::priv {

void windowPostCreation(
    const detail::Scaffold&,
    const detail::ChildWindowBase&,
    wxWindow *win
);

template <typename Base, typename Receiver>
struct WinBase : Base, Receiver {
    WinBase() = default;

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

        windowPostCreation(scaffold, desc, this);

        updateSizes();
    }

    void onDetach() override {
        updateEnable();
    }

    void onAttach() override {
        updateEnable();
    }

    void onEnabled() override {
        safeCall([this]() {
            updateEnable();
        });
    }

    void onFocus() override {
        safeCall([this]() {
            Base::SetFocus();
        });
    }

    void safeCall(std::function<void()>&& func) {
        if (wxIsMainThread()) func();
        else Base::CallAfter(std::move(func));
    }

    void Fit() override {
        updateSizes();
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
    struct ShowReceiver : data::logic::Receiver {
        ShowReceiver(WinBase *ptr) : winbase_{ptr} {
            attach(*winbase_->mShow);
        }

        void onChange(bool val) override {
            winbase_->safeCall([this, val]() {
                queueShow(winbase_, val);
                layoutAndFitFor(winbase_);
            });
        }

        WinBase *winbase_;
    };
    std::unique_ptr<ShowReceiver> mShowReceiver;
    data::logic::Holder mShow;

    struct EnableReceiver : data::logic::Receiver {
        EnableReceiver(WinBase *ptr) : winbase_{ptr} {
            attach(*winbase_->mEnable);
        }

        void onChange(bool val) override {
            winbase_->safeCall([this, val]() {
                winbase_->updateEnable();
            });
        }

        WinBase *winbase_;
    };
    std::unique_ptr<EnableReceiver> mEnableReceiver;
    data::logic::Holder mEnable;

    void updateEnable() {
        auto *ptr{Receiver::maybeModel()};
        using ModelType = std::decay_t<decltype(*ptr)>;

        Base::Enable(
            (not ptr or typename ModelType::ROContext(*ptr).enabled()) and
            (not mEnable or mEnable->val())
        );
    }

    wxSize mMinSize;
    wxSize mMaxSize;
};

} // namespace pcui::priv

