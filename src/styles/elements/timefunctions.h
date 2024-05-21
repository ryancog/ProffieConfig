#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek,
 * adapted from ProffieOS code, copyright Fredrik Hubinette et al.
 *
 * styles/elements/timefunctions.h
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
#include "styles/elements/functions.h"
#include "stylepreview/blade.h"

namespace BladeStyles {

class TimeFunctionStyle : public FunctionStyle {
public:

    // Note the normal functionStyle functions are here virtually

    // The actual manipulation function
    // Similar to update() this is templated... so we use a double instead.
    // I don't like this, obviously... but...
    virtual double bend(uint32_t time, uint32_t length, double scale) = 0;

    static const StyleMap& getMap();
    static StyleGenerator get(const std::string& styleName);

protected:
    TimeFunctionStyle(const char* osName, const char* humanName, const std::vector<Param*>&, const BladeStyle* parent);

private:
    static const StyleMap map;
};

// This doesn't get added to the map, it's for internal use only (and Transition)!
class AddBend : public TimeFunctionStyle {
public:
  AddBend() : TimeFunctionStyle(
              "AddBend", 
              "SCARY",
              PARAMS(new StyleParam("MILLIS", FUNCTION, nullptr)),
              nullptr
              ) {}

  virtual void run(StylePreview::Blade& blade) override {
      millisStyle = const_cast<FunctionStyle*>(static_cast<const FunctionStyle*>(getParamStyle(0)));
      millisStyle->run(blade);
  }
  virtual double bend(uint32_t time, uint32_t length, double scale) override {
    return (scale * time) / length;
  }
  virtual int32_t getInt(int32_t led) override {
    return millisStyle->getInt(led);
  }

private:
  FunctionStyle* millisStyle;
};



}
