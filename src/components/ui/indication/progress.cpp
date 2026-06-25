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

#include <utility>

#include <wx/event.h>
#include <wx/gauge.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Indicator : detail::DataWindow<wxGauge> {
    Indicator(const detail::Scaffold& scaffold, const Progress& desc) :
        data_{desc.data_} {
        Create(
            scaffold.childParent_,
            desc.win_.id_,
            0,
            wxDefaultPosition,
            wxDefaultSize,
            wxGA_SMOOTH | desc.orient_ | (desc.showOnBar_ ? wxGA_PROGRESS : 0)
        );

        postCreation(scaffold, desc.win_);

        static const auto table{[] {
            Progress::Data::RecvTable table;
            table.onEnable_ = data::map<&DataWindow::onEnable>();
            table.onSet_ = data::map<&Indicator::onSet>();
            table.onRange_ = data::map<&Indicator::onRange>();
            table.onPulse_ = data::map<&Indicator::onPulse>();
            return table;
        }()};
        observeWith(desc.data_, table);

        activate();
    }

    const data::base::Model *primaryModel() override {
        return &data_;
    }

    void onActivate() override {
        DataWindow::onActivate();

        auto ctxt{data::context(data_)};
        SetRange(static_cast<int>(ctxt.range()));
        SetValue(ctxt.val());
    }

    void onSet() {
        safeCall([this, val=data::context(data_).val()] {
            SetValue(val);
        });
    }

    void onRange() {
        safeCall([this, range=data::context(data_).range()] {
            SetRange(static_cast<int>(range));
        });
    }

    void onPulse() {
        safeCall([this] {
            Pulse();
        });
    }

    const Progress::Data& data_;
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

detail::Descriptor *Progress::Desc::clone() const {
    return new Desc(*this);
}

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
    struct DoneAdapter : data::logic::detail::Base, data::Receiver {
        DoneAdapter(const Data& data) : data_{data} {
            static const auto table{[] {
                RecvTable table;
                table.onSet_ = data::map<&DoneAdapter::onSet>();
                return table;
            }()};

            observeWith(data_, table);
        }

        ~DoneAdapter() override { deactivate(); }

        bool tryLock() override {
            return data_.tryLock();
        }

        void unlock() override {
            data_.unlock();
        }

        bool doActivate() override {
            auto ctxt{data::context(data_)};
            Receiver::activate();
            return isTrue(ctxt);
        }

        void onSet() {
            std::lock_guard scopeLock(*pLock);
            Base::onChange(isTrue(data_));
        }

        static bool isTrue(const ROContext& ctxt) {
            return ctxt.val() == ctxt.range();
        }

        const Data& data_;
    };

    switch (logic) {
        case Logic::Is_Done:
            return std::make_unique<DoneAdapter>(*this);
            break;
    }

    std::unreachable();
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
    data.sendToObservers<&RecvTable::onSet_>();
}

void Progress::Data::Context::setRange(uint32 r) const {
    auto& data{model<Data>()};

    data.mRange = r;
    data.sendToObservers<&RecvTable::onRange_>();
}

void Progress::Data::Context::pulse() const {
    auto& data{model<Data>()};

    data.mVal = -1;
    data.sendToObservers<&RecvTable::onPulse_>();
}

