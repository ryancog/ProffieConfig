#include "update.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * launcher/update/update.cpp
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

#include <tomcrypt.h>

#include <utils/types.h>

namespace Update {

wxEvtHandler *handler{nullptr};

} // namespace Update


void Update::init() {
    handler = new wxEvtHandler();
}

wxEvtHandler *Update::getEventHandler() { return handler; }

filepath Update::typeFolder(ItemType type) {
    switch (type) {
        case ItemType::EXEC: return "bin";
        case ItemType::LIB:  return "lib";
        case ItemType::COMP: return "components";
        case ItemType::RSRC: return "resources";
        case TYPE_MAX: break;
    }
    return {};
}

