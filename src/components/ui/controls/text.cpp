#include "text.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/text.cpp
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

#include <wx/font.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>

#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxTextCtrl, data::String::Receiver> {
    Control(const detail::Scaffold& scaffold, const Text& desc) {
        wxString initial;
        long style{0};
        bool handleEnter{false};

        if (desc.readOnly_) style |= wxTE_READONLY;
        if (desc.autoLink_) style |= wxTE_AUTO_URL;

        if (const auto *ptr{std::get_if<Text::SingleLine>(&desc.mode_)}) {
            if (ptr->insertNewline_) {
                handleEnter = true;
                style |= wxTE_PROCESS_ENTER;
            }
        } else {
            const auto& mode{std::get<Text::MultiLine>(desc.mode_)};
            style |= wxTE_PROCESS_TAB;
            style |= wxTE_MULTILINE;
            switch (mode.wrap_) {
                using enum Text::MultiLine::Wrap;
                case None:
                    style |= wxTE_DONTWRAP;
                    break;
                case Character:
                    style |= wxTE_CHARWRAP;
                    break;
                case Word:
                    style |= wxTE_WORDWRAP;
                    break;
                case Best:
                    style |= wxTE_BESTWRAP;
                    break;
            }
        }

        if (desc.data_.index() == 0) {
            style |= wxTE_READONLY;
        }

        Create(
            scaffold.childParent_,
            wxID_ANY,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );

        postCreation(scaffold, desc.win_);
        SetOwnFont(desc.style_.makeFont());

        if (const auto *ptr{std::get_if<Text::SingleLine>(&desc.mode_)}) {
            // Only call set if it's non-empty... these things tend to have
            // side-effects.
            if (not ptr->hint_.empty()) {
                SetHint(ptr->hint_);
            }
        }

#       ifdef __WXMAC__
        if (GetFont().GetFamily() == wxFONTFAMILY_TELETYPE) {
            OSXDisableAllSmartSubstitutions();
        }
#       endif

        if (const auto *ptr{std::get_if<1>(&desc.data_)}) {
            data::String::Context ctxt{*ptr};
            SetValue(ctxt.val());
            attach(*ptr);
        } else {
            SetValue(std::get<0>(desc.data_));
        }

        Bind(wxEVT_TEXT, &Control::onText, this);

        if (handleEnter) {
            Bind(wxEVT_TEXT_ENTER, &Control::onEnter, this);
        }
    }

    void preDestroyCripple() override {
        detach();
        DataWindow::preDestroyCripple();
    }

    void onText(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto& str{const_cast<data::String&>(model<data::String>())};

        auto res{str.processUIAction(std::make_unique<data::String::ChangeAction>(
            GetValue().ToStdString(), posFromInsertPoint(GetInsertionPoint())
        ))};

        if (not res) {
            auto ctxt{context<data::String>()};
            ChangeValue(ctxt.val());
            SetInsertionPoint(insertPointFromPos(ctxt.pos()));
        }
    }

    void onEnter(wxCommandEvent&) {
        // Simply forward the work to normal onText
        WriteText("\\n");
    }
    
    void onChange() override {
        const auto val{context<data::String>().val()};
        safeCall([this, val] {
            ChangeValue(val);
        });
    }

    void onMove() override {
        const auto pos{context<data::String>().pos()};
        safeCall([this, pos] {
            SetInsertionPoint(insertPointFromPos(pos));
        });
    }

    long insertPointFromPos(size pos) const {
        size numNewlines{0};
        ssize lastNewlinePos{-1};

        const auto str{GetValue()};
        assert(pos <= str.length());

        for (long idx{0}; idx < pos; ++idx) {
            if (str[idx] == '\n') {
                ++numNewlines;
                lastNewlinePos = idx;
            }
        }

        return XYToPosition(
            static_cast<long>(pos - lastNewlinePos - 1),
            static_cast<long>(numNewlines)
        );
    }

    size posFromInsertPoint(long ip) const {
        long x{};
        long y{};
        PositionToXY(ip, &x, &y);

        long pos{0};
        for (auto idx{0}; idx < y; ++idx) {
            pos += GetLineLength(idx);
            // And consider the newline
            pos += 1;
        }
        pos += x;

        return pos;
    }
};

} // namespace

std::unique_ptr<detail::Descriptor> Text::operator()() {
    return std::make_unique<Text::Desc>(std::move(*this));
}

Text::Desc::Desc(Text&& data) :
    Text{std::move(data)} {}

wxSizerItem *Text::Desc::build(const detail::Scaffold& scaffold) const {
    auto *ctrl{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(ctrl)};

    detail::apply(win_.base_, item);

    if (const auto *ptr{std::get_if<Text::MultiLine>(&mode_)}) {
        if (not ptr->scroll_.vertical_) {
            const auto lineHeight{ctrl->GetTextExtent('M').y};
            
#ifdef      __WXMSW__
            // The # of lines function is wonky in general, apparently, and is
            // somewhat considered deprecated by wx devs it seems.
            //
            // On MSW, it doesn't work right at all afaics, so here's an
            // incorrect approximation.
            const auto val{ctrl->GetValue()};
            const auto numLines{1 + std::count_if(
                val.begin(),
                val.end(),
                [](char c) { return c == '\n'; }
            )};
#           else
            const auto numLines{ctrl->GetNumberOfLines()};
#           endif
            ctrl->SetMinSize({
                ctrl->GetMinWidth(),
                static_cast<int32>(
                    lineHeight * (static_cast<float64>(numLines) + 0.5)
                )
            });
        }
    }

    return item;
}

