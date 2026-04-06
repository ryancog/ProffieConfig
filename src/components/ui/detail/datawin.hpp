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

#include "ui/detail/window.hpp"

class wxWindow;

namespace pcui::detail {

template <typename Base, typename Receiver>
struct DataWindow : Window<Base>, Receiver {
    DataWindow() = default;

    void onDetach() override {
        Window<Base>::updateVisualEnable();
    }

    void onAttach() override {
        Window<Base>::updateVisualEnable();
    }

    void onEnabled() override {
        Window<Base>::updateVisualEnable();
    }

    void onFocus() override {
        this->safeCall([this]() {
            Window<Base>::SetFocus();
        });
    }

    // Derived likely has receivers, it must handle detach, so nothing to
    // override preDestroyCripple() here for.

    bool freezeGetRealEnable() {
        auto *ptr{Receiver::maybeModel()};
        using ModelType = std::decay_t<decltype(*ptr)>;

        bool modelEn{true};
        if (ptr) {
            ptr->lock();
            modelEn = typename ModelType::ROContext(*ptr).enabled();
        }

        return Window<Base>::freezeGetRealEnable() and modelEn;
    }

    void thawRealEnable() {
        auto *ptr{Receiver::maybeModel()};
        using ModelType = std::decay_t<decltype(*ptr)>;

        if (ptr) ptr->unlock();

        Window<Base>::thawRealEnable();
    }

private:
    bool visualEnableOverride() override {
        auto *ptr{Receiver::maybeModel()};
        using ModelType = std::decay_t<decltype(*ptr)>;

        if (not ptr) return true;
        return typename ModelType::ROContext(*ptr).enabled();
    }
};

} // namespace pcui::detail

