#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/labeled.hpp
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

#include "ui/detail/general.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

// TODO: In the prior UI stuff I had, *everything* went onto an intermediary
// panel, and the panel, the text, and the control were all tooltipped. The
// panel was overkill, but it would be nice to be able to to have a way to
// tooltip both the static text and control.
//
// I'd like the tooltip from ctrl_ (if it has one) to be applied to the label
// automatically, but ctrl_ being provided as DescriptorPtr and just the
// general way the window data is handled currently (as an arbitrary member)
// makes this tricky.
//
// So yeah... todo I guess.
struct UI_EXPORT Labeled {
    detail::ChildBase base_;

    LabelData label_;
    wxOrientation orient_{wxVERTICAL};

    DescriptorPtr ctrl_;

    DescriptorPtr operator()();
};

} // namespace pcui

