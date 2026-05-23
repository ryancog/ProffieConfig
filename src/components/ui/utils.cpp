#include "utils.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/utils.cpp
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

#include <wx/app.h>
#include <wx/thread.h>
#include <wx/window.h>

void pcui::safeCall(const std::function<void()>& func) {
    if (wxIsMainThread()) func();
    else wxTheApp->CallAfter(func);
}

wxWindow *pcui::getUniqueChild(const wxWindow *win) {
    // This aims to do the same thing as wxTopLevelWindowBase::GetUniqueChild()
    // since that's a private func, and also doesn't work for non-tlw.
    wxWindow *ret{nullptr};
    for (auto *child : win->GetChildren()) {
        if (child->IsTopLevel() or not win->IsClientAreaChild(child))
            continue;

        // There's more than one client area child.
        if (ret)
            return nullptr;

        ret = child;
    }

    return ret;
}

