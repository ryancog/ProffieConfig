#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * prop/propfile.h
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
#include <unordered_set>
#include <vector>

#include "config/settings.h"

namespace PropFile {

struct Data;

std::vector<Data*> getPropData(const std::vector<std::string>& pconfs);

struct Data {
    struct LayoutItem;
    struct LayoutLevel;

    struct ButtonMap;
    struct ButtonState;
    struct Button;

    std::string name{};
    std::string filename{};

    std::string info{};

    Config::Setting::DefineMap* settings{nullptr};

    typedef std::vector<LayoutItem*> LayoutVec;
    LayoutVec* layout{nullptr};

    typedef std::unordered_map<uint8_t, ButtonMap*> Buttons;
    Buttons buttonControls;

    ~Data();
};

enum class LayoutType {
    ITEM,
    LEVEL
};

struct Data::LayoutItem {
    Config::Setting::DefineBase* setting{nullptr};

    virtual LayoutType getType() const { return LayoutType::ITEM; }
    virtual ~LayoutItem() {
        if (setting) delete setting;
    }
};

struct Data::LayoutLevel : LayoutItem {
    std::string label{""};
    std::vector<LayoutItem*>* items{nullptr};

    enum class Direction {
        HORIZONTAL,
        VERTICAL
    } direction;

    virtual LayoutType getType() const { return LayoutType::LEVEL; }
    virtual ~LayoutLevel() {
        if (items) {
            for (auto& item : *items) if (item) delete item;
            delete items;
        }
    }
};


struct Data::Button {
    std::string label;
    std::unordered_map<Config::Setting::DefineBase*, std::string> descriptions;
};

struct Data::ButtonState {
    std::string label;
    std::vector<Button*> buttons;

    virtual ~ButtonState() {
        for (auto& button : buttons) if (button) delete button;
    }
};

struct Data::ButtonMap {
    std::unordered_set<ButtonState*> states;
    int8_t numButton;

    virtual ~ButtonMap() {
        for (auto& state : states)
            if (state)
                delete state;
    }
};


}
