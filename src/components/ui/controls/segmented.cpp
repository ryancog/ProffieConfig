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

#include "data/helpers/exclusive.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
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

struct Control : detail::DataWindow<wxToggleButton, data::Model::Receiver> {
    Control(
        const detail::Scaffold& scaffold,
        const Segmented::Label& label,
        data::Bool& data,
        const detail::ChildWindowBase& win
    ) {
        Create(scaffold.childParent_, wxID_ANY, label.text_);
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

        data::Bool::Context ctxt{data};
        SetValue(ctxt.val());

        attach(data);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    Bitmap bmp_;
};

struct Manager : detail::DataWindow<wxPanel, data::Exclusive::Receiver> {
    Manager(const detail::Scaffold& scaffold, const Segmented& desc) {
        Create(scaffold.childParent_);
        auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

        postCreation(scaffold, desc.win_);

        auto childScaffold{scaffold};
        childScaffold.childParent_ = this;
        for (auto idx{0}; idx < desc.data_.data().size(); ++idx) {
            auto& bl{*desc.data_.data()[idx]};
            auto *ctrl{new Control(
                childScaffold,
                desc.labels_[idx],
                bl,
                desc.win_
            )};

            sizer->Add(ctrl);
        }

        SetSizer(sizer);

        attach(desc.data_);
        Bind(wxEVT_TOGGLEBUTTON, &Manager::onSet, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onSet(wxCommandEvent& evt) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        for (auto *child : GetChildren()) {
            if (child != evt.GetEventObject()) continue;

            auto& bl{const_cast<data::Bool&>(
                static_cast<Control *>(child)->model<data::Bool>()
            )};
            auto res{bl.processUIAction(
                std::make_unique<data::Bool::SetAction>(true)
            )};

            if (not res) {
                auto selected{model<data::Exclusive>().selected()};
                auto *child{GetChildren()[selected]};

                static_cast<Control *>(child)->SetValue(true);
            }
            break;
        }
    }
    
    void onSelection(size idx) override {
        safeCall([this, idx] {
            auto& children{GetChildren()};

            for (size childIdx{0}; childIdx < children.size(); ++childIdx) {
                auto *child{static_cast<wxToggleButton *>(children[childIdx])};
                child->SetValue(childIdx == idx);
            }
        });
    }
};

} // namespace

DescriptorPtr Segmented::operator()() {
    // Make sure there's the correct labels for the data.
    assert(labels_.size() == data_.data().size());

    return std::make_unique<Segmented::Desc>(std::move(*this));
}

Segmented::Desc::Desc(Segmented&& data) :
    Segmented{std::move(data)} {}

wxSizerItem *Segmented::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Manager(scaffold, *this)};
    auto *item{new wxSizerItem(chk)};
    detail::apply(win_.base_, item);
    return item;
}

detail::Descriptor *Segmented::Desc::clone() const {
    return new Desc(*this);
}

