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

auto data::logic::operator|(const data::Bool& bl, IsSet) -> Element {
    struct Adapter : detail::Base, data::Bool::Receiver {
        Adapter(const data::Bool& bl) : bl_{bl} {}
        ~Adapter() override { detach(); }

        void lock() override {
            bl_.lock();
        }

        void unlock() override {
            bl_.unlock();
        }

        bool doActivate() override {
            data::Bool::ROContext ctxt{bl_};
            attach(bl_);
            return ctxt.val();
        }

        void onSet() override {
            std::lock_guard scopeLock{*pLock};
            onChange(context<Bool>().val());
        }

        const data::Bool& bl_;
    };

    return std::make_unique<Adapter>(bl);
}

auto data::logic::operator|(
    const data::Choice& choice, HasSelection sels
) -> Element {
    struct Adapter : detail::Base, data::Choice::Receiver {
        Adapter(const data::Choice& choice, HasSelection sels) :
            choice_{choice}, sels_{std::move(sels)} {}
        ~Adapter() override { detach(); }

        void lock() override {
            choice_.lock();
        }

        void unlock() override {
            choice_.unlock();
        }

        bool doActivate() override {
            data::Choice::ROContext ctxt{choice_};
            attach(choice_);
            return isTrue(ctxt.choice());
        }

        void onChoice() override {
            std::lock_guard scopeLock{*pLock};
            onChange(isTrue(context<Choice>().choice()));
        }

        bool isTrue(int32 choice) {
            if (sels_.empty()) return choice != -1;
            return sels_.contains(choice);
        }

        const data::Choice& choice_;
        HasSelection sels_;
    };

    return std::make_unique<Adapter>(choice, std::move(sels));
}

auto data::logic::operator|(
    const data::String& choice, IsEmpty
) -> Element {
    struct Adapter : detail::Base, data::String::Receiver {
        Adapter(const data::String& str) :
            str_{str} {}
        ~Adapter() override { detach(); }

        void lock() override {
            str_.lock();
        }

        void unlock() override {
            str_.unlock();
        }

        bool doActivate() override {
            data::String::ROContext ctxt{str_};
            attach(str_);
            return ctxt.val().empty();
        }

        void onChange() override {
            std::lock_guard scopeLock{*pLock};
            Base::onChange(context<String>().val().empty());
        }

        const String& str_;
    };

    return std::make_unique<Adapter>(choice);
}

auto data::logic::operator|(
    const data::Integer& model, BitAnd val
) -> Element {
    struct Adapter : detail::Base, data::Integer::Receiver {
        Adapter(const data::Integer& model, BitAnd val) :
            int_{model}, val_{val} {}
        ~Adapter() override { detach(); }

        void lock() override {
            int_.lock();
        }

        void unlock() override {
            int_.unlock();
        }

        bool doActivate() override {
            data::Integer::ROContext ctxt{int_};
            attach(int_);
            return isTrue(ctxt.val());
        }

        void onSet() override {
            std::lock_guard scopeLock{*pLock};
            Base::onChange(isTrue(context<Integer>().val()));
        }

        [[nodiscard]] bool isTrue(int32 val) const {
            return val & val_.val_;
        }

        BitAnd val_;
        const Integer& int_;
    };

    return std::make_unique<Adapter>(model, val);
}

