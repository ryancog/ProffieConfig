#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/layers.h
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

#include "styles/bladestyle.h"
#include "styles/elements/colorstyles.h"

namespace BladeStyles {

class LayerStyle : public ColorStyle {
public:

    virtual StyleType getType() const override;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    LayerStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent);

private:
    const BladeStyle* parent;

    static const StyleMap map;
};

}


