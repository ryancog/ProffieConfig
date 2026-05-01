#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/ui/types.hpp
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

#include <memory>

#include <wx/toplevel.h>
#include <wx/window.h>

#include "data/base/models/string.hpp"
#include "ui/detail/descriptor.hpp"

namespace pcui {

struct DescriptorPtr : std::unique_ptr<detail::Descriptor> {
    using unique_ptr::unique_ptr;
    using unique_ptr::operator=;

    DescriptorPtr(const DescriptorPtr& other) :
        unique_ptr(other ? other->clone() : nullptr) {}

    DescriptorPtr& operator=(const DescriptorPtr& other) {
        if (this == &other) return *this;

        if (other)
            reset(other->clone());

        return *this;
    }
};

template <typename T>
using RefWrap = std::reference_wrapper<T>;

using LabelData = std::variant<
    wxString,
    RefWrap<const data::base::String>
>;

struct CallbackContext {
    wxWindow *window_;
    wxTopLevelWindow *topLevel_;
};

} // namespace pcui

