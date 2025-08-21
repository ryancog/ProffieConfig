#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "log/logger.h"
#include "pconf/types.h"
#include "ui/controls/numeric.h"
#include "ui/controls/radios.h"
#include "ui/controls/toggle.h"
#include "utils/types.h"

#include "versions_export.h"

namespace Versions {

struct Prop;
enum class PropDataType {
    TOGGLE,
    SELECTION,
    NUMERIC,
    DECIMAL,
};

struct VERSIONS_EXPORT PropDataBase {
    PropDataBase(const PropDataBase&) = delete;
    PropDataBase(PropDataBase&&) = delete;
    PropDataBase& operator=(const PropDataBase&) = delete;
    PropDataBase& operator=(PropDataBase&&) = delete;

    /**
     * @return True if enabled and value is "active," false otherwise
     */
    [[nodiscard]] bool isActive() const;

    /**
     * @return True if isActive() and define doesn't have any other blocks to being output.
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

    const PropDataType dataType;

protected:
    PropDataBase(
        Prop& prop,
        PropDataType type,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny
    ) : mProp{prop}, dataType{type}, name{std::move(name)}, define{std::move(define)},
        description{std::move(description)}, required{std::move(required)},
        requiredAny{std::move(requiredAny)} {}

    PropDataBase(const PropDataBase& other, Prop& prop) :
        PropDataBase{
            prop, 
            other.dataType,
            other.name,
            other.description,
            other.define,
            other.required,
            other.requiredAny
        } {}

private:
    friend Prop;
    Prop& mProp;
};

enum PropSettingType {
    TOGGLE,
    NUMERIC,
    DECIMAL,
    OPTION,
};

struct VERSIONS_EXPORT PropSettingBase {
  PropSettingBase(PropSettingType settingType) : settingType(settingType) {}
  virtual ~PropSettingBase() = default;

  [[nodiscard]] virtual string id() const = 0;

  const PropSettingType settingType;
};

struct VERSIONS_EXPORT PropToggle : PropDataBase, PropSettingBase {
    PropToggle(
        Prop& prop,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        vector<string> disables
    ) : PropDataBase{
            prop,
            PropDataType::TOGGLE,
            std::move(name),
            std::move(define),
            std::move(description),
            std::move(required),
            std::move(requiredAny)
        },
        PropSettingBase{
            PropSettingType::TOGGLE,
        },
        disables{std::move(disables)} {}

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

    [[nodiscard]] string id() const override { return define; }

    const vector<string> disables;

    PCUI::ToggleData value;
};

struct VERSIONS_EXPORT PropNumeric : PropDataBase, PropSettingBase {
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
        optional<int32> defaultVal
    ) : PropDataBase{
            prop,
            PropDataType::NUMERIC,
            std::move(name),
            std::move(define),
            std::move(description),
            std::move(required),
            std::move(requiredAny)
        },
        PropSettingBase{
            PropSettingType::NUMERIC,
        }, 
        defaultVal{defaultVal} {
        value.setRange(min, max);
        value.setIncrement(increment);
        if (defaultVal) value = *defaultVal;
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

    [[nodiscard]] string id() const override { return define; }

    const optional<int32> defaultVal;

    PCUI::NumericData value;
};

struct VERSIONS_EXPORT PropDecimal : PropDataBase, PropSettingBase {
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
        optional<float64> defaultVal
    ) : PropDataBase{
            prop,
            PropDataType::DECIMAL,
            std::move(name),
            std::move(define),
            std::move(description),
            std::move(required),
            std::move(requiredAny)
        },
        PropSettingBase{
            PropSettingType::DECIMAL,
        }, 
        defaultVal{defaultVal} {
        value.setRange(min, max);
        value.setIncrement(increment);
        if (defaultVal) value = *defaultVal;
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

    [[nodiscard]] string id() const override { return define; }

    const optional<float64> defaultVal;

    PCUI::DecimalData value;
};

struct PropOption;
struct VERSIONS_EXPORT PropSelection : PropDataBase {
    void select();
    [[nodiscard]] bool value() const;
    [[nodiscard]] bool enabled() const;
    [[nodiscard]] bool isDefault() const;

    const vector<string> disables;

    PropSelection(
        Prop& prop,
        PropOption& option,
        string name,
        string define,
        string description,
        vector<string> required,
        vector<string> requiredAny,
        vector<string> disables
    ) : PropDataBase{
            prop,
            PropDataType::SELECTION,
            std::move(name),
            std::move(define),
            std::move(description),
            std::move(required),
            std::move(requiredAny)
        },
        disables{std::move(disables)}, 
        mOption{option} {}

    PropSelection(const PropSelection& other, Prop& prop, PropOption& option) :
        PropSelection{
            prop,
            option,
            other.name,
            other.define,
            other.description,
            other.required,
            other.requiredAny,
            other.disables
        } {}

    [[nodiscard]] const PropOption& parent() const { return mOption; }
    [[nodiscard]] PropOption& parent() { return mOption; }

private:
    PropOption& mOption;
};

struct VERSIONS_EXPORT PropOption : PropSettingBase {
    PropOption(const PropOption&) = delete;
    PropOption(PropOption&&) = delete;
    PropOption& operator=(const PropOption&) = delete;
    PropOption& operator=(PropOption&&) = delete;

    struct PropSelectionData {
        string name;
        string define;
        string description;
        vector<string> required;
        vector<string> requiredAny;
        vector<string> disables;
    };

    PropOption(
        Prop&,
        string id,
        string name,
        string description,
        const vector<PropSelectionData>&
    );
    PropOption(const PropOption&, Prop&);

    [[nodiscard]] const vector<std::unique_ptr<PropSelection>>& selections() const { return mSelections; }

    [[nodiscard]] string id() const override { return idLabel; }

    const string idLabel;
    const string name;
    const string description;
    PCUI::RadiosData selection;

private:
    friend PropSelection;
    vector<std::unique_ptr<PropSelection>> mSelections;
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

using PropSettings = vector<std::unique_ptr<PropSettingBase>>;
// <ID, Data>
// ID may be a define or (only this for now) an option id
using PropSettingMap = std::unordered_map<string, PropSettingBase *>;

// <Define, Data>
using PropDataMap = std::unordered_map<string, PropDataBase *>;


struct VERSIONS_EXPORT PropLayout {
    using Children = vector<variant<PropLayout, PropSettingBase *>>;

    // Only public for Children emplace_back
    PropLayout(wxOrientation axis = wxVERTICAL, string label = "", const Children& children = {}) :
        axis{axis},
        label{std::move(label)},
        children{children} {}

    /**
     * Generate a prop layout from given PConf data
     *
     * @return IDs used in PropLayout
     */
    static void generate(
        const PConf::Data&,
        const PropSettingMap&,
        PropLayout& out,
        std::unordered_set<string>& usedSettings,
        Log::Logger * = nullptr
    );

    wxOrientation axis;
    string label;
    Children children;

private:
    friend Prop;

    PropLayout(const PropLayout& other, const PropSettingMap& parentMap);

};

struct VERSIONS_EXPORT Prop {
    Prop(const Prop&);
    // Both this and PropOption cannot be moved because
    // there would be hanging references.
    Prop(Prop&&) = delete;
    Prop& operator=(const Prop&) = delete;
    Prop& operator=(Prop&&) = delete;

    static std::shared_ptr<Prop> generate(const PConf::HashedData&, Log::Branch * = nullptr);

    const string name;
    const string filename;
    const string info;

    const PropSettings& settings() const { return mSettings; }
    const PropDataMap& dataMap() const { return mDataMap; }
    const PropSettingMap& settingMap() const { return mSettingMap; }
    const PropLayout& layout() const { return mLayout; }
    PropButtons buttons(uint32 idx) const { return mButtons.at(idx); }
    PropErrors errors() const { return mErrors; }

    void migrateFrom(const Prop&);

private:
    Prop(string name, string filename, string info) : 
        name{std::move(name)}, filename{std::move(filename)}, info{std::move(info)} {}

    /**
     * Rebuild mDataMap and mSettingMap according to the current mSettings.
     *
     * @param pruneList If provided, a list of setting ids in use as understood by
     * the mLayout. As the maps are being generated, settings not in the pruneList
     * will be pruned. (as well as their data, of course)
     */
    void rebuildMaps(
        optional<std::unordered_set<string>> pruneList = nullopt,
        Log::Branch * = nullptr
    );

    PropSettings mSettings;
    PropLayout mLayout;
    PropDataMap mDataMap;
    PropSettingMap mSettingMap;
    array<PropButtons, 4> mButtons;
    PropErrors mErrors;
};

} // namespace Versions
