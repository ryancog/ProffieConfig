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
#include <utility>
#include <vector>

#include <wx/gdicmn.h>

#include "data/bool.hpp"
#include "data/hierarchy/node.hpp"
#include "data/number.hpp"
#include "data/helpers/exclusive.hpp"
#include "log/branch.hpp"
#include "pconf/types.hpp"
#include "ui/detail/descriptor.hpp"
#include "utils/types.hpp"

#include "utils/version.hpp"
#include "versions_export.h"

namespace versions::props {

struct Prop;
struct Context;

namespace detail {

struct VERSIONS_EXPORT SettingBase {
    virtual ~SettingBase();

    SettingBase(SettingBase&&) = delete;
    SettingBase& operator=(const SettingBase&) = delete;
    SettingBase& operator=(SettingBase&&) = delete;

    data::Model& model();

    /**
     * @return True if enabled and value is "active," false otherwise
     */
    [[nodiscard]] bool isActive();

    /**
     * @return True if isActive() and define doesn't have any other blocks to being output.
     */
    [[nodiscard]] bool shouldOutputDefine();

    /**
     * @return The define str if shouldOutputDefine() returns true, nullopt
     * otherwise
     */
    [[nodiscard]] std::optional<std::string> generateDefineString();

    const std::string name_;
    const std::string define_;
    const std::string description_;

    const std::vector<uint64> required_;
    const std::vector<uint64> requiredAny_;

protected:
    SettingBase(
        std::string name,
        std::string define,
        std::string description,
        std::vector<uint64> required,
        std::vector<uint64> requiredAny
    ) : name_{std::move(name)}, define_{std::move(define)},
        description_{std::move(description)}, required_{std::move(required)},
        requiredAny_{std::move(requiredAny)} {}

    SettingBase(const SettingBase& other) :
        SettingBase{
            other.name_,
            other.description_,
            other.define_,
            other.required_,
            other.requiredAny_
        } {}

private:
    friend Prop;

    void enable(bool);
};

} // namespace detail

struct VERSIONS_EXPORT Toggle : detail::SettingBase, data::Bool {
    Toggle(
        Prop&,
        std::string name,
        std::string define,
        std::string description,
        std::vector<uint64> required,
        std::vector<uint64> requiredAny,
        std::vector<uint64> disables
    );

    Toggle(const Toggle& other, Prop& prop);

    const std::vector<uint64> disables_;
};

struct VERSIONS_EXPORT Integer : detail::SettingBase, data::Integer {
    Integer(
        Prop& prop,
        std::string name,
        std::string define,
        std::string description,
        std::vector<uint64> required,
        std::vector<uint64> requiredAny,
        data::Integer::Params params,
        std::optional<int32> defaultVal
    );

    Integer(const Integer& other, Prop& prop);

    const std::optional<int32> defaultVal_;
};

struct VERSIONS_EXPORT Decimal : detail::SettingBase, data::Decimal {
    Decimal(
        Prop& prop,
        std::string name,
        std::string define,
        std::string description,
        std::vector<uint64> required,
        std::vector<uint64> requiredAny,
        data::Decimal::Params params,
        std::optional<float64> defaultVal
    );

    Decimal(const Decimal& other, Prop& prop);

    const std::optional<float64> defaultVal_;
};

struct VERSIONS_EXPORT Option : detail::SettingBase, data::Exclusive {
    struct Selection;

    Option(
        Prop&,
        std::string id,
        std::string name,
        std::string description,
        std::vector<std::unique_ptr<Selection>>&
    );

    Option(const Option&, Prop&);
};

struct VERSIONS_EXPORT Option::Selection : detail::SettingBase, data::Bool {
    Selection(
        Prop& prop,
        std::string name,
        std::string define,
        std::string description,
        std::vector<uint64> required,
        std::vector<uint64> requiredAny,
        std::vector<uint64> disables
    );

    Selection(const Selection& other, Prop& prop);

    // Since the label may be left blank, need more to identify
    std::string idString(const std::string& optId);

    const std::vector<uint64> disables_;
};

/**
 * A single button/control mapping to a prop action
 */
struct Button {
    const std::string name_;

    // <Predicate, Description>
    const std::unordered_map<uint64, std::string> descriptions_;
};

/**
 * A collection of PropButton for a certain prop mode/state
 */
struct ButtonState {
    ButtonState(std::string stateName, std::vector<Button> buttons);

    const std::string stateName_;
    const std::vector<Button> buttons_;
};

using Buttons = std::vector<ButtonState>;

struct ErrorMapping {
    ErrorMapping(std::string arduinoError, std::string displayError);
    const std::string arduinoError_;
    const std::string displayError_;
};
using Errors = std::vector<ErrorMapping>;

using Settings = std::vector<std::unique_ptr<detail::SettingBase>>;
using SettingMap = std::unordered_map<uint64, detail::SettingBase *>;

struct Layout {
    wxOrientation orient_;
    wxString label_;
    std::vector<std::variant<uint64, Layout>> children_;
};

struct VERSIONS_EXPORT Prop : data::Node {
    bool enumerate(const EnumFunc&) override;
    Model *find(uint64) override;

    static std::unique_ptr<Prop> generate(
        const pconf::HashedData& data,
        logging::Branch *lBranch
    );

    [[nodiscard]] const Settings& settings() const;
    [[nodiscard]] Buttons buttons(uint32 numButtons) const;
    [[nodiscard]] const Errors& errors() const;

    [[nodiscard]] std::unique_ptr<pcui::detail::Descriptor> layout();

    void migrateFrom(const Prop&);

    void recalculateRequires();

    const std::string name_;
    const std::string filename_;
    const std::string info_;

private:
    friend versions::props::Context;

    Prop(std::string name, std::string filename, std::string info);
    Prop(const Prop& other, data::Node *);

    void rebuildLookup();

    Settings mSettings;
    SettingMap mSettingMap;
    Layout mLayout;

    std::map<uint32, Buttons> mButtons;
    Errors mErrors;
};

struct VERSIONS_EXPORT Versioned {
    Versioned(
        std::string name,
        std::vector<utils::Version> supportedVersions,
        std::unique_ptr<const Prop> prop
    );

    const std::string name_;
    const std::vector<utils::Version> supportedVersions_;

    const std::unique_ptr<const Prop> prop_;
};

struct VERSIONS_EXPORT Available {
    const std::string name_;
    const std::vector<utils::Version> supportedVersions_;
};

struct VERSIONS_EXPORT Context {
    Context();
    ~Context();

    const std::vector<Available>& available() [[clang::lifetimebound]];

    const std::vector<std::unique_ptr<Versioned>>&
        list() [[clang::lifetimebound]];

    /**
     * Build a set of props for version and node.
     */
    std::vector<std::unique_ptr<Prop>> forVersion(
        const utils::Version&, data::Node *
    );
};

} // namespace versions::props

