#include "adapter.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/logic/adapter.cpp
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

auto data::logic::adapt(data::Bool& bl) -> Element {
    struct Adapter : detail::Base, data::Bool::Receiver {
        Adapter(data::Bool& bl) : bl_{bl} {}
        ~Adapter() override { detach(); }

        bool doActivate(ChangeFunc changeFunc) override {
            changeFunc_ = std::move(changeFunc);
            attach(bl_);
            return context<Bool>().val();
        }

        void onSet() override {
            std::lock_guard scopeLock{*pLock};
            changeFunc_(context<Bool>().val());
        }

        data::Bool& bl_;
        ChangeFunc changeFunc_;
    };

    return std::make_unique<Adapter>(bl);
}

