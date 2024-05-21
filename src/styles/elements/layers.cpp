#include "layers.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/layers.cpp
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

using namespace BladeStyles;

LayerStyle::LayerStyle(const char* osName, const char* humanName, const std::vector<Param*>& params, const BladeStyle* parent) :
    ColorStyle(osName, humanName, params, parent, BladeStyles::LAYER) {}

StyleGenerator LayerStyle::get(const std::string& styleName) {
    const auto& mapIt{map.find(styleName)};
    if (mapIt == map.end()) return nullptr;
    return mapIt->second;
}

// TODO
StyleType LayerStyle::getType() const { return type; }

const StyleMap& LayerStyle::getMap() { return map; }

#define RUN(varname) virtual void run(const StylePreview::Blade& varname) override
#define GETINT(varname) virtual uint16_t getInt(uint32_t varname) override
#define GETINT3D(varname) virtual uint16_t getInt(const StylePreview::Vector3D& varname) override

#define LAYER(osName, humanName, params, ...) \
    class osName : public LayerStyle { \
    public: \
        osName() : LayerStyle(#osName, humanName, params) {} \
        __VA_ARGS__ \
    }; 

const StyleMap LayerStyle::map{

};

