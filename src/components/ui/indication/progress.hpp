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

    enum class Logic {
        Is_Done,
    };

    // TODO: Make this base w/ C++ P2287.
    detail::ChildWindowBase win_;

    Data& data_;

    wxOrientation orient_{wxHORIZONTAL};
    
    /**
     * Show on Taskbar/Dock
     */
    bool showOnBar_{false};

    DescriptorPtr operator()();
};

struct UI_EXPORT Progress::Desc : Progress, detail::Descriptor {
    Desc(Progress&&);

    [[nodiscard]] wxSizerItem *build(const detail::Scaffold&) const override;
    [[nodiscard]] Descriptor *clone() const override;
};

struct UI_EXPORT Progress::Data : data::Model {
    struct ROContext;
    struct Context;
    struct Receiver;

    Data();
    ~Data() override;

    /**
     * Due to the nature in which this data is likely to be used, these are
     * provided; the verbosity of strictly conveying contextual semantics is
     * violated in favor of... sanity, really.
     */
    void set(uint32);
    void range(uint32);

    void pulse();

    data::logic::Element operator|(Logic);

private:
    int32 mVal{0};
    uint32 mRange{100};
};

struct UI_EXPORT Progress::Data::ROContext : virtual Model::ROContext {
    ROContext(const Data&);
    ~ROContext();

    [[nodiscard]] int32 val() const;
    [[nodiscard]] uint32 range() const;
};

struct UI_EXPORT Progress::Data::Context : Model::Context, ROContext {
    Context(Data&);
    ~Context();

    void set(uint32) const;

    void setRange(uint32) const;

    void pulse() const;
};

struct UI_EXPORT Progress::Data::Receiver : Model::Receiver {
protected:
    friend Data;

    /**
     * Value changed
     */
    virtual void onSet() {}

    /**
     * Range updated
     */
    virtual void onRange() {}

    /**
     * Pulsed.
     */
    virtual void onPulse() {}
};

} // namespace pcui

