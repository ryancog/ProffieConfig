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

#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"
#include "ui/types.hpp"

using namespace pcui;

namespace {

struct Indicator : priv::WinBase<wxGauge, Progress::Data::Receiver> {
    Indicator(wxWindow *parent, const Progress& desc) : WinBase(desc.win_) {
        Create(
            parent,
            wxID_ANY,
            0,
            wxDefaultPosition,
            wxDefaultSize,
            wxGA_SMOOTH | desc.orient_ | (desc.showOnBar_ ? wxGA_PROGRESS : 0)
        );

        attach(desc.data_);
    }

    ~Indicator() override {
        detach();
    }
    void onSet(uint32 val) override {
        CallAfter([this, val]{
            SetValue(static_cast<int32>(val));
        });
    }

    void onRange(uint32 val) override {
        CallAfter([this, val]{
            SetRange(static_cast<int32>(val));
        });
    }

    void onPulse() override {
        CallAfter([this]{
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
    auto *bar{new Indicator(scaffold.childParent_, *this)};
    auto *item{new wxSizerItem(bar)};
    priv::apply(base_, item);
    return item;
}

Progress::Data::Data() = default;

Progress::Data::~Data() = default;

void Progress::Data::set(uint32 val) {
    std::lock_guard scopeLock{pLock};
    sendToReceivers(&Receiver::onSet, val);
}

void Progress::Data::range(uint32 val) {
    std::lock_guard scopeLock{pLock};
    sendToReceivers(&Receiver::onRange, val);
}

void Progress::Data::pulse() {
    std::lock_guard scopeLock{pLock};
    sendToReceivers(&Receiver::onPulse);
}

