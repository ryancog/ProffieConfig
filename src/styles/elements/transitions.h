#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/transitions.h
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
#include "stylepreview/blade.h"
#include "styles/elements/colorstyles.h"

namespace BladeStyles {

/*
 * Well here we are again, it's always such a pleasure...
 * Trying to understand Transitions:
 *
 * TransitionBaseX<class MILLIS> functions:
 *
 * run(BladeBase* blade) -> TransitionStyle::run (will wrap virtual doRun call after setup)
 * bool done(); // Check if length? is equal to 0
 * bool restart(); // check reset var
 * void begin(); // set restart var to true to be check on next run() 
 *
 * uint32_t start_millis(); // Get start time in millis
 *
 * // Unsure what scale is supposed to be, checks to see if the transition is done.
 * // If so, scale is directly returned. (This will also update the state to a done state if it's crossed
 * // the threshold and hasn't been updated yet)... perhaps that's where the "update" name comes from?
 * // Otherwise, the MILLIS class, wrapped in AddBend<>, has bend called with the timeDelta, length, and scale
 * template<typename T>
 * T update(T scale); 
 *
 * Ok, so TrWipeX will be our example. It inherits publicly from TransitionBaseX<MILLIS>,
 * and on the surface it looks like a standard color function, with a run and getColor, however
 * the getColor is templated (of course it is), and the colors come from the getColor call, it doesn't
 * just take in the led index.
 *
 * (Sidenote, this'd mean that Transitions are particularly expensive in memory...)
 *
 * So it seems the special thing about transitions is that their getColor takes in colors, and
 * that they inherit (and use!) all the functions from TransitionBaseX.
 *
 * TransitionHelper<class T> function:
 *
 * void begin();
 * void run(BladeBase* blade)
 * operator bool();
 *
 * // If "active_", return T::getColor(a, b, int),
 * // else return b...
 * template<class A, class B>
 * Color getColor(A, B, int);
 *
 * Studying a "Layer" that takes in a transition,
 * InOutTrL wraps the "OutTr" and "InTr" input transition classes
 * in a "TransitionHelper"
 *
 * There's the actual "Transition Functions" as I'm dubbing them (this is getting complicated),
 * and then there's the elements that take in transitions, and they don't seem to be any different in
 * interface than a normal ColorStyle, so they'll probably go with those.
 *
 * The run function seems to serve the same role as always, doing
 * main setup, and getColor is pretty standard.
 *
 *
 */

// This is *almost* entirely an interface so that on the backend
// I can have non-X wrappers.
//
// I'll probably do things more this way for the other elements too.
class TransitionStyle : public BladeStyle {
public:
    // Not sure if all this needs to be public interface
    //
    // The familiar functions are the same as always
    virtual void run(StylePreview::Blade&) = 0;
    virtual void begin() = 0;
    virtual bool isDone() const = 0;
    virtual bool shouldRestart() const = 0;
    virtual uint32_t getStartMillis() const = 0;
    virtual ColorData getColor(const ColorData&, const ColorData&, int32_t) = 0;

    static StyleGenerator get(const std::string& styleName);
    static const StyleMap& getMap();

protected:
    TransitionStyle(const char* osName, const char* humanName, const std::vector<Param*>&, const BladeStyle* parent);

private:
    static const StyleMap map;

};

}
