#include "tlw.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/priv/tlw.cpp
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

#include <wx/toplevel.h>

#ifdef _WIN32
#include <dwmapi.h>
#include <windows.h>

#include "app/app.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"
#include "ui/frame.hpp"
#endif

using namespace pcui;

void priv::tlw::preCreate(wxTopLevelWindow *tlw) {
#   ifdef __WXMSW__
    tlw->SetDoubleBuffered(true);
#   endif
}

void priv::tlw::postCreate(wxTopLevelWindow *tlw) {
#   ifdef _WIN32
    tlw->SetIcon(wxICON(ApplicationIcon));

#   ifdef __WXGTK__
    auto *hwnd{tlw->GTKGetWin32Handle()};
#   else
    auto *hwnd{tlw->GetHWND()};
#   endif

    auto exStyle{GetWindowLongA(hwnd, GWL_EXSTYLE)};
    SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
#   endif
}

void priv::tlw::bindOnCreate(wxTopLevelWindow *tlw) {
    tlw->Bind(wxEVT_CREATE, [tlw](wxWindowCreateEvent& evt) {
        evt.Skip();
        if (evt.GetEventObject() != tlw) return;

#       ifdef _WIN32
#       ifdef __WXGTK__
        auto *hwnd{tlw->GTKGetWin32Handle()};
#       else
        auto *hwnd{tlw->GetHWND()};
#       endif

        HRESULT res{};
        auto& logger{logging::Context::getGlobal().createLogger("TLW Create Event")};

        BOOL useDarkMode{app::darkMode()};
        res = DwmSetWindowAttribute(
            hwnd,
            DWMWA_USE_IMMERSIVE_DARK_MODE,
            &useDarkMode,
            sizeof useDarkMode
        );
        if (res != S_OK) {
            logger.warn("Immersive dark mode setup failed: " + std::to_string(res));
        }

        auto backdrop{dynamic_cast<Frame *>(tlw)
            ? DWMSBT_MAINWINDOW
            : DWMSBT_TRANSIENTWINDOW
        };
        res = DwmSetWindowAttribute(
            hwnd,
            DWMWA_SYSTEMBACKDROP_TYPE,
            &backdrop,
            sizeof backdrop
        );
        if (res != S_OK) {
            logger.warn("Backdrop setup failed: " + std::to_string(res));
        }
#       endif
    });
}

[[nodiscard]] std::unique_ptr<data::String::Receiver>
priv::tlw::setTitle(
    wxTopLevelWindow *tlw,
    const LabelData& title
) {
    if (const auto *ptr{std::get_if<0>(&title)}) {
        tlw->SetTitle(std::get<0>(title));
        return nullptr;
    }

    const auto& data{std::get<1>(title)};
    data::String::ROContext ctxt{data};
    tlw->SetTitle(ctxt.val());

    struct Receiver : data::String::Receiver {
        Receiver(wxTopLevelWindow *tlw) : mTlw{tlw} {}

        ~Receiver() override {
            detach();
        }

        void onChange() override {
            mTlw->SetTitle(context<data::String>().val());
        }

    private:
        wxTopLevelWindow *mTlw;
    };

    auto ret{std::make_unique<Receiver>(tlw)};
    ret->attach(data);
    return ret;
}

