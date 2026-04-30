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

#include "data/context.hpp"
#include "data/receiver.hpp"

// TODO: Looking back on this, it all feels kind of ugly. I'm not sure what a
// better solution might look like. Building some kind of operator/link
// representation and then having a single object that is a model and has a
// RecvTable w/ a boolean update cb feels like the most consistent way.
//
// That'd require splitting base::Model apart and having some other base the
// Receiver uses as a base (so that the base::Model enable and focus stuff
// isn't unnecessarily present), that has the main interface stuff. It's
// probably a good idea anyways, but I don't know what that should look like
// organizationally.
//
// The representation is also kind of a tedious issue of its own.
//
// Anyways, what's here right now is a half-baked fudge adaptation from the
// old models stuff and so doesn't make much sense. Not that it made a
// terrible amount of sense to begin with.

auto data::logic::operator|(const base::Model& model, IsEnabled) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::Model& model) : model_{model} {
            static const auto table{[] {
                base::Model::RecvTable table;
                table.onEnable_ = map(&Adapter::onEnabled);
                return table;
            }()};

            pRecvMap[&model_] = &table;
        }
        
        ~Adapter() override { deactivate(); }

        void lock() override {
            model_.lock();
        }

        void unlock() override {
            model_.unlock();
        }

        bool doActivate() override {
            base::Model::ROContext ctxt{model_};
            Receiver::activate();
            return ctxt.enabled();
        }

        void onEnabled() {
            std::lock_guard scopeLock(*pLock);
            onChange(context(model_).enabled());
        }

        const base::Model& model_;
    };

    return std::make_unique<Adapter>(model);
}

auto data::logic::operator|(const base::Bool& bl, IsSet) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::Bool& bl) : bl_{bl} {
            static const auto table{[] {
                base::Bool::RecvTable table;
                table.onSet_ = map(&Adapter::onSet);
                return table;
            }()};

            pRecvMap[&bl_] = &table;
        }

        ~Adapter() override { deactivate(); }

        void lock() override {
            bl_.lock();
        }

        void unlock() override {
            bl_.unlock();
        }

        bool doActivate() override {
            base::Bool::ROContext ctxt{bl_};
            Receiver::activate();
            return ctxt.val();
        }

        void onSet() {
            std::lock_guard scopeLock(*pLock);
            onChange(context(bl_).val());
        }

        const base::Bool& bl_;
    };

    return std::make_unique<Adapter>(bl);
}

auto data::logic::operator|(
    const base::Choice& choice, HasSelection sels
) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::Choice& choice, HasSelection sels) :
            choice_{choice}, sels_{std::move(sels)} {
            static const auto table{[] {
                base::Choice::RecvTable table;
                table.onChoice_ = map(&Adapter::onChoice);
                return table;
            }()};

            pRecvMap[&choice_] = &table;
        }

        ~Adapter() override { deactivate(); }

        void lock() override {
            choice_.lock();
        }

        void unlock() override {
            choice_.unlock();
        }

        bool doActivate() override {
            base::Choice::ROContext ctxt{choice_};
            Receiver::activate();
            return isTrue(ctxt.idx());
        }

        void onChoice() {
            std::lock_guard scopeLock(*pLock);
            onChange(isTrue(context(choice_).idx()));
        }

        bool isTrue(int32 choice) {
            if (sels_.empty()) return choice != -1;
            return sels_.contains(choice);
        }

        const base::Choice& choice_;
        HasSelection sels_;
    };

    return std::make_unique<Adapter>(choice, std::move(sels));
}

auto data::logic::operator|(
    const base::String& choice, IsEmpty
) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::String& str) : str_{str} {
            static const auto table{[] {
                base::String::RecvTable table;
                table.onChange_ = map(&Adapter::onChange);
                return table;
            }()};

            pRecvMap[&str_] = &table;
        }

        ~Adapter() override { deactivate(); }

        void lock() override {
            str_.lock();
        }

        void unlock() override {
            str_.unlock();
        }

        bool doActivate() override {
            base::String::ROContext ctxt{str_};
            Receiver::activate();
            return ctxt.val().empty();
        }

        void onChange() {
            std::lock_guard scopeLock(*pLock);
            Base::onChange(context(str_).val().empty());
        }

        const base::String& str_;
    };

    return std::make_unique<Adapter>(choice);
}

auto data::logic::operator|(
    const base::Vector& choice, IsEmpty
) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::Vector& vec) : vec_{vec} {
            static const auto table{[] {
                base::Vector::RecvTable table;
                table.onInsert_ = map(&Adapter::onInsert);
                table.preRemove_ = map(&Adapter::preRemove);
                return table;
            }()};

            pRecvMap[&vec_] = &table;
        }

        ~Adapter() override { deactivate(); }

        void lock() override {
            vec_.lock();
        }

        void unlock() override {
            vec_.unlock();
        }

        bool doActivate() override {
            base::Vector::ROContext ctxt{vec_};
            Receiver::activate();
            return isTrue(ctxt);
        }

        void onInsert(size) {
            std::lock_guard scopeLock(*pLock);
            onChange(isTrue(vec_));
        }

        void preRemove(size) {
            std::lock_guard scopeLock(*pLock);
            onChange(isTrue(vec_));
        }

        static bool isTrue(const base::Vector::ROContext& ctxt) {
            return ctxt.children().empty();
        }

        const base::Vector& vec_;
    };

    return std::make_unique<Adapter>(choice);
}

auto data::logic::operator|(
    const base::Integer& model, BitAnd val
) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::Integer& model, BitAnd val) :
            int_{model}, val_{val} {
            static const auto table{[] {
                base::Integer::RecvTable table;
                table.onSet_ = map(&Adapter::onSet);
                return table;
            }()};

            pRecvMap[&int_] = &table;
        }

        ~Adapter() override { deactivate(); }

        void lock() override {
            int_.lock();
        }

        void unlock() override {
            int_.unlock();
        }

        bool doActivate() override {
            base::Integer::ROContext ctxt{int_};
            Receiver::activate();
            return isTrue(ctxt.val());
        }

        void onSet() {
            std::lock_guard scopeLock(*pLock);
            onChange(isTrue(context(int_).val()));
        }

        [[nodiscard]] bool isTrue(int32 val) const {
            return val & val_.val_;
        }

        BitAnd val_;
        const base::Integer& int_;
    };

    return std::make_unique<Adapter>(model, val);
}

auto data::logic::operator|(
    const base::Integer& model, Equals equals
) -> Element {
    struct Adapter : detail::Base, data::Receiver {
        Adapter(const base::Integer& model, Equals equals) :
            int_{model}, equals_{equals} {
            static const auto table{[] {
                base::Integer::RecvTable table;
                table.onSet_ = map(&Adapter::onSet);
                return table;
            }()};

            pRecvMap[&int_] = &table;
        }

        ~Adapter() override { deactivate(); }

        void lock() override {
            int_.lock();
        }

        void unlock() override {
            int_.unlock();
        }

        bool doActivate() override {
            base::Integer::ROContext ctxt{int_};
            Receiver::activate();
            return isTrue(ctxt.val());
        }

        void onSet() {
            std::lock_guard scopeLock(*pLock);
            onChange(isTrue(context(int_).val()));
        }

        [[nodiscard]] bool isTrue(int32 val) const {
            return val == equals_.val_;
        }

        Equals equals_;
        const base::Integer& int_;
    };

    return std::make_unique<Adapter>(model, equals);
}

