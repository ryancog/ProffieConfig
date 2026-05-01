#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/controls/choice.hpp
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

#include "data/base/models/choice.hpp"
#include "data/base/models/number.hpp"
#include "data/base/models/selector.hpp"
#include "data/base/models/string.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/detail/general.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Choice {
    struct Desc;

    // TODO: Make this a base w/ C++ P2287.
    detail::ChildWindowBase win_;

    std::variant<
        RefWrap<data::base::Choice>,
        RefWrap<const data::base::Selector>
    > data_;

    /**
     * Maximum number of choices to display.
     */
    std::optional<RefWrap<const data::base::Integer>> clamp_;

    /**
     * A "ChoiceBox" or PopupButton
     */
    struct UI_EXPORT PopUp {
        /**
         * Entry label for whenever there is no choice selected.
         * Instead of no selection resulting in a blank control.
         */
        wxString unselected_;
    };

    /**
     * Simple, always-visible list.
     */
    struct UI_EXPORT List {
    };

    std::variant<PopUp, List> style_;

    using Label = std::variant<wxString, RefWrap<const data::base::String>>;
    using Labeler = std::function<Label(uint32)>;

    /**
     * Label to use when labeler provides an empty label.
     */
    wxString emptyLabel_;

    Labeler labeler_;

    DescriptorPtr operator()();
};

struct UI_EXPORT Choice::Desc : Choice, detail::Descriptor {
    Desc(Choice&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

} // namespace pcui

