#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/helpers.hpp
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
#include <wx/toplevel.h>

#include "ui_export.h"

namespace pcui::detail {

/**
 * Queue a window to be shown when updates take place.
 *
 * This is done to ensure that a window is always shown in the same event as
 * it's laid out to ensure the window doesn't appear in an unexpected place to
 * the user.
 */
UI_EXPORT void queueShow(wxWindow *, bool);

/**
 * Layout this window and its parents recursively, queued to ensure multiple
 * updates only require a single set of recalculations.
 */
UI_EXPORT void layoutAndFitFor(wxWindow *);

/**
 * Immediately processes any shows queued for the given window and any
 * children.
 */
UI_EXPORT void flushShowQueueFor(wxWindow *);

/**
 * Immediately processes any layout queued for the given window and any
 * children.
 */
UI_EXPORT void flushLayoutQueueFor(wxWindow *);

/**
 * Discard any layout actions that were for this window or any of its children.
 * Presumably they were/are going to be handled elsewhere.
 */
UI_EXPORT void discardLayoutsFor(wxWindow *);

} // namespace pcui::detail

