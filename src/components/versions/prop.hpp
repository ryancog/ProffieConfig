#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/versions/prop.hpp
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

#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <wx/gdicmn.h>

#include "data/bool.hpp"
#include "data/number.hpp"
#include "data/option.hpp"
#include "logging/logger.hpp"
#include "pconf/types.h"
#include "utils/types.hpp"

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
     */ [[nodiscard]] bool shouldOutputDefine() const;

    /**
     * @return The define str if shouldOutputDefine() returns true, nullopt
     * otherwise
     */
    [[nodiscard]] std::optional<std::string> generateDefineString() const;

    const std::string name;
    const std::string define;
    const std::string description;

    const std::vector<std::string> required;
    const std::vector<std::string> requiredAny;

    const PropDataType dataType;

protected:
    PropDataBase(
        Prop& prop,
        PropDataType type,
        std::string name,
        std::string define,
        std::string description,
        std::vector<std::string> required,
        std::vector<std::string> requiredAny
    ) : pProp{prop}, dataType{type}, name{std::move(name)}, define{std::move(define)},
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

    Prop& pProp;

private:
    friend Prop;

    void enable(bool);
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

  [[nodiscard]] virtual std::string id() const = 0;

  const PropSettingType settingType;
};

struct VERSIONS_EXPORT PropToggle : PropDataBase, PropSettingBase {
    PropToggle(
        Prop& prop,
        std::string name,
        std::string define,
        std::string description,
        std::vector<std::string> required,
        std::vector<std::string> requiredAny,
        std::vector<std::string> disables
    );

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

    [[nodiscard]] std::string id() const override { return define; }

    const std::vector<std::string> disables;

    data::Bool value;
};

struct VERSIONS_EXPORT PropNumeric : PropDataBase, PropSettingBase {
    PropNumeric(
        Prop& prop,
        std::string name,
        std::string define,
        std::string description,
        std::vector<std::string> required,
        std::vector<std::string> requiredAny,
        int32 min,
        int32 max,
        int32 increment,
        std::optional<int32> defaultVal
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
            other.defaultVal,
        } {}

    [[nodiscard]] std::string id() const override { return define; }

    const std::optional<int32> defaultVal;

    data::Integer value;
};

struct VERSIONS_EXPORT PropDecimal : PropDataBase, PropSettingBase {
    PropDecimal(
        Prop& prop,
        std::string name,
        std::string define,
        std::string description,
        std::vector<std::string> required,
        std::vector<std::string> requiredAny,
        float64 min,
        float64 max,
        float64 increment,
        std::optional<float64> defaultVal
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
            other.defaultVal,
        } {}

    [[nodiscard]] std::string id() const override { return define; }

    const std::optional<float64> defaultVal;

    data::Decimal value;
};

struct PropOption;
struct VERSIONS_EXPORT PropSelection : PropDataBase {
    void select();
    [[nodiscard]] bool value() const;
    [[nodiscard]] bool enabled() const;
    [[nodiscard]] bool isDefault() const;

    const std::vector<std::string> disables;

    PropSelection(
        Prop& prop,
        PropOption& option,
        std::string name,
        std::string define,
        std::string description,
        std::vector<std::string> required,
        std::vector<std::string> requiredAny,
        std::vector<std::string> disables
    );

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
        std::string name;
        std::string define;
        std::string description;
        std::vector<std::string> required;
        std::vector<std::string> requiredAny;
        std::vector<std::string> disables;
    };

    PropOption(
        Prop&,
        std::string id,
        std::string name,
        std::string description,
        const std::vector<PropSelectionData>&
    );
    PropOption(const PropOption&, Prop&);

    [[nodiscard]] const std::vector<std::unique_ptr<PropSelection>>& selections() const { return mSelections; }

    [[nodiscard]] std::string id() const override { return idLabel; }

    const std::string idLabel;
    const std::string name;
    const std::string description;
    data::Option selection;

private:
    friend PropSelection;
    std::vector<std::unique_ptr<PropSelection>> mSelections;
};

/**
 * A single button/control mapping to a prop action
 */
struct PropButton {
    const std::string name;

    // <Predicate, Description>
    const std::unordered_map<std::string, std::string> descriptions;
};

/**
 * A collection of PropButton for a certain prop mode/state
 */
struct PropButtonState {
    PropButtonState(std::string stateName, std::vector<PropButton> buttons);

    const std::string stateName;
    const std::vector<PropButton> buttons;
};

using PropButtons = std::vector<PropButtonState>;

struct PropErrorMapping {
    PropErrorMapping(std::string arduinoError, std::string displayError);
    const std::string arduinoError;
    const std::string displayError;
};
using PropErrors = std::vector<PropErrorMapping>;

using PropSettings = std::vector<std::unique_ptr<PropSettingBase>>;
// <ID, Data>
// ID may be a define or (only this for now) an option id
using PropSettingMap = std::unordered_map<std::string, PropSettingBase *>;

// <Define, Data>
using PropDataMap = std::unordered_map<std::string, PropDataBase *>;


struct VERSIONS_EXPORT PropLayout {
    using Children = std::vector<std::variant<PropLayout, PropSettingBase *>>;

    // Only public for Children emplace_back
    PropLayout(wxOrientation axis = wxVERTICAL, std::string label = "", Children children = {}) :
        axis{axis},
        label{std::move(label)},
        children{std::move(children)} {}

    /**
     * Generate a prop layout from given PConf data
     *
     * @return IDs used in PropLayout
     */
    static void generate(
        const PConf::Data&,
        const PropSettingMap&,
        PropLayout& out,
        std::unordered_set<std::string>& usedSettings,
        logging::Logger * = nullptr
    );

    wxOrientation axis;
    std::string label;
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

    static std::unique_ptr<Prop> generate(const PConf::HashedData&, Log::Branch * = nullptr, bool forDefault = false);

    const std::string name;
    const std::string filename;
    const std::string info;

    [[nodiscard]] const PropSettings& settings() const { return mSettings; }
    [[nodiscard]] const PropDataMap& dataMap() const { return mDataMap; }
    [[nodiscard]] const PropSettingMap& settingMap() const { return mSettingMap; }
    [[nodiscard]] const PropLayout& layout() const { return mLayout; }
    [[nodiscard]] PropButtons buttons(uint32 numButtons) const;
    [[nodiscard]] const PropErrors& errors() const { return mErrors; }

    void migrateFrom(const Prop&);

    // TODO: Make this a little more foolproof
    bool isDefault() { return filename.empty(); }

    void recalculateRequires();

private:
    Prop(std::string name, std::string filename, std::string info) : 
        name{std::move(name)}, filename{std::move(filename)}, info{std::move(info)} {}

    /**
     * Rebuild mDataMap and mSettingMap according to the current mSettings.
     *
     * @param pruneList If provided, a list of setting ids in use as understood by
     * the mLayout. As the maps are being generated, settings not in the pruneList
     * will be pruned. (as well as their data, of course)
     */
    void rebuildMaps(
        std::optional<std::unordered_set<std::string>> pruneList = std::nullopt,
        logging::Branch * = nullptr
    );

    PropSettings mSettings;
    PropLayout mLayout;
    PropDataMap mDataMap;
    PropSettingMap mSettingMap;
    std::map<uint32, PropButtons> mButtons;
    PropErrors mErrors;
};

} // namespace Versions
