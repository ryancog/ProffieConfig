#include "selector.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/selector.cpp
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

#include <wx/panel.h>

#include "ui/build.hpp"
#include "ui/priv/helpers.hpp"
#include "ui/priv/winbase.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/builder.hpp"

using namespace pcui;

namespace {

struct Control : priv::WinBase<wxPanel, data::Selector::Receiver>,
                 data::Choice::Receiver {
    Control(wxWindow *parent, const Selector& desc) {
        Create(parent);

        data::Selector::Receiver::attach(desc.data_);
        data::Choice::Receiver::attach(desc.data_.choice_);
    }

    ~Control() override {
        data::Selector::Receiver::detach();
        data::Choice::Receiver::detach();
    }

    void onChoice(uint32 choice) override {
        data::Vector::Context vec{*vec_};

        build(this, *builder_(&*vec.children()[choice]));
    }

    void onRebound(data::Vector *vec) override {
        vec_ = vec;

        build(this, *builder_(nullptr));
    }

    const detail::DescBuilder builder_;
    data::Vector *vec_;
};

} // namespace

std::unique_ptr<detail::Descriptor> Selector::operator()() {
    return std::make_unique<Selector::Desc>(std::move(*this));
}

Selector::Desc::Desc(Selector&& data) :
    Selector{std::move(data)} {}

wxSizerItem *Selector::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Control(scaffold.childParent_, *this)};
    auto *item{new wxSizerItem(chk)};
    priv::apply(base_, item);
    return item;
}

