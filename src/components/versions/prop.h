#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <set>
#include <unordered_map>

#include "log/logger.h"
#include "pconf/types.h"
#include "utils/types.h"

namespace Versions {

struct Prop;
struct PropSetting;
using PropSettingMap = std::unordered_map<string, PropSetting *>;

struct PropSetting {
    /**
     * @return Whether the setting is enabled
     */
    [[nodiscard]] inline bool enabled() const { return mEnabled; }

    /**
     * @return True if enabled and value is "active," false otherwise
     */
    [[nodiscard]] bool shouldOutputDefine() const;

    /**
     * @return The define str if shouldOutputDefine() returns true, nullopt 
     * otherwise
     */
    [[nodiscard]] optional<string> generateDefineString() const;

    const string name;
    const string define;
    const string description;

    const vector<string> required;
    const vector<string> requiredAny;
    // For now this is toggle/selection specific
    // const vector<string> disables;

    enum class Type {
        TOGGLE,
        SELECTION,
        NUMERIC,
        DECIMAL,
    } const type;

protected:
    PropSetting(
        Prop&,
        Type,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny
    );

private:
    friend Prop;
    bool mEnabled;
    Prop& mProp;
};

struct PropToggle : PropSetting {
    PropToggle(
        Prop&,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        vector<string> disables
    );

    const vector<string> disables;
    bool value{false};
};

struct PropNumeric : PropSetting {
    PropNumeric(
        Prop&,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        int32 min,
        int32 max,
        int32 increment,
        int32 defaultVal
    );

    int32 value;
    const int32 min;
    const int32 max;
    const int32 increment;
    const int32 defaultVal;
};

struct PropDecimal : PropSetting {
    PropDecimal(
        Prop&,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        float64 min,
        float64 max,
        float64 increment,
        float64 defaultVal
    );

    float64 value;
    const float64 min;
    const float64 max;
    const float64 increment;
    const float64 defaultVal;
};

struct PropOption;
struct PropSelection : PropSetting {
    void select();
    [[nodiscard]] bool value() const;
    [[nodiscard]] bool isDefault() const;

    const vector<string> disables;
    const bool shouldOutput;

private:
    friend PropOption;
    PropSelection(
        Prop&,
        PropOption&,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        vector<string> disables,
        bool shouldOutput
    );

    PropOption& mOption;
};

struct PropCommonSettingData {
    string name;
    string define;
    string description;
    vector<string> required;
    vector<string> requiredAny;
};

struct PropOption {
    struct PropSelectionData : PropCommonSettingData {
        vector<string> disables;
        bool shouldOutput;
    };

    PropOption(
        Prop&,
        vector<PropSelectionData>
    );

    [[nodiscard]] inline const list<PropSelection>& selections() const { return mSelections; }

private:
    friend Prop;
    list<PropSelection> mSelections;
    uint32 mSelected{0};
};

/**
 * A single button/control mapping to a prop action
 */
struct PropButton {
    const string name;

    // <Predicate, Description>
    const std::unordered_map<string, string> descriptions;
};

/**
 * A collection of PropButtons for a certain prop mode/state
 */
struct PropButtonState {
    PropButtonState(string stateName, vector<PropButton> buttons);

    const string stateName;
    const vector<PropButton> buttons;
};

using PropButtons = vector<PropButtonState>;

struct PropErrorMapping {
    PropErrorMapping(string arduinoError, string displayError);
    const string arduinoError;
    const string displayError;
};
using PropErrors = vector<PropErrorMapping>;

using PropSettingVariant = std::variant<PropToggle, PropNumeric, PropDecimal, PropOption>;

struct PropLayout {
    /**
     * Generate a prop layout from given PConf data
     *
     * @param data PConf input data
     * @param settings Parsed settings from prop pconf
     * @param out PropLayout object to fill
     *
     * @return Settings used in PropLayout
     */
    static std::set<PropSetting *> generate(
        const PConf::Data& data,
        const list<PropSettingVariant>& settings,
        PropLayout& out,
        Log::Logger * = nullptr
    );

    enum class Axis {
        HORIZONTAL,
        VERTICAL,
    } axis{Axis::VERTICAL};

    string label;
    vector<std::variant<PropLayout, PropSetting *>> children;
};

struct Prop {
    static optional<Prop> generate(const PConf::HashedData&);

    const string name;
    const string filename;
    const string info;

private:
    Prop(string name, string filename, string info) : 
        name{name}, filename{filename}, info{info} {}

    PropLayout mLayout;
    array<PropButtons, 4> mButtons;
    list<PropSettingVariant> mSettings;
    PropSettingMap mSettingMap;
    PropErrors mErrors;
};

} // namespace Versions
