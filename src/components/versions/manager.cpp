#include "manager.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/versions/manager.cpp
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

#include <wx/sizer.h>

namespace Versions {

Manager *Manager::smInstance;

} // namespace Versions

void Versions::Manager::open(wxWindow *parent, wxWindowID id) {
    if (smInstance) smInstance->Raise();
    else smInstance = new Manager(parent, id);
}

bool Versions::Manager::Destroy() {
    smInstance = nullptr;
    return PCUI::Frame::Destroy();
}

Versions::Manager::Manager(wxWindow *parent, wxWindowID id) :
    PCUI::Frame(parent, id, _("Versions Manager")) {
    
    createUI();
    Show();
}

void Versions::Manager::createUI() {
    auto *sizer{new wxBoxSizer(wxVERTICAL)};

    SetSizerAndFit(sizer);
}

