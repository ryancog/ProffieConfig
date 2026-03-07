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

#include "ui/detail/general.hpp"
#include "data/logic/logic.hpp"

namespace pcui::priv {

template <typename Base, typename Receiver>
struct WinBase : Base, Receiver {
    WinBase(const detail::ChildWindowBase& desc) {
        mShow = desc.show_;
        if (mShow) mShowReceiver = std::make_unique<ShowReceiver>(this);

        Base::SetToolTip(desc.tooltip_);
    }

    void onAttach() override {
        Base::CallAfter([this]() {
            Base::Enable(Receiver::context().enabled());
        });
    }

    void onDetach() override {
        Base::CallAfter([this]() {
            Base::Disable();
        });
    }

    void onEnabled() override {
        Base::CallAfter([this]() {
            Base::Enable(Receiver::context().enabled());
        });
    }

    void onFocus() override {
        Base::CallAfter([this]() {
            Base::SetFocus();
        });
    }

private:
    struct ShowReceiver : data::logic::Receiver {
        ShowReceiver(WinBase *ptr) : winbase_{ptr} {
            attach(*winbase_->mShow);
        }

        void onChange(bool val) override {
            winbase_->CallAfter([this, val]() {
                winbase_->Base::Enable(val);
            });
        }

        WinBase *winbase_;
    };
    std::unique_ptr<ShowReceiver> mShowReceiver;
    data::logic::Holder mShow;
};

} // namespace pcui::priv

