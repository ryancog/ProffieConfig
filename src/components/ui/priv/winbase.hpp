#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/declarative/priv/winbase.hpp
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

namespace pcui::declarative::priv {

template <typename Base, typename Receiver>
struct WinBase : Base, Receiver {
    void onAttach() override {
        Base::Enable(Receiver::context().enabled());
    }

    void onDetach() override {
        Base::Disable();
    }

    void onEnabled(bool en) override {
        Base::Enable(en);
    }

    void onShown(bool show) override {
        Base::Show(show);
    }

    void onFocus() override {
        Base::SetFocus();
    }
};

} // namespace pcui::declarative::priv

