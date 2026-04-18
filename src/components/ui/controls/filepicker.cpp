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

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxFilePickerCtrl, data::String::Receiver> {
    Control(const detail::Scaffold& scaffold, const FilePicker &desc) {
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
            wxID_ANY,
            wxEmptyString,
            desc.message_,
            desc.wildcard_,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );

        postCreation(scaffold, desc.win_);

        data::String::Context ctxt{desc.data_};
        SetPath(ctxt.val());

        attach(desc.data_);
        Bind(wxEVT_FILEPICKER_CHANGED, &Control::onPick, this);
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onPick(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto& str{const_cast<data::String&>(model<data::String>())};
        auto path{GetPath().ToStdString()};
        const auto len{path.length()};

        auto res{str.processUIAction(std::make_unique<data::String::ChangeAction>(
            std::move(path), len
        ))};

        if (not res) {
            auto ctxt{context<data::String>()};
            SetPath(ctxt.val());
        }
    }

    void onChange() override {
        const auto val{context<data::String>().val()};
        safeCall([this, val] {
            SetPath(val);
        });
    }
};

} // namespace

DescriptorPtr FilePicker::operator()() {
    return std::make_unique<FilePicker::Desc>(std::move(*this));
}

FilePicker::Desc::Desc(FilePicker&& data) :
    FilePicker{std::move(data)} {}

wxSizerItem *FilePicker::Desc::build(const detail::Scaffold& scaffold) const {
    auto *chk{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(chk)};
    detail::apply(win_.base_, item);
    return item;
}

detail::Descriptor *FilePicker::Desc::clone() const {
    return new Desc(*this);
}

