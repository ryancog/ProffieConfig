#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/controldata.h
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

#include "utils/types.h"

namespace PCUI {

struct ControlData {
    ControlData& operator=(const ControlData&) = delete;
    ControlData& operator=(ControlData&&) = delete;

    /**
     * A callback to alert program-side code that the contained
     * value has been updated, either by program or by the user in UI
     */
    function<void(void)> onUpdate;

    /**
     * Enable/disable the UI control.
     */
    void enable(bool en = true) {
        mEnabled = en;
        pNew = true; 
    }
    void disable() { enable(false); }

    /**
     * Show/hide the UI control.
     */
    void show(bool show = true) {
        mShown = show;
        pNew = true;
    }
    void hide() { show(false); }

    /**
     * If data has been updated since the UI has last seen it
     */
    bool isNew() const { return pNew; }
    /**
     * Mark data new
     */
    void refresh() { pNew = true; }

    [[nodiscard]] bool isEnabled() const { return mEnabled; }
    [[nodiscard]] bool isShown() const { return mShown; }

protected:
    ControlData() = default;
    ControlData(const ControlData&) = delete;
    ControlData(ControlData&&) = default;

    /**
     * Set true whenever the user modifies the data.
     *
     * Windows can use UpdateWindowUI() to force updates for dirty data.
     */
    bool pNew{false};

private:
    /**
     * UI Control states
     */
    bool mEnabled{true};
    bool mShown{true};
};

struct ButtonData : ControlData {
    void operator=(bool val) {
        if (not val) return;
        if (onUpdate) onUpdate();
    }

private:
    friend class Button;
};

struct ToggleData : ControlData {
    operator bool() const { return mValue; }
    void operator=(bool val) {
        mValue = val;
        if (onUpdate) onUpdate();
        pNew = true;
    }

private:
    friend class Toggle;
    friend class CheckBox;
    bool mValue;
};

struct ChoiceData : ControlData {
    operator int32() const { return mValue; }
    operator string() const { 
        if (mValue == -1) return {};
        return mChoices[mValue];
    }

    void operator=(int32 val) {
        mValue = val;
        pNew = true;
    }

    const vector<string>& choices() const { return mChoices; }
    void setChoices(vector<string>&& choices) { 
        mChoices = std::move(choices); 
        pNew = true;
    }

private:
    friend class Choice;
    vector<string> mChoices;
    int32 mValue{-1};
};

struct ComboBoxData : ControlData {
    operator string() const { return mValue; }
    void operator=(string&& val) {
        mValue = std::move(val);
        pNew = true;
    }

    const vector<string>& defaults() const { return mDefaults; }
    void setDefaults(vector<string>&& defaults) {
        mDefaults = std::move(defaults);
        pNew = true;
    }

private:
    friend class ComboBox;
    vector<string> mDefaults;
    string mValue;
};

struct NumericData : ControlData {
    operator int32() const { return mValue; }
    void operator=(int32 val) {
        mValue = val;
        pNew = true;
    }

private:
    friend class Numeric;
    int32 mValue{0};
};

struct DecimalData : ControlData {
    operator float64() const { return mValue; }
    void operator=(float64 val) {
        mValue = val;
        pNew = true;
    }

private:
    friend class Decimal;
    float64 mValue;
};

struct TextData : ControlData {
    operator string() { return mValue; }
    void operator=(string&& val) {
        mValue = std::move(val);
        pNew = true;
    }

private:
    friend class Text;
    string mValue;
};

struct FilePickerData : ControlData {
    operator filepath() { return mValue; }
    void operator=(filepath&& val) {
        mValue = std::move(val);
        pNew = true;
    }

private:
    friend class FilePicker;
    filepath mValue;
};

} // namespace PCUI
