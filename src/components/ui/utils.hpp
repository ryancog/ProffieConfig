#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/utils.hpp
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

#include <functional>

#include "ui_export.h"

class wxWindow;

namespace pcui {

UI_EXPORT void safe_call(const std::function<void()>&);

/**
 * If there's a single child of the window, get it.
 */
[[nodiscard]] UI_EXPORT wxWindow *get_unique_child(const wxWindow *);

/**
 * For whenever just "some" sizer is needed, for inserting a receiver, and
 * mainly specifically for pcui::Group, where there may or may not be a sizer
 * immediately within a window, it varies between platforms and style.
 */
[[nodiscard]] UI_EXPORT wxSizer *find_sizer(const wxWindow *);

} // namespace pcui

