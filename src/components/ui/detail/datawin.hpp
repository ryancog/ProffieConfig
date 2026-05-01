#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/detail/datawin.hpp
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

#include <wx/thread.h>

#include "data/base/model.hpp"
#include "ui/detail/window.hpp"

#include "ui_export.h"

class wxWindow;

namespace pcui::detail {

// TODO: Whenever I update the data logic stuff, this and Window<> also need
// reworking. Similarly ugly things here.

struct UI_EXPORT DataWindowImpl : virtual WindowImpl {
    void onActivate() override;
    void onDeactivate() override;

    void onEnable();
    void onFocus();

    virtual const data::base::Model *primaryModel() = 0;

    bool freezeGetRealEnable() override;
    void thawRealEnable() override;

private:
    bool visualEnableOverride() override;
};

template <typename Base>
struct DataWindow : DataWindowImpl, Window<Base> {
    DataWindow() = default;
};

} // namespace pcui::detail

