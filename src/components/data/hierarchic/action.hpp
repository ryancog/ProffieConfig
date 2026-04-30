#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/action.hpp
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

#include <cassert>

#include "data_export.h"

namespace data::hier {

struct Model;

struct DATA_EXPORT Action {
    virtual ~Action();

    /**
     * If a new action is added that matches the type of the previous one,
     * this will be called on the previous with the new to provide an
     * opportunity to coalesce the actions together, however is appropriate
     * for the action.
     *
     * By default, this does nothing.
     *
     * @return Whether or not the action was absorbed
     *         (if true will not be added)
     */
    [[nodiscard]] virtual bool maybeCoalesce(Action&);

    /**
     * @return whether or not the action will actually do anything/should be
     *         performed.
     */
    [[nodiscard]] virtual bool setup() = 0;

    /**
     * Perform the action on the Model
     */
    virtual void perform() = 0;

    /**
     * Retract the action on the Model
     */
    virtual void retract() = 0;

protected:
    Action();

    template <typename M>
    [[nodiscard]] M& source() const {
        assert(mSource);
        return static_cast<M&>(*mSource);
    }

private:
    friend Model;

    Model *mSource{nullptr};
};

} // namespace data::hier

