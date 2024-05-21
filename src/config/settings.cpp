#include "settings.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * config/settings.h
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

bool Config::Setting::DefineBase::isDisabled() {
    bool hasRequired{requireAny ? false : true};
    for (const auto& reqName : require) {
        auto req{group.find(reqName)};
        if (req == group.end() && !requireAny) {
            hasRequired = false;
            break;
        }

        if (req->second->getType() != SettingType::TOGGLE && req->second->getType() != SettingType::SELECTION) continue;
        auto castReq{static_cast<Toggle<DefineBase>*>(req->second)};

        if (requireAny && castReq->value) {
            hasRequired = true;
            break;
        }
        if (!requireAny) hasRequired &= castReq->value;
    }

    bool disabled{false};
    for (const auto& [ _, disabler ] : group) {
        if (disabler->getType() != SettingType::TOGGLE && disabler->getType() != SettingType::SELECTION) continue;

        auto castDis{static_cast<Toggle<DefineBase>*>(disabler)};
        if (castDis->disable.find(define) == castDis->disable.end()) continue;

        disabled |= !castDis->value;
    }

    return !hasRequired || disabled;
}
