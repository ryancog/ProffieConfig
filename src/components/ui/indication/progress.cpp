#include "progress.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/indication/progress.cpp
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

#include <wx/event.h>
#include <wx/gauge.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Indicator : detail::DataWindow<wxGauge, Progress::Data::Receiver> {
    Indicator(const detail::Scaffold& scaffold, const Progress& desc) {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            0,
            wxDefaultPosition,
            wxDefaultSize,
            wxGA_SMOOTH | desc.orient_ | (desc.showOnBar_ ? wxGA_PROGRESS : 0)
        );

        postCreation(scaffold, desc.win_);

        attach(desc.data_);
    }

    void preDestroyCripple() override {
        detach();
    }

    void onSet() override {
        safeCall([this, val=context<Progress::Data>().val()] {
            SetValue(val);
        });
    }

    void onRange() override {
        safeCall([this, range=context<Progress::Data>().range()] {
            SetRange(static_cast<int32>(range));
        });
    }

    void onPulse() override {
        safeCall([this]{
            Pulse();
        });
    }
};

} // namespace

DescriptorPtr Progress::operator()() {
    return std::make_unique<Progress::Desc>(std::move(*this));
}

Progress::Desc::Desc(Progress&& prog) : Progress(std::move(prog)) {}

wxSizerItem *Progress::Desc::build(const detail::Scaffold& scaffold) const {
    auto *bar{new Indicator(scaffold, *this)};
    auto *item{new wxSizerItem(bar)};
    detail::apply(win_.base_, item);
    return item;
}

Progress::Data::Data() = default;

Progress::Data::~Data() = default;

void Progress::Data::set(uint32 val) {
    Context{*this}.set(val);
}

void Progress::Data::range(uint32 range) {
    Context{*this}.setRange(range);
}

void Progress::Data::pulse() {
    Context{*this}.pulse();
}

data::logic::Element Progress::Data::operator|(Logic logic) {
    struct DoneAdapter : data::logic::detail::Base, Receiver {
        DoneAdapter(const Data& data) : data_{data} {}
        ~DoneAdapter() override { detach(); }

        void lock() override {
            data_.lock();
        }

        void unlock() override {
            data_.unlock();
        }

        bool doActivate() override {
            attach(data_);
            return isTrue();
        }

        void onSet() override {
            std::lock_guard scopeLock{*pLock};
            Base::onChange(isTrue());
        }

        bool isTrue() {
            auto ctxt{context<Data>()};
            return ctxt.val() == ctxt.range();
        }

        const Data& data_;
    };

    switch (logic) {
        case Logic::Is_Done:
            return std::make_unique<DoneAdapter>(*this);
            break;
    }

    assert(0);
    __builtin_unreachable();
}

Progress::Data::ROContext::ROContext(const Data& data) :
    Model::ROContext(data) {}

Progress::Data::ROContext::~ROContext() = default;

int32 Progress::Data::ROContext::val() const {
    const auto& data{model<Data>()};
    return data.mVal;
}

uint32 Progress::Data::ROContext::range() const {
    const auto& data{model<Data>()};
    return data.mRange;
}

Progress::Data::Context::Context(Data& data) :
    Model::Context(data), ROContext(data), Model::ROContext(data) {}

Progress::Data::Context::~Context() = default;

void Progress::Data::Context::set(uint32 v) const {
    auto& data{model<Data>()};

    data.mVal = static_cast<int32>(v);
    data.sendToReceivers(&Receiver::onSet);
}

void Progress::Data::Context::setRange(uint32 r) const {
    auto& data{model<Data>()};

    data.mRange = r;
    data.sendToReceivers(&Receiver::onRange);
}

void Progress::Data::Context::pulse() const {
    auto& data{model<Data>()};

    data.mVal = -1;
    data.sendToReceivers(&Receiver::onPulse);
}

