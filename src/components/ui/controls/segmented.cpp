#include "segmented.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/segmented.cpp
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
#include <wx/sizer.h>
#include <wx/tglbtn.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/layout/priv/panel.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

/*
 * This is a segmented control in spirit...
 *
 * Making it a proper-looking one would be nice. I know one exists for macOS
 * and probably GTK... not sure about Windows (probably not).
 *
 * In any case, afaik it's not something wxWidgets exposes, so it'd be
 * non-trivial to add.
 */

namespace {

struct Control : detail::DataWindow<wxToggleButton> {
    Control(
        const detail::Scaffold& scaffold,
        const Segmented::Label& label,
        data::base::Bool& data,
        const detail::ChildWindowBase& win
    ) : bl_{data} {
        Create(
            scaffold.childParent_,
            wxID_ANY,
            label.text_
        );

        if (label.bmp_) {
            bmp_ = label.bmp_;

            const auto updateBitmap{[this]() {
                SetBitmap(bmp_.realize());
            }};

            Bind(
                wxEVT_SYS_COLOUR_CHANGED,
                [updateBitmap](wxSysColourChangedEvent& evt) {
                    evt.Skip();
                    updateBitmap();
                }
            );

            updateBitmap();
        }

        postCreation(scaffold, win);

        static const auto table{[] {
            data::base::Bool::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onSet_ = data::map(&Control::onSet);
            return table;
        }()};
        amend(bl_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        SetValue(data::context(bl_).val());

        Bind(wxEVT_TOGGLEBUTTON, &Control::onButton, this);
    }

    const data::base::Model *primaryModel() override {
        return &bl_;
    }

    void onButton(wxCommandEvent& evt) {
        if (not evt.GetInt()) {
            SetValue(true);
            return;
        }

        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        wxToggleButton *last{nullptr};
        for (auto *child : GetParent()->GetChildren()) {
            auto *btn{static_cast<wxToggleButton *>(child)};
            if (btn->GetValue()) {
                last = btn;
                break;
            }
        }
        assert(last != nullptr);

        auto ctxt{data::context(bl_)};

        ctxt.set(true);

        if (not ctxt.val())
            SetValue(false);
    }

    void onSet() {
        safeCall([this, val=data::context(bl_).val()] {
            SetValue(val);
        });
    }
    
    Bitmap bmp_;
    data::base::Bool& bl_;
};

struct Manager : detail::Window<priv::Panel> {
    Manager(const detail::Scaffold& scaffold, const Segmented& desc) {
        create(scaffold.childParent_, desc.win_.id_);
        auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

        postCreation(scaffold, desc.win_);

        auto childScaffold{scaffold};
        childScaffold.childParent_ = this;

        auto ctxt{data::context(desc.data_)};
        for (size idx{0}; idx < ctxt.num(); ++idx) {
            auto& bl{ctxt[idx]};
            auto *ctrl{new Control(
                childScaffold,
                desc.labels_[idx],
                bl,
                desc.win_
            )};
            sizer->Add(ctrl);
        }

        SetSizer(sizer);

        activate();
    }
};

} // namespace

DescriptorPtr Segmented::operator()() {
    // Make sure there's the correct labels for the data.
    assert(labels_.size() == data::context(data_).num());

    return std::make_unique<Segmented::Desc>(std::move(*this));
}

Segmented::Desc::Desc(Segmented&& data) :
    Segmented{std::move(data)} {}

wxSizerItem *Segmented::Desc::build(const detail::Scaffold& scaffold) const {
    auto *seg{new Manager(scaffold, *this)};

    auto *item{new wxSizerItem(seg)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *Segmented::Desc::clone() const {
    return new Desc(*this);
}

