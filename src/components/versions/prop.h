#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <set>
#include <unordered_map>

#include "log/logger.h"
#include "pconf/types.h"
#include "ui/controls/numeric.h"
#include "ui/controls/radios.h"
#include "ui/controls/toggle.h"
#include "ui/panel.h"
#include "utils/types.h"

namespace Versions {

struct Prop;
struct PropSetting;
using PropSettingMap = std::unordered_map<string, PropSetting *>;

struct PropSetting {
  PropSetting(const PropSetting&) = delete;
  PropSetting(PropSetting&&) = default;
  PropSetting& operator=(const PropSetting&) = delete;
  PropSetting& operator=(PropSetting&&) = delete;

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
        Prop& prop,
        Type type,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny
    ) : mProp{prop}, type{type}, name{name}, define{define}, description{description},
        required{required}, requiredAny{requiredAny} {}

    PropSetting(const PropSetting& other, Prop& prop) :
        PropSetting{
            prop, 
            other.type,
            other.name,
            other.define,
            other.description,
            other.required,
            other.requiredAny
        } {}

private:
    friend Prop;
    Prop& mProp;
};

struct PropToggle : PropSetting {
    PropToggle(const PropToggle&) = delete;
    PropToggle(PropToggle&&) = default;
    PropToggle& operator=(const PropToggle&) = delete;
    PropToggle& operator=(PropToggle&&) = delete;


    PropToggle(
        Prop& prop,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        vector<string> disables
    ) : PropSetting{prop, Type::TOGGLE, name, define, description, required, requiredAny},
        disables{disables} {}

    PropToggle(const PropToggle& other, Prop& prop) :
        PropToggle{
            prop,
            other.name,
            other.define,
            other.description,
            other.required,
            other.requiredAny,
            other.disables,
        } {}

    const vector<string> disables;
    PCUI::ToggleData value;
};

struct PropNumeric : PropSetting {
    PropNumeric(const PropNumeric&) = delete;
    PropNumeric(PropNumeric&&) = default;
    PropNumeric& operator=(const PropNumeric&) = delete;
    PropNumeric& operator=(PropNumeric&&) = delete;

    PropNumeric(
        Prop& prop,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        int32 min,
        int32 max,
        int32 increment,
        int32 defaultVal
    ) : PropSetting{prop, Type::NUMERIC, name, define, description, required, requiredAny} {
        value.setRange(min, max);
        value.setIncrement(increment);
        value = defaultVal;
    }

    PropNumeric(const PropNumeric& other, Prop& prop) :
        PropNumeric{
            prop,
            other.name,
            other.define,
            other.description,
            other.required,
            other.requiredAny,
            other.value.min(),
            other.value.max(),
            other.value.increment(),
            other.value,
        } {}

    PCUI::NumericData value;
};

struct PropDecimal : PropSetting {
    PropDecimal(const PropDecimal&) = delete;
    PropDecimal(PropDecimal&&) = default;
    PropDecimal& operator=(const PropDecimal&) = delete;
    PropDecimal& operator=(PropDecimal&&) = delete;

    PropDecimal(
        Prop& prop,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        float64 min,
        float64 max,
        float64 increment,
        float64 defaultVal
    ) : PropSetting{prop, Type::DECIMAL, name, define, description, required, requiredAny} {
        value.setRange(min, max);
        value.setIncrement(increment);
        value = defaultVal;
    }

    PropDecimal(const PropDecimal& other, Prop& prop) :
        PropDecimal{
            prop,
            other.name,
            other.define,
            other.description,
            other.required,
            other.requiredAny,
            other.value.min(),
            other.value.max(),
            other.value.increment(),
            other.value,
        } {}

    PCUI::DecimalData value;
};

struct PropOption;
struct PropSelection : PropSetting {
    PropSelection(const PropSelection&) = delete;
    PropSelection(PropSelection&&) = default;
    PropSelection& operator=(const PropSelection&) = delete;
    PropSelection& operator=(PropSelection&&) = delete;

    void select();
    [[nodiscard]] bool value() const;
    [[nodiscard]] bool enabled() const;
    [[nodiscard]] bool isDefault() const;

    const vector<string> disables;
    const bool shouldOutput;

private:
    friend PropOption;
    PropSelection(
        Prop& prop,
        PropOption& option,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        vector<string> disables,
        bool shouldOutput
    ) : PropSetting{prop, Type::SELECTION, name, define, description, required, requiredAny},
        disables{disables}, shouldOutput{shouldOutput}, mOption{option} {}

    PropSelection(const PropSelection& other, Prop& prop, PropOption& option) :
        PropSelection{
            prop,
            option,
            other.name,
            other.define,
            other.description,
            other.required,
            other.requiredAny,
            other.disables,
            other.shouldOutput,
        } {}

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
    PropOption(const PropOption&) = delete;
    PropOption(PropOption&&) = delete;
    PropOption& operator=(const PropOption&) = delete;
    PropOption& operator=(PropOption&&) = delete;

    struct PropSelectionData : PropCommonSettingData {
        vector<string> disables;
        bool shouldOutput;
    };

    PropOption(Prop&, vector<PropSelectionData>);
    PropOption(const PropOption&, Prop&);

    [[nodiscard]] inline const list<PropSelection>& selections() const { return mSelections; }

    PCUI::RadiosData selection;

private:
    friend Prop;
    friend PropSelection;
    list<PropSelection> mSelections;
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
 * A collection of PropButton for a certain prop mode/state
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
using PropSettings = list<PropSettingVariant>;

struct PropLayout {
    /**
     * Generate a prop layout from given PConf data
     *
     * @param data PConf input data
     * @param settings Setting map built from Prop
     * @param out PropLayout object to fill
     *
     * @return Settings used in PropLayout
     */
    static std::set<PropSetting *> generate(
        const PConf::Data& data,
        const PropSettingMap& settings,
        PropLayout& out,
        Log::Logger * = nullptr
    );

    enum class Axis {
        HORIZONTAL,
        VERTICAL,
    }; 

    using Children = vector<std::variant<PropLayout, PropSetting *>>;

    PropLayout(const PropLayout&) = delete;
    PropLayout(PropLayout&&) = default;
    PropLayout& operator=(const PropLayout&) = delete;
    PropLayout& operator=(PropLayout&&) = default;

    PropLayout(const PropLayout& other, const PropSettingMap& settingMap);
    PropLayout(Axis axis = Axis::VERTICAL, string label = "", Children children = {}) :
        axis{axis},
        frame{
            label.empty() ? PCUI::PanelData::Type::BASIC : PCUI::PanelData::Type::FRAMED,
            std::move(label)
        },
        children{std::move(children)} {}

    Axis axis;
    PCUI::PanelData frame;
    Children children;
};


struct Prop {
    Prop(const Prop&);
    // Both this and PropOption cannot be moved because
    // there would be hanging references.
    Prop(Prop&&) = delete;
    Prop& operator=(const Prop&) = delete;
    Prop& operator=(Prop&&) = delete;

    static optional<Prop> generate(const PConf::HashedData &);

    const string name;
    const string filename;
    const string info;

    const PropSettings& settings() const { return mSettings; }
    const PropSettingMap& settingMap() const { return mSettingMap; }
    const PropLayout& layout() const { return mLayout; }
    const PropButtons buttons(uint32 idx) const { return mButtons.at(idx); }
    const PropErrors errors() const { return mErrors; }

private:
    Prop(string name, string filename, string info) : 
        name{name}, filename{filename}, info{info} {}

    /**
     * Rebuild mSettingMap according to the current mSettings.
     *
     * @param pruneList If provided, a list of settings in use as understood by
     * the mLayout. As the mSettingMap is being generated, mSettings not in the pruneList
     * will be pruned.
     */
    void rebuildSettingMap(
        optional<std::set<PropSetting *>> pruneList = nullopt,
        Log::Branch * = nullptr
    );

    PropSettings mSettings;
    PropLayout mLayout;
    PropSettingMap mSettingMap;
    array<PropButtons, 4> mButtons;
    PropErrors mErrors;
};

} // namespace Versions
