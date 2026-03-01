#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/spacer.hpp
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

#include <wx/sizer.h>

#include "ui/detail/descriptor.hpp"
#include "utils/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Spacer {
    struct Desc;

    int32 size_{8};

    std::unique_ptr<detail::Descriptor> operator()();
};

struct UI_EXPORT Spacer::Desc : Spacer, detail::Descriptor {
    Desc(Spacer&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
};

struct UI_EXPORT StretchSpacer {
    struct Desc;

    int32 size_{8};
    int32 proportion_{1};

    std::unique_ptr<detail::Descriptor> operator()();
};

struct UI_EXPORT StretchSpacer::Desc : StretchSpacer, detail::Descriptor {
    Desc(StretchSpacer&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
};

} // namespace pcui

