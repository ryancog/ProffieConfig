#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/functions.h
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

#include "stylepreview/blade.h"
#include "styles/bladestyle.h"
#include "proffieconstructs/vector3d.h"

namespace BladeStyles {

/*
 * In ProffieOS, Fredrik has created what he calls "SVF"s or "Single Value Functions"
 * The apparent goal is to improve performance by only calculating one value for functions
 * which will have the same value for the entire blade.
 *
 * It's unclear to me exactly why this couldn't be accomplished by doing the calculation in the run()
 * function and then going from there, as that's what the SingleValueAdaptor does anyways, but that's
 * beyond the scope of recreating the style calculation system. 
 * (Thus the reason in ProffieConfig there's only a getInt())
 *
 * SVFs have a single "calculate" function which does the single calculation for the entire blade,
 * in the adapter this value is stored to a variable and then returned for any led getInt() is called
 * with, true to form.
 *
 * getInt() is the function that seems to be actually used by other *things* that expect a "function"
 * bladestyle, it takes in an led index and returns the appropriate value.
 * (It's worth noting that I'm referencing what ProffieOS calls "function" bladestyles and C++
 * functions back and forth here, and they're *NOT* the same thing) 
 */
class FunctionStyle : public BladeStyle {
public:
    /**
     * Used to do initial setup/calculations for each frame
     */
    virtual void run(StylePreview::Blade&) = 0;
    /**
     *  Used to get the value for each LED
     */
    virtual int32_t getInt(int32_t) = 0;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    FunctionStyle(const char* osName, const char* humanName, const std::vector<Param*>&, const BladeStyle* parent, StyleType typeOverride = 0);

private:
    static const StyleMap map;
};

/*
 * There's only two of these, the so-called "Density" functions.
 * As far as I can tell they're only used with Slice, and function like
 * POV data, as stated in the ProffieOS code.
 *
 * Evidently the functionality is intended to be that it looks like the saber
 * is moving through some kind of matrix, the two that exist are both Smoke
 * functions.
 */
class Function3DStyle : public BladeStyle {
public:
    /*
     * Same as FunctionStyle, used to do initial calculations/setup.
     */
    virtual void run(StylePreview::Blade&) = 0;

    /*
     * Same as FunctionStyle, gets value per LED
     */
    virtual int32_t getInt(const Vector3D&) = 0;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    Function3DStyle(const char* osName, const char* humanName, const std::vector<Param*>&, const BladeStyle* parent);

private:
    static const StyleMap map;
};

}
