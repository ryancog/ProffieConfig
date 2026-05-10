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
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <wx/gdicmn.h>

#include "data/hierarchic/models/bool.hpp"
#include "data/hierarchic/models/exclusive.hpp"
#include "data/hierarchic/models/number.hpp"
#include "log/branch.hpp"
#include "pconf/types.hpp"
#include "ui/types.hpp"
#include "utils/types.hpp"
#include "utils/version.hpp"

#include "versions_export.h"

namespace versions::props {

struct Prop;
struct Context;

namespace detail {

struct VERSIONS_EXPORT Data {
    Data(
        std::string,
        std::string,
        std::string,
        std::vector<std::string>,
        std::vector<std::string>
    );
    Data(Data&&) = default;
    Data(const Data&) = default;
    virtual ~Data() = default;

    const std::string name_;
    const std::string define_;
    const std::string description_;

    const std::vector<std::string> required_;
    const std::vector<std::string> requireAny_;
};

// The consistency of these state getters generally relies on the underlying
// models being locked by hierarchy.
struct VERSIONS_EXPORT SettingBase : virtual detail::Data {
    /**
     * @return True if enabled and value is "active," false otherwise
     */
    [[nodiscard]] virtual bool isActive() const = 0;

    /**
     * @return True if isActive() and define doesn't have any other blocks to
     * being output.
     */
    [[nodiscard]] virtual bool shouldOutputDefine() const = 0;

    /**
     * @return The define str if shouldOutputDefine() returns true, nullopt
     * otherwise
     */
    [[nodiscard]] virtual std::optional<std::string>
        generateDefineString() const = 0;
};

} // namespace detail

struct VERSIONS_EXPORT ToggleData : virtual detail::Data {
    ToggleData(Data, std::vector<std::string>);

    const std::vector<std::string> disables_;
};

struct VERSIONS_EXPORT Toggle : detail::SettingBase,
                                ToggleData,
                                data::hier::Bool {
    Toggle(Prop&, ToggleData);
    bool isActive() const override;
    bool shouldOutputDefine() const override;
    std::optional<std::string> generateDefineString() const override;
};

struct VERSIONS_EXPORT IntegerData : virtual detail::Data {
    IntegerData(Data, data::base::Integer::Params, std::optional<int32>);

    const data::base::Integer::Params params_;
    const std::optional<int32> defaultVal_;
};

struct VERSIONS_EXPORT Integer : detail::SettingBase,
                                 IntegerData,
                                 data::hier::Integer {
    Integer(Prop&, IntegerData);
    bool isActive() const override;
    bool shouldOutputDefine() const override;
    std::optional<std::string> generateDefineString() const override;
};

struct VERSIONS_EXPORT DecimalData : virtual detail::Data {
    DecimalData(Data, data::base::Decimal::Params, std::optional<float64>);

    const data::base::Decimal::Params params_;
    const std::optional<float64> defaultVal_;
};

struct VERSIONS_EXPORT Decimal : detail::SettingBase,
                                 DecimalData,
                                 data::hier::Decimal {
    Decimal(Prop& prop, DecimalData);
    bool isActive() const override;
    bool shouldOutputDefine() const override;
    std::optional<std::string> generateDefineString() const override;
};


struct VERSIONS_EXPORT OptionData : virtual detail::Data {
    struct SelectionData;

    OptionData(Data, std::vector<SelectionData *>);

    std::vector<SelectionData *> selections_;
};

struct VERSIONS_EXPORT Option : detail::SettingBase, 
                                OptionData,
                                data::hier::Exclusive {
    struct Selection;

    Option(Prop&, OptionData);
    std::unique_ptr<data::base::Bool> create(size) override;
    bool isActive() const override;
    bool shouldOutputDefine() const override;
    std::optional<std::string> generateDefineString() const override;
};

struct VERSIONS_EXPORT OptionData::SelectionData : virtual detail::Data {
    SelectionData(Data, std::vector<std::string>);

    const std::vector<std::string> disables_;
};

struct VERSIONS_EXPORT Option::Selection : detail::SettingBase, 
                                           SelectionData,
                                           data::hier::Bool {
    Selection(data::hier::Root&, SelectionData);
    bool isActive() const override;
    bool shouldOutputDefine() const override;
    std::optional<std::string> generateDefineString() const override;
};

/**
 * A single button/control mapping to a prop action
 */
struct VERSIONS_EXPORT Button {
    const std::string name_;

    // <Predicate, Description>
    const std::unordered_map<std::string, std::string> descriptions_;
};

/**
 * A collection of Button for a certain prop mode/state
 */
struct VERSIONS_EXPORT ButtonState {
    const std::string stateName_;
    const std::vector<Button> buttons_;
};

using Buttons = std::vector<ButtonState>;

struct VERSIONS_EXPORT ErrorMapping {
    const std::string arduinoError_;
    const std::string displayError_;
};

using Errors = std::vector<ErrorMapping>;

struct VERSIONS_EXPORT Layout {
    wxOrientation orient_;
    wxString label_;
    std::vector<std::variant<std::string, Layout>> children_;
};

struct VERSIONS_EXPORT PropData {
    static std::optional<PropData> generate(
        const pconf::HashedData& data,
        logging::Branch *lBranch
    );

    std::string name_;
    std::string filename_;
    std::string info_;

    std::vector<std::unique_ptr<detail::Data>> settings_;
    std::map<uint32, Buttons> buttons_;
    Layout layout_;
    Errors errors_;
};

struct VERSIONS_EXPORT Prop : data::hier::Model, data::Receiver {
    [[nodiscard]] std::span<const std::unique_ptr<detail::SettingBase>>
        settings() const { return mSettings; }
    [[nodiscard]] Buttons buttons(uint32 numButtons) const;
    [[nodiscard]] const Errors& errors() const { return mErrors; }

    [[nodiscard]] detail::SettingBase *find(const std::string&) const;

    [[nodiscard]] pcui::DescriptorPtr layout();

    void migrateFrom(const Prop&);

    const std::string name_;
    const std::string filename_;
    const std::string info_;

private:
    friend versions::props::Context;

    Prop(
        data::hier::Root&,
        std::string name,
        std::string filename,
        std::string info,
        std::map<uint32, Buttons> buttons,
        Layout layout,
        Errors errors
    );

    void rebuildLookup(logging::Branch * = nullptr);
    void onSet(const data::base::Model&);

    std::vector<std::unique_ptr<detail::SettingBase>> mSettings;
    const std::map<uint32, Buttons> mButtons;
    const Layout mLayout;
    const Errors mErrors;

    // Maps to accelerate setting lookup. 
    //
    // Mapping of all settings' define/IDs (if they're named) to the data.
    std::unordered_map<std::string, detail::SettingBase *> mMap;

    using RelationMap = std::map<
        const detail::SettingBase *, std::set<detail::SettingBase *>
    >;
    // <Required, Required By>
    //
    // E.g. If `OPTION1` `REQUIRES` `OPTION2`, then the mapping is:
    // mReqMap[`OPTION2`] = {`OPTION1`}
    RelationMap mReqMap;
    // <Disabled, Disabled By>
    //
    // E.g. If `OPTION1` `DISABLE`s `OPTION2`, then the mapping is:
    // mReqMap[`OPTION2`] = {`OPTION1`}
    RelationMap mDisMap;
};

struct VERSIONS_EXPORT Versioned {
    const std::string name_;
    const std::vector<utils::Version> supportedVersions_;

    const PropData data_;
};

struct VERSIONS_EXPORT Available {
    const std::string name_;
    const std::vector<utils::Version> supportedVersions_;
};

struct VERSIONS_EXPORT Context {
    Context();
    ~Context();

    const std::vector<Available>& available() LIFETIMEBOUND;

    const std::vector<std::unique_ptr<Versioned>>& list() LIFETIMEBOUND;

    /**
     * Build a set of props for version
     */
    std::vector<std::unique_ptr<Prop>> forVersion(
        const utils::Version&, data::hier::Root&
    );
};

} // namespace versions::props

