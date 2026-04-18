#include "choice.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/choice.cpp
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

#include <wx/choice.h>
#include <wx/event.h>
#include <wx/listbox.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

template<typename Derived, typename Ctrl>
struct ControlBase : detail::DataWindow<Ctrl, data::Choice::Receiver>,
                     data::Selector::Receiver,
                     data::Vector::Receiver {
    void preDestroyCripple() override {
        data::Choice::Receiver::detach();
        data::Selector::Receiver::detach();
        data::Vector::Receiver::detach();

        detail::DataWindow<Ctrl, data::Choice::Receiver>::preDestroyCripple();
    }

    void onChoice(wxCommandEvent& evt) {
        auto en{this->freezeGetRealEnable()};
        defer { this->thawRealEnable(); };

        if (not en) return;
        
        auto& ch{const_cast<data::Choice&>(
            data::Choice::Receiver::model<data::Choice>()
        )};

        const auto self{static_cast<Derived *>(this)};
        auto res{ch.processUIAction(
            std::make_unique<data::Choice::ChoiceAction>(
                self->controlToData(evt.GetInt())
            )
        )};

        if (not res) {
            auto ctxt{data::Choice::Receiver::context<data::Choice>()};
            const auto self{static_cast<Derived *>(this)};
            this->SetSelection(self->dataToControl(ctxt.idx()));
        }
    }
    
    void onChoice() override {
        const auto self{static_cast<Derived *>(this)};
        const auto model{data::Choice::Receiver::context<data::Choice>()};
        this->safeCall([this, self, choice=model.idx()] {
            Ctrl::SetSelection(self->dataToControl(choice));
        });
    }

    void onUpdate(data::Choice::Receiver::UpdateInfo info) override {
        const auto self{static_cast<Derived *>(this)};
        const auto choices{self->generateChoices(
            data::Choice::Receiver::context<data::Choice>().numChoices()
        )};

        this->safeCall([this, choices, info] {
            auto sel{Ctrl::GetSelection()};
            Ctrl::Set(choices);
            if (info.choicePreserved_) Ctrl::SetSelection(sel);
        });
    }

    [[nodiscard]] int32 controlToData(int32 choice) const {
        return choice;
    }

    [[nodiscard]] int32 dataToControl(int32 choice) const {
        return choice;
    }

    std::vector<wxString> generateChoices(uint32 num) {
        std::vector<wxString> choices;

        mRcvrs.clear();

        for (uint32 idx{0}; idx < num; ++idx) {
            if (not mLabeler) {
                choices.emplace_back("LABEL???");
                continue;
            }

            auto res{mLabeler(idx)};

            wxString str;
            if (auto *ptr{std::get_if<1>(&res)}) {
                const auto self{static_cast<Derived *>(this)};
                const auto& model{std::get<1>(res).get()};
                data::String::ROContext ctxt{model};
                str = ctxt.val();
                mRcvrs.push_back(std::make_unique<LabelReceiver>(
                    this, self->dataToControl(idx), model
                ));
            } else {
                str = std::move(std::get<0>(res));
            }

            if (str.empty()) choices.push_back(mEmptyLabel);
            else choices.emplace_back(std::move(str));
        }

        return choices;
    }

    void onRebound() override {
        auto selCtxt{data::Selector::Receiver::context<data::Selector>()};

        data::Vector::Receiver::detach();
        if (selCtxt.bound()) data::Vector::Receiver::attach(*selCtxt.bound());
    }

    void onSwap(size size) override {
        auto tmp{Ctrl::GetString(size)};
        Ctrl::SetString(size, Ctrl::GetString(size + 1));
        Ctrl::SetString(size + 1, tmp);

        // Also swap receiver idxs if found
        for (auto& rcvr : mRcvrs) {
            auto& labelRcvr{static_cast<LabelReceiver&>(*rcvr)};
            if (labelRcvr.idx_ == size) labelRcvr.idx_ = size + 1;
            else if (labelRcvr.idx_ == size + 1) labelRcvr.idx_ = size;
        }
    }

private:
    friend Derived;

    void create(const detail::Scaffold& scaffold, const Choice& desc) {
        mLabeler = desc.labeler_;
        mEmptyLabel = desc.emptyLabel_;

        Ctrl::Create(
            scaffold.childParent_,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize
        );

        detail::DataWindow<Ctrl, data::Choice::Receiver>::postCreation(
            scaffold, desc.win_
        );

        data::Choice *choice{};
        const data::Selector *sel{};
        if (const auto *ptr{std::get_if<1>(&desc.data_)}) {
            sel = &ptr->get();
            choice = &ptr->get().choice_;
        } else choice = &std::get<0>(desc.data_).get();

        data::Choice::Context ctxt{*choice};
        const auto self{static_cast<Derived *>(this)};
        Ctrl::Set(self->generateChoices(ctxt.numChoices()));
        Ctrl::SetSelection(self->dataToControl(
            ctxt.idx()
        ));

        data::Choice::Receiver::attach(*choice);
        if (sel) {
            data::Selector::ROContext selCtxt{*sel};
            if (selCtxt.bound()) {
                data::Vector::Receiver::attach(*selCtxt.bound());
            }
            data::Selector::Receiver::attach(*sel);
        }

        Ctrl::Bind(Derived::evt(), &ControlBase::onChoice, this);
    }

    ControlBase() = default;

    ControlBase(const detail::Scaffold& scaffold, const Choice& desc) {
        create(scaffold, desc);
    }

    std::vector<std::unique_ptr<data::String::Receiver>> mRcvrs;
    Choice::Labeler mLabeler;
    wxString mEmptyLabel;

    struct LabelReceiver final : data::String::Receiver {
        LabelReceiver(
            ControlBase *ctrl,
            uint32 idx,
            const data::String& model
        ) : mCtrl{ctrl}, idx_{idx} {
            attach(model);
        }

        ~LabelReceiver() override {
            detach();
        }

        void onChange() override {
            // Capture info by value, the receiver could die before the UI
            // updates occur.
            mCtrl->safeCall([
                ctrl=mCtrl, idx=idx_, val=context<data::String>().val()
            ] {
                ctrl->SetString(idx, val);
            });
        }

        uint32 idx_;

    private:
        ControlBase *mCtrl;
    };
};

struct PopUpControl : ControlBase<PopUpControl, wxChoice> {
    PopUpControl(
        const detail::Scaffold& scaffold,
        const Choice& desc,
        const Choice::PopUp& style
    ) : unselected_{style.unselected_} {
        create(scaffold, desc);
    }

    static const auto& evt() { return wxEVT_CHOICE; }

    [[nodiscard]] int32 controlToData(int32 choice) const {
        return unselected_.empty() ? choice : choice - 1;
    }

    [[nodiscard]] int32 dataToControl(int32 choice) const {
        return unselected_.empty() ? choice : choice + 1;
    }

    std::vector<wxString> generateChoices(uint32 num) {
        std::vector<wxString> choices;

        if (not unselected_.empty()) {
            choices.push_back(unselected_);
        }

        auto tmp{ControlBase::generateChoices(num)};
        choices.reserve(choices.size() + tmp.size());

        for (auto& label : tmp) {
            choices.push_back(std::move(label));
        }

        return choices;
    }

    wxString unselected_;
};

struct ListControl : ControlBase<ListControl, wxListBox> {
    ListControl(
        const detail::Scaffold& scaffold,
        const Choice& desc,
        const Choice::List&
    ) : ControlBase(scaffold, desc) {
    }

    static const auto& evt() { return wxEVT_LISTBOX; }
};

} // namespace

DescriptorPtr Choice::operator()() {
    return std::make_unique<Choice::Desc>(std::move(*this));
}

Choice::Desc::Desc(Choice&& data) :
    Choice{std::move(data)} {}

wxSizerItem *Choice::Desc::build(const detail::Scaffold& scaffold) const {
    wxWindow *ctrl{};
    if (const auto *ptr{std::get_if<PopUp>(&style_)}) {
        ctrl = new PopUpControl(scaffold, *this, *ptr);
    } else if (const auto *ptr{std::get_if<List>(&style_)}) {
        ctrl = new ListControl(scaffold, *this, *ptr);
    }

    auto *item{new wxSizerItem(ctrl)};
    detail::apply(win_.base_, item);
    return item;
}

detail::Descriptor *Choice::Desc::clone() const {
    return new Desc(*this);
}

