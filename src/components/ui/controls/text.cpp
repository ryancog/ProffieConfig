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

#include "data/context.hpp"
#include "ui/detail/scaffold.hpp"
#include "ui/detail/datawin.hpp"
#include "ui/types.hpp"
#include "utils/defer.hpp"

using namespace pcui;

namespace {

struct Control : detail::DataWindow<wxTextCtrl> {
    Control(const detail::Scaffold& scaffold, const Text& desc) {
        wxString initial;
        // This appearance is broken right now: https://github.com/wxWidgets/wxWidgets/issues/26551
        // long style{wxTE_RICH2};
        long style{0};

        if (desc.readOnly_) style |= wxTE_READONLY;
        if (desc.autoLink_) style |= wxTE_AUTO_URL;

        if (const auto *ptr{std::get_if<Text::SingleLine>(&desc.style_)}) {
            onEnter_ = ptr->onEnter_;
        } else {
            const auto& mode{std::get<Text::MultiLine>(desc.style_)};
            style |= wxTE_PROCESS_TAB;
            style |= wxTE_MULTILINE;
            switch (mode.wrap_) {
                using enum Text::Wrap;
                case None:
                    if (mode.scroll_.horizontal_)
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

            if (not mode.scroll_.vertical_)
                style |= wxTE_NO_VSCROLL;
        }

        if (desc.data_.index() == 0) {
            style |= wxTE_READONLY;
        }

        if (onEnter_.index() != 0) {
            style |= wxTE_PROCESS_ENTER;
        }

        Create(
            scaffold.childParent_,
            desc.win_.id_,
            wxEmptyString,
            wxDefaultPosition,
            wxDefaultSize,
            style
        );

        postCreation(scaffold, desc.win_);
        SetOwnFont(desc.font_.makeFont());

        if (const auto *ptr{std::get_if<Text::SingleLine>(&desc.style_)}) {
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

        if (const auto *ptr{std::get_if<0>(&desc.data_)}) {
            SetValue(*ptr);
            return;
        }

        str_ = &std::get<1>(desc.data_).get();

        static const auto table{[] {
            data::base::String::RecvTable table;
            table.onEnable_ = data::map(&DataWindow::onEnable);
            table.onFocus_ = data::map(&DataWindow::onFocus);
            table.onChange_ = data::map(&Control::onChange);
            table.onMove_ = data::map(&Control::onMove);
            return table;
        }()};
        observeWith(*str_, table);

        activate();
    }

    void onActivate() override {
        DataWindow::onActivate();

        SetValue(data::context(*str_).val());

        Bind(wxEVT_TEXT, &Control::onText, this);
        if (onEnter_.index() != 0)
            Bind(wxEVT_TEXT_ENTER, &Control::onEnter, this);
    }

    const data::base::Model *primaryModel() override {
        return str_;
    }

    void onText(wxCommandEvent&) {
        auto en{freezeGetRealEnable()};
        defer { thawRealEnable(); };

        if (not en) return;

        auto res{str_->change(
            GetValue().utf8_string(), posFromInsertPoint(GetInsertionPoint())
        )};

        if (not res) {
            auto ctxt{data::context(*str_)};
            ChangeValue(ctxt.val());
            SetInsertionPoint(insertPointFromPos(ctxt.pos()));
        }
    }

    void onEnter(wxCommandEvent&) {
        if (auto *ptr{std::get_if<std::function<void()>>(&onEnter_)}) {
            (*ptr)();
        } else if (std::get_if<Text::InsertLiteral>(&onEnter_)) {
            // Simply forward the work to normal onText
            WriteText("\\n");
        }
    }
    
    void onChange() {
        safeCall([this, val=data::context(*str_).val()] {
            ChangeValue(val);
        });
    }

    void onMove() {
        safeCall([this, pos=data::context(*str_).pos()] {
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

    Text::SingleLine::EnterAction onEnter_;
    data::base::String *str_{nullptr};
};

} // namespace

DescriptorPtr Text::operator()() {
    return std::make_unique<Text::Desc>(std::move(*this));
}

Text::Desc::Desc(Text&& data) :
    Text{std::move(data)} {}

wxSizerItem *Text::Desc::build(const detail::Scaffold& scaffold) const {
    auto *ctrl{new Control(scaffold, *this)};
    auto *item{new wxSizerItem(ctrl)};

    detail::apply(win_.base_, item);

    if (const auto *ptr{std::get_if<Text::MultiLine>(&style_)}) {
        wxSize minSize;

        // The # of lines function is wonky in general, apparently, and is
        // somewhat considered deprecated by wx devs it seems.
        //
        // On MSW, it doesn't work right at all, and tries to count physical/
        // displayed lines rather than logical ones. GetLineText is similarly
        // unhelpful for MSW.
        //
        // Since GetLineText under the hood, for at least macOS, isn't any more
        // interesting than what you'd expect, just do this for all platforms.
        const auto val{ctrl->GetValue().utf8_string()};
        auto computeLines{[](const std::string& val) {
            std::vector<std::string_view> lines;
            lines.resize(1);

            auto startIter{val.begin()};
            for (auto iter{val.begin()}; iter != val.end(); ++iter) {
                if (*iter == '\n') {
                    lines.emplace_back(startIter, iter);
                    startIter = std::next(iter);
                }
            }
            lines.emplace_back(startIter, val.end());

            return lines;
        }};
        auto lines{computeLines(val)};

        if (not ptr->scroll_.vertical_) {
            const auto lineHeight{ctrl->GetCharHeight()};
            
            minSize.y = static_cast<int32>(
                lineHeight * (static_cast<float64>(lines.size()) + 0.5)
            );
        }

        if (not ptr->scroll_.horizontal_) {
            int32 width{0};

            for (int32 idx{0}; idx < lines.size(); ++idx) {
                // This has to be a wxString to pass to GetTextExtent().
                wxString line(lines[idx]);
                auto extent{ctrl->GetTextExtent(line)};
                width = std::max(width, extent.GetWidth());
            }

            minSize.x = width + (ctrl->GetCharWidth() * 2);
        }

        minSize.IncTo(ctrl->GetMinClientSize());
        ctrl->SetMinClientSize(minSize);
    }

    return item;
}

detail::Descriptor *Text::Desc::clone() const {
    return new Desc(*this);
}

