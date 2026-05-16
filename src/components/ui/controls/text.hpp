#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/text.hpp
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

#include "data/base/models/string.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"
#include "ui/font.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Text {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildWindowBase win_;

    std::variant<wxString, RefWrap<data::base::String>> data_;

    /**
     * Implicitly true if only a static string is provided for data.
     */
    bool readOnly_{false};

    /**
     * Generate clickable hyperlinks if URLs are found in the text.
     */
    bool autoLink_{false};

    detail::FontData font_;

    /**
     * Insert "\n" on enter.
     */
    struct InsertLiteral {};

    enum class Wrap {
        None,
        Character,
        Word,
        Best,
    };

    struct SingleLine {
        /**
         * Placeholder text displayed when no text has been entered yet.
         */
        wxString hint_;

        using EnterAction = std::variant<
            std::monostate, std::function<void()>, InsertLiteral
        >;

        /**
         * How to handle the enter key being pressed.
         */
        EnterAction onEnter_;
    };

    struct MultiLine {
        struct {
            bool vertical_{true};
            bool horizontal_{true};
        } scroll_;

        Wrap wrap_{Wrap::Best};
    };

    std::variant<SingleLine, MultiLine> style_;

    DescriptorPtr operator()();
};

struct UI_EXPORT Text::Desc : Text, detail::Descriptor {
    Desc(Text&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

} // namespace pcui

