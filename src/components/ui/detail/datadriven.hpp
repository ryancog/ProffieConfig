#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/datadriven.hpp
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

namespace pcui::detail {

struct IDataDriven {
    virtual ~IDataDriven() = default;

    /**
     * Separate window destruction, which must occur on main thread, from
     * detachment of receivers and events.
     *
     * E.g. Receivers/data a window relies upon is disappearing and so the
     * window needs to forget it *now*, but if this occurs on non-main thread,
     * destruction would be delayed.
     *
     * Additionally, when data is locked, as in a data event, trying to send an
     * event to the main queue and waiting for it could very easily cause a
     * deadlock with an earlier event waiting on the very data currently
     * locked.
     */
    virtual void preDestroyCripple() = 0;
};

} // namespace pcui::detail

