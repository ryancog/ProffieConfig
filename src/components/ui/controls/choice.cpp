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

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

template <typename Ctrl>
struct ControlBase : detail::DataWindow<Ctrl> {
    using DataWindow = detail::DataWindow<Ctrl>;

    ControlBase() = default;

    ControlBase(const detail::Scaffold& scaffold, const Choice& desc) {
        create(scaffold, desc);
    }

    void create(const detail::Scaffold& scaffold, const Choice& desc) {
        mLabeler = desc.labeler_;
        mEmptyLabel = desc.emptyLabel_;

        Ctrl::Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxDefaultPosition,
            wxDefaultSize
        );

        DataWindow::postCreation(
            scaffold, desc.win_
        );

        static const auto choiceTable{[] {
            data::base::Choice::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onChoice_ = data::map(&ControlBase::onChoice);
            table.onUpdate_ = data::map(&ControlBase::onUpdate);
            return table;
        }()};

        if (const auto *ptr{std::get_if<1>(&desc.data_)}) {
            choice_ = &ptr->get().choice();
            sel_ = &ptr->get();

            static const auto selTable{[] {
                data::base::Selector::RecvTable table;
                table.preRebound_ = data::map(&ControlBase::preSelRebound);
                table.onRebound_ = data::map(&ControlBase::onSelRebound);
                return table;
            }()};
            data::Receiver::observeWith(*sel_, selTable);
        } else {
            choice_ = &std::get<0>(desc.data_).get();
        }

        data::Receiver::observeWith(*choice_, choiceTable);

        if (desc.clamp_) {
            clamp_ = &desc.clamp_->get();

            static const auto clampTable{[] {
                data::base::Integer::RecvTable table;
                table.onSet_ = data::map(&ControlBase::onClampSet);
                return table;
            }()};
            data::Receiver::observeWith(*clamp_, clampTable);
        }
    }

    void onActivate() override {
        DataWindow::onActivate();

        auto ctxt{data::context(*choice_)};
        Ctrl::Set(generateChoices(ctxt.num()));
        Ctrl::SetSelection(dataToControl(ctxt.idx()));

        if (sel_) {
            auto selCtxt{data::context(*sel_)};
            if (selCtxt.bound()) {
                data::Receiver::observeWith(*selCtxt.bound(), VEC_TABLE);
            }
        }

        Ctrl::Bind(event(), &ControlBase::onChoiceEvt, this);
    }

    const data::base::Model *primaryModel() override {
        return choice_;
    }

    void onChoiceEvt(wxCommandEvent& evt) {
        auto en{this->freezeGetRealEnable()};
        defer { this->thawRealEnable(); };

        if (not en) return;
        
        auto res{choice_->choose(controlToData(evt.GetInt()))};

        if (not res) {
            auto ctxt{data::context(*choice_)};
            Ctrl::SetSelection(dataToControl(ctxt.idx()));
        }
    }
    
    void onChoice() {
        const auto idx{data::context(*choice_).idx()};
        detail::WindowImpl::safeCall([this, idx] {
            Ctrl::SetSelection(dataToControl(idx));
        });
    }

    void onUpdate(data::base::Choice::UpdateInfo info) {
        const auto choices{
            generateChoices(data::context(*choice_).num())
        };

        detail::WindowImpl::safeCall([this, choices, info] {
            auto sel{Ctrl::GetSelection()};
            Ctrl::Set(choices);
            if (info.choicePreserved_) Ctrl::SetSelection(sel);
        });
    }

    void preSelRebound() {
        if (const auto *ptr{data::context(*sel_).bound()})
            data::Receiver::repeal(*ptr);
    }

    void onSelRebound() {
        auto ctxt{data::context(*sel_)};
        if (not ctxt.bound()) return;

        data::Receiver::observeWith(*ctxt.bound(), VEC_TABLE);
    }

    void onClampSet() {
        const auto choices{
            generateChoices(data::context(*choice_).num())
        };

        detail::WindowImpl::safeCall([this, choices] {
            auto sel{Ctrl::GetSelection()};
            Ctrl::Set(choices);
            Ctrl::SetSelection(sel);
        });
    }

    void onVectorSwap(size idx) {
        for (auto& [model, mapIdx] : mLabelMapping) {
            if (mapIdx == idx)
                mapIdx = idx + 1;
            else if (mapIdx == idx + 1)
                mapIdx = idx;
        }

        detail::WindowImpl::safeCall([this, idx] {
            auto tmp{Ctrl::GetString(idx)};
            Ctrl::SetString(idx, Ctrl::GetString(idx + 1));
            Ctrl::SetString(idx + 1, tmp);
        });
    }

    void onLabelChange(const data::base::Model& model) {
        const auto& strModel{dynamic_cast<const data::base::String&>(model)};
        auto str{data::context(strModel).val()};
        auto idx{mLabelMapping[&strModel]};

        detail::WindowImpl::safeCall([
            this, idx=dataToControl(idx), str=std::move(str)
        ] {
            Ctrl::SetString(idx, str.empty() ? mEmptyLabel : str);
        });
    }

    [[nodiscard]] virtual int32 controlToData(int32 choice) const {
        return choice;
    }

    [[nodiscard]] virtual int32 dataToControl(int32 choice) const {
        return choice;
    }

    [[nodiscard]] virtual const wxEventTypeTag<wxCommandEvent>&
        event() const = 0;

    virtual std::vector<wxString> generateChoices(uint32 num) {
        std::lock_guard scopeLock(data::Receiver::pMutex);

        std::vector<wxString> choices;

        for (auto [str, idx] : mLabelMapping) {
            data::Receiver::repeal(*str);
        }
        mLabelMapping.clear();

        if (clamp_)
            num = std::min<uint32>(data::context(*clamp_).val(), num);

        for (uint32 idx{0}; idx < num; ++idx) {
            if (not mLabeler) {
                choices.emplace_back("LABEL???");
                continue;
            }

            auto res{mLabeler(idx)};

            wxString str;
            if (auto *ptr{std::get_if<1>(&res)}) {
                auto ctxt{data::context(ptr->get())};
                str = ctxt.val();

                mLabelMapping[&ptr->get()] = idx;

                static const auto labelTable{[] {
                    data::base::String::RecvTable table;
                    table.onChange_ = data::map(&ControlBase::onLabelChange);
                    return table;
                }()};
                data::Receiver::observeWith(ptr->get(), labelTable);
            } else {
                str = std::move(std::get<0>(res));
            }

            if (str.empty()) choices.push_back(mEmptyLabel);
            else choices.emplace_back(std::move(str));
        }

        return choices;
    }

    // Should never be null, but it's assigned late, so ptr.
    data::base::Choice *choice_;
    const data::base::Selector *sel_{nullptr};
    const data::base::Integer *clamp_{nullptr};

private:
    std::map<const data::base::String *, uint32> mLabelMapping;
    Choice::Labeler mLabeler;
    wxString mEmptyLabel;

    static const data::base::Vector::RecvTable VEC_TABLE;
};

template <typename Ctrl>
const data::base::Vector::RecvTable ControlBase<Ctrl>::VEC_TABLE{[] {
    data::base::Vector::RecvTable table;
    table.onSwap_ = data::map(&ControlBase::onVectorSwap);
    return table;
}()};

struct PopUpControl : ControlBase<wxChoice> {
    PopUpControl(
        const detail::Scaffold& scaffold,
        const Choice& desc,
        const Choice::PopUp& style
    ) : unselected_{style.unselected_} {
        create(scaffold, desc);
        activate();
    }

    const wxEventTypeTag<wxCommandEvent>& event() const override {
        return wxEVT_CHOICE;
    }

    [[nodiscard]] int32 controlToData(int32 choice) const override {
        return unselected_.empty() ? choice : choice - 1;
    }

    [[nodiscard]] int32 dataToControl(int32 choice) const override {
        return unselected_.empty() ? choice : choice + 1;
    }

    std::vector<wxString> generateChoices(uint32 num) override {
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

struct ListControl : ControlBase<wxListBox> {
    ListControl(
        const detail::Scaffold& scaffold,
        const Choice& desc,
        const Choice::List&
    ) : ControlBase(scaffold, desc) {
        activate();
    }

    const wxEventTypeTag<wxCommandEvent>& event() const override {
        return wxEVT_LISTBOX;
    }
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

