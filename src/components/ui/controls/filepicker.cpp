#include "filepicker.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/filepicker.cpp
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

#include <wx/filepicker.h>

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxFilePickerCtrl> {
    Control(const detail::Scaffold& scaffold, const FilePicker &desc) :
        str_{desc.data_} {
        // Use textctrl default is platform-dependent, check if it exists in
        // the defaults instead of unilaterally setting it.
        long style{wxFLP_DEFAULT_STYLE & wxFLP_USE_TEXTCTRL};

        if (const auto *ptr{std::get_if<FilePicker::Open>(&desc.mode_)}) {
            style |= wxFLP_OPEN;
            if (ptr->mustExist_) style |= wxFLP_FILE_MUST_EXIST;
        } else {
            const auto& mode{std::get<FilePicker::Save>(desc.mode_)};
            style |= wxFLP_SAVE;
            if (mode.confirmOverwrite_) style |= wxFLP_OVERWRITE_PROMPT;
        }

        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxEmptyString,
            desc.message_,
            desc.wildcard_,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );

        postCreation(scaffold, desc.win_);

        static const auto table{[] {
            data::base::String::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onChange_ = data::map(&Control::onChange);
            return table;
        }()};
        observeWith(str_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        SetPath(data::context(str_).val());

        Bind(wxEVT_FILEPICKER_CHANGED, &Control::onPick, this);
    }

    const data::base::Model *primaryModel() override {
        return &str_;
    }

    void onPick(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{str_.change(GetPath().utf8_string(), GetPath().length())};

        if (not res)
            SetPath(data::context(str_).val());
    }

    void onChange() {
        safeCall([this, val=data::context(str_).val()] {
            SetPath(val);
        });
    }

    data::base::String& str_;
};

} // namespace

DescriptorPtr FilePicker::operator()() {
    return std::make_unique<FilePicker::Desc>(std::move(*this));
}

FilePicker::Desc::Desc(FilePicker&& data) :
    FilePicker{std::move(data)} {}

wxSizerItem *FilePicker::Desc::build(const detail::Scaffold& scaffold) const {
    auto *pick{new Control(scaffold, *this)};

    auto *item{new wxSizerItem(pick)};
    detail::apply(win_.base_, item);

    return item;
}

detail::Descriptor *FilePicker::Desc::clone() const {
    return new Desc(*this);
}

