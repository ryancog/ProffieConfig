#pragma once
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

#include <string>
#include <unordered_map>
#include <map>
#include <unordered_set>

#include "ui/combobox.h"
#include "ui/toggle.h"
#include "ui/selection.h"
#include "ui/numeric.h"
#include "ui/numericdec.h"

namespace Config::Setting {

struct DefineBase;
struct SettingBase;
template<class> struct Toggle;
template<class> struct Selection;
template<class> struct Numeric;
template<class> struct Decimal;
template<class> struct Combo;

typedef std::map<std::string, DefineBase*> DefineMap;
typedef std::unordered_map<std::string, SettingBase*> SettingMap;

enum class SettingType {
    TOGGLE,
    SELECTION,
    NUMERIC,
    DECIMAL,
    COMBO
};

struct SettingBase {
    std::string name;
    std::string description;

    virtual SettingType getType() const = 0;
    virtual ~SettingBase() {}
};

struct DefineBase : SettingBase {
    std::string define;
    std::string postfix;
    bool pureDef{true};

    DefineMap group;
    std::unordered_set<std::string> require;
    bool requireAny{false};

    virtual SettingType getType() const = 0;
    virtual bool isDisabled();
    virtual ~DefineBase() {}
};


template<class Base>
struct Toggle : Base {
    std::unordered_set<std::string> disable;
    bool value{false};

    PCUI::Toggle* control{nullptr};

    virtual SettingType getType() const { return SettingType::TOGGLE; }
    virtual ~Toggle() {}
};

template<class Base>
struct Selection : Toggle<Base> {
    bool output{true};
    std::unordered_set<Selection*> peers;

    PCUI::Selection* control{nullptr};

    virtual SettingType getType() const { return SettingType::SELECTION; }
    virtual ~Selection() {}
};

template<class Base>
struct Numeric : Base {
    int32_t min{0};
    int32_t max{100};
    int32_t value{min};
    int32_t increment{1};

    PCUI::Numeric* control{nullptr};

    virtual SettingType getType() const { return SettingType::NUMERIC; }
    virtual ~Numeric() {}
};

template<class Base>
struct Decimal : Base {
    double min{0};
    double max{0};
    double value{min};
    double increment{0.1};

    PCUI::NumericDec* control{nullptr};

    virtual SettingType getType() const { return SettingType::DECIMAL; }
    virtual ~Decimal() {}
};

template<class Base>
struct Combo : Base {
    typedef std::unordered_map<std::string, std::string> OptionMap;
    OptionMap options;
    std::string value;

    PCUI::ComboBox* control{nullptr};

    virtual SettingType getType() const { return SettingType::COMBO; }
    virtual ~Combo() {}
};

}
