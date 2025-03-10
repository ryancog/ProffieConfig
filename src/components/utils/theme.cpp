#include "theme.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/theme.cpp
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

#include <map>

#include <wx/app.h>
#include <wx/event.h>
#include <wx/settings.h>

#include <utils/types.h>

namespace Theme {

void init();
void color(wxWindow *, ColorInfo);
void rebuildHierarchy();

// <Win, <Info, Ref>>
std::map<wxWindow *, vector<ColorInfo>>windowLookup;
deque<wxWindow *> colorQueue;

} // namespace Theme

void Theme::colorWindow(wxWindow *window, vector<ColorInfo> colorsInfo) {
    init();
    for (const auto& colorInfo : colorsInfo) color(window, colorInfo);
    // Override default behavior in case of wxPanel
    window->Bind(wxEVT_SYS_COLOUR_CHANGED, [](wxSysColourChangedEvent& evt) { evt.Skip(false); });
    windowLookup.emplace(window, std::move(colorsInfo));
    rebuildHierarchy();

    window->Bind(wxEVT_DESTROY, [window](wxWindowDestroyEvent& evt) {
        evt.Skip();
        if (evt.GetWindow() != window) return;

        windowLookup.erase(windowLookup.find(window));
        rebuildHierarchy();
    });
}

void Theme::init() {
    static bool initialized{false};
    if (initialized) return;
    initialized = true;

    wxApp::GetGUIInstance()->GetTopWindow()->Bind(wxEVT_SYS_COLOUR_CHANGED, [](wxSysColourChangedEvent& evt) {
        evt.Skip();
        for (auto *const win : colorQueue) {
            auto colorsInfo{windowLookup.find(win)->second};
            for (const auto& colorInfo : colorsInfo) color(win, colorInfo);
        }
    });
}


void Theme::color(wxWindow *window, ColorInfo colorInfo) {
    auto *refWin{colorInfo.referenceWindow ? colorInfo.referenceWindow : window->GetParent()};
    wxColour baseColor;
    if (refWin) {
        baseColor = colorInfo.referencePlane == FOREGROUND ? 
            refWin->GetForegroundColour() : 
            refWin->GetBackgroundColour();
    } else baseColor = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);

    auto newColor{
        wxSystemSettings::SelectLightDark(
                baseColor.ChangeLightness(100 + colorInfo.lightOffset), 
                baseColor.ChangeLightness(100 + colorInfo.darkOffset))
    };
    switch (colorInfo.colorPlane) {
        case FOREGROUND:
            window->SetForegroundColour(newColor);
            break;
        case BACKGROUND:
            window->SetBackgroundColour(newColor);
            break;
        default:
            // Make it obviously broken
            auto uglyColor{wxColour(0xFF, 0x00, 0xCC)};
            window->SetBackgroundColour(uglyColor);
            window->SetForegroundColour(uglyColor);
    }

}

void Theme::rebuildHierarchy() {
    std::map<wxWindow *, bool> coloredWindows{};
    for (const auto& [ win, data ] : windowLookup) coloredWindows.emplace(win, false);

    colorQueue.clear();
    for (const auto& winData : windowLookup) {
        auto *win{winData.first};
        auto coloredIt{coloredWindows.find(win)};
        if (coloredIt->second) continue;
        coloredIt->second = true;


        colorQueue.emplace_back(win);
        auto insertIt{std::prev(colorQueue.end())};
        while (win->GetParent()) {
            win = win->GetParent();
            auto coloredIt{coloredWindows.find(win)};
            if (coloredIt != coloredWindows.end()) {
                if (coloredIt->second) break;
                coloredIt->second = true;

                insertIt = colorQueue.emplace(insertIt, win);
            }
        }
    }
}

