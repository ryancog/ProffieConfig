#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/indication/progress.hpp
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

#include "data/hierarchy/model.hpp"
#include "ui/detail/general.hpp"
#include "ui/types.hpp"

#include "ui_export.h"

namespace pcui {

struct UI_EXPORT Progress {
    struct Desc;
    struct Data;

    // TODO: Make these base w/ C++ P2287.
    detail::ChildBase base_;
    detail::ChildWindowBase win_;

    Data& data_;

    wxOrientation orient_{wxVERTICAL};
    
    /**
     * Show on Taskbar/Dock
     */
    bool showOnBar_{false};

    DescriptorPtr operator()();
};

struct UI_EXPORT Progress::Desc : Progress, detail::Descriptor {
    Desc(Progress&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
};

struct UI_EXPORT Progress::Data : data::Model {
    struct Receiver;

    Data();
    ~Data() override;

    void set(uint32);
    void range(uint32);

    void pulse();

private:
    uint32 mVal{0};
    uint32 mRange{100};
};

struct UI_EXPORT Progress::Data::Receiver : Model::Receiver {
protected:
    friend Data;

    /**
     * Value changed
     */
    virtual void onSet(uint32) {}

    /**
     * Range updated
     */
    virtual void onRange(uint32) {}

    /**
     * Pulsed.
     */
    virtual void onPulse() {}
};

} // namespace pcui

