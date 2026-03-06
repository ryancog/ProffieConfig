#include "prop.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/versions/prop.cpp
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

#include <memory>
#include <mutex>
#include <set>

#include "data/bool.hpp"
#include "data/helpers/exclusive.hpp"
#include "data/number.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"
#include "pconf/utils.hpp"
#include "ui/controls/checkbox.hpp"
#include "ui/controls/radios.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/detail/descriptor.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/stack.hpp"
#include "utils/string.hpp"
#include "versions/priv/data.hpp"

using namespace versions::props;

namespace {

Settings parseSettings(
    const pconf::Data&, Prop&, logging::Branch&
);

struct CommonData {
    std::string name_;
    std::string description_;
    std::vector<uint64> required_;
    std::vector<uint64> requiredAny_;
};

/**
 * Parse entries that are somewhat common across setting/data sections,
 * and verify that a label is present for the section (see requireLabel).
 *
 * @param requireLabel If the label is actually required for this section
 * @param requireName If the NAME entry is required for this section
 */
std::optional<std::pair<CommonData, pconf::HashedData>> parseSettingCommon(
    const pconf::EntryPtr&,
    logging::Logger&,
    bool requireLabel,
    bool requireName
);

Layout parseLayout(
    const pconf::Data&, const SettingMap&, logging::Branch&
);

Buttons parseButtons(const pconf::Data&, logging::Branch&);
Errors parseErrors(const pconf::Data&, logging::Branch&);

} // namespace

detail::SettingBase::~SettingBase() = default;

data::Model& detail::SettingBase::model() {
    if (auto *ptr{dynamic_cast<Toggle *>(this)}) {
        return *ptr;
    }
    if (auto *ptr{dynamic_cast<Option *>(this)}) {
        return *ptr;
    }
    if (auto *ptr{dynamic_cast<Option::Selection *>(this)}) {
        return *ptr;
    }
    if (auto *ptr{dynamic_cast<Integer *>(this)}) {
        return *ptr;
    }
    if (auto *ptr{dynamic_cast<Decimal *>(this)}) {
        return *ptr;
    }

    assert(0);
    __builtin_unreachable();
}

bool detail::SettingBase::isActive() {
    if (auto *ptr{dynamic_cast<Toggle *>(this)}) {
        data::Bool::Context ctxt{*ptr};
        if (not ctxt.val()) return false;
    } else if (auto *ptr{dynamic_cast<Option::Selection *>(this)}) {
        data::Bool::Context ctxt{*ptr};
        if (not ctxt.val()) return false;
    }

    data::Model::Context ctxt{model()};
    return ctxt.enabled();
}

bool detail::SettingBase::shouldOutputDefine() {
    if (not isActive()) return false;

    if (auto *ptr{dynamic_cast<Option::Selection *>(this)}) {
        return not ptr->define_.empty();
    }
    if (auto *ptr{dynamic_cast<Integer *>(this)}) {
        return ptr->defaultVal_ != data::Integer::Context{*ptr}.val();
    }
    if (auto *ptr{dynamic_cast<Decimal *>(this)}) {
        return ptr->defaultVal_ != data::Decimal::Context{*ptr}.val();
    }

    return true;
}

std::optional<std::string> detail::SettingBase::generateDefineString() {
    if (not shouldOutputDefine()) return std::nullopt;

    if (auto *ptr{dynamic_cast<Toggle *>(this)}) {
        return define_;
    }
    if (auto *ptr{dynamic_cast<Option::Selection *>(this)}) {
        return define_;
    }
    if (auto *ptr{dynamic_cast<Integer *>(this)}) {
        data::Integer::Context ctxt{*ptr};
        return define_ + ' ' + std::to_string(ctxt.val());
    }
    if (auto *ptr{dynamic_cast<Decimal *>(this)}) {
        data::Decimal::Context ctxt{*ptr};
        return define_ + ' ' + std::to_string(ctxt.val());
    }

    return std::nullopt;
}

void detail::SettingBase::enable(bool en) {
    data::Model::Context{model()}.enable(en);
}

Toggle::Toggle(
    Prop& prop,
    std::string name,
    std::string define,
    std::string description,
    std::vector<uint64> required,
    std::vector<uint64> requiredAny,
    std::vector<uint64> disables
) : detail::SettingBase(
        std::move(name),
        std::move(define),
        std::move(description),
        std::move(required),
        std::move(requiredAny)
    ),
    data::Bool(&prop),
    disables_{std::move(disables)} {
    responder().onSet_ = [](const data::Bool::Context& ctxt) {
        ctxt.model().parent<Prop>()->recalculateRequires();
    };
}

Toggle::Toggle(const Toggle& other, Prop& prop) :
    detail::SettingBase(other),
    data::Bool(other, &prop),
    disables_{other.disables_} {}

Integer::Integer(
    Prop& prop,
    std::string name,
    std::string define,
    std::string description,
    std::vector<uint64> required,
    std::vector<uint64> requiredAny,
    data::Integer::Params params,
    std::optional<int32> defaultVal
) : detail::SettingBase(
        std::move(name),
        std::move(define),
        std::move(description),
        std::move(required),
        std::move(requiredAny)
    ),
    data::Integer(&prop),
    defaultVal_{defaultVal} {

    data::Integer::Context ctxt{*this};
    ctxt.update(params);

    if (defaultVal) ctxt.set(*defaultVal);
}

Integer::Integer(const Integer& other, Prop& prop) :
    detail::SettingBase(other),
    data::Integer(other, &prop),
    defaultVal_{other.defaultVal_} {}

Decimal::Decimal(
    Prop& prop,
    std::string name,
    std::string define,
    std::string description,
    std::vector<uint64> required,
    std::vector<uint64> requiredAny,
    data::Decimal::Params params,
    std::optional<float64> defaultVal
) : detail::SettingBase(
        std::move(name),
        std::move(define),
        std::move(description),
        std::move(required),
        std::move(requiredAny)
    ),
    data::Decimal(&prop),
    defaultVal_{defaultVal} {

    data::Decimal::Context ctxt{*this};
    ctxt.update(params);

    if (defaultVal) ctxt.set(*defaultVal);
}

Decimal::Decimal(const Decimal& other, Prop& prop) :
    detail::SettingBase(other),
    data::Decimal(other, &prop),
    defaultVal_{other.defaultVal_} {}

Option::Option(
    Prop& prop,
    std::string id,
    std::string name,
    std::string description,
    std::vector<std::unique_ptr<Selection>>& selections
) : detail::SettingBase(
        std::move(name),
        std::move(id),
        std::move(description),
        {},
        {}
    ),
    data::Exclusive(
        [&selections]() {
            std::vector<std::unique_ptr<data::Bool>> ret;
            ret.reserve(selections.size());
            for (auto& sel : selections) ret.push_back(std::move(sel));
            return ret;
        }(),
        &prop
    ) {}

Option::Option(const Option& other, Prop& prop) :
    detail::SettingBase(other),
    data::Exclusive(other, &prop) {}

Option::Selection::Selection(
    Prop& prop,
    std::string name,
    std::string define,
    std::string description,
    std::vector<uint64> required,
    std::vector<uint64> requiredAny,
    std::vector<uint64> disables
) : detail::SettingBase(
        std::move(name),
        std::move(define),
        std::move(description),
        std::move(required),
        std::move(requiredAny)
    ),
    data::Bool(&prop),
    disables_{std::move(disables)} {
    responder().onSet_ = [](const data::Bool::Context& ctxt) {
        ctxt.model().parent<Prop>()->recalculateRequires();
    };
}

Option::Selection::Selection(const Selection& other, Prop& prop) :
    detail::SettingBase(other),
    data::Bool(other, &prop),
    disables_{other.disables_} {}


std::string Option::Selection::idString(const std::string& optId) {
    return optId + "::" + define_;
}

ButtonState::ButtonState(std::string stateName, std::vector<Button> buttons) : 
    stateName_{std::move(stateName)}, buttons_{std::move(buttons)} {}

ErrorMapping::ErrorMapping(
    std::string arduinoError,
    std::string displayError
) : arduinoError_{std::move(arduinoError)},
    displayError_{std::move(displayError)} {}

Prop::Prop(const Prop& other, data::Node *parent) :
    data::Node(parent),
    name_{other.name_},
    filename_{other.filename_},
    info_{other.info_},
    mLayout{other.mLayout},
    mButtons{other.mButtons},
    mErrors{other.mErrors} {

    mSettings.reserve(other.mSettings.size());
    for (const auto& setting : other.mSettings) {
        if (auto *ptr{dynamic_cast<Toggle *>(setting.get())}) {
            mSettings.push_back(std::make_unique<Toggle>(*ptr, *this));
        } else if (auto *ptr{dynamic_cast<Option *>(setting.get())}) {
            mSettings.push_back(std::make_unique<Option>(*ptr, *this));
        } else if (auto *ptr{dynamic_cast<Integer *>(setting.get())}) {
            mSettings.push_back(std::make_unique<Integer>(*ptr, *this));
        } else if (auto *ptr{dynamic_cast<Decimal *>(setting.get())}) {
            mSettings.push_back(std::make_unique<Decimal>(*ptr, *this));
        }
    }

    rebuildLookup();
    recalculateRequires();
}

auto Prop::buttons(uint32 numButtons) const -> Buttons {
    if (mButtons.contains(numButtons)) return mButtons.at(numButtons);
    return {};
}

std::unique_ptr<pcui::detail::Descriptor> Prop::layout() {
    struct Layer {
        const Layout& layout_;
        decltype(Layout::children_)::const_iterator iter_;
        std::variant<pcui::Stack, pcui::Group> stack_;

        auto& children() {
            if (auto *ptr{std::get_if<pcui::Stack>(&stack_)}) {
                return ptr->children_;
            }
            return std::get<pcui::Group>(stack_).children_;
        }

        auto makeDesc() {
            if (auto *ptr{std::get_if<pcui::Stack>(&stack_)}) {
                return (*ptr)();
            }
            return std::get<pcui::Group>(stack_)();
        }
    };
    std::vector<Layer> layers;
    layers.push_back({
        .layout_ = mLayout,
        .iter_ = mLayout.children_.begin(),
        .stack_ = pcui::Stack {
            .orient_ = wxVERTICAL,
        }
    });

    while (not false) {
        if (layers.back().iter_ == layers.back().layout_.children_.end()) {
            auto desc{layers.back().makeDesc()};

            layers.pop_back();
            if (layers.empty()) return desc;

            layers.back().children().push_back(std::move(desc));
            continue;
        }

        const auto& child{*layers.back().iter_};
        ++layers.back().iter_;

        if (const auto *id{std::get_if<uint64>(&child)}) {
            auto *setting{find(*id)};

            auto& children{layers.back().children()};
            if (auto *ptr{dynamic_cast<Toggle *>(setting)}) {
                children.push_back(pcui::CheckBox{
                    .data_ = *ptr,
                }());
            } else if (auto *ptr{dynamic_cast<Option *>(setting)}) {
                children.push_back(pcui::Radios{
                    .data_ = *ptr,
                }());
            } else if (auto *ptr{dynamic_cast<Integer *>(setting)}) {
                children.push_back(pcui::Stepper{
                    .data_ = *ptr,

                }());
            } else if (auto *ptr{dynamic_cast<Decimal *>(setting)}) {
                children.push_back(pcui::Stepper{
                    .data_ = *ptr,
                }());
            }

            continue;
        }

        const auto& layout{std::get<Layout>(child)};
        Layer layer{
            .layout_ = layout,
            .iter_ = layout.children_.begin(),
        };
        if (not layout.label_.empty()) {
            layer.stack_ = pcui::Group{
                .label_ = layout.label_,
                .orient_ = layout.orient_,
            };
        } else {
            layer.stack_ = pcui::Stack {
                .orient_ = layout.orient_,
            };
        }

        layers.push_back(std::move(layer));
    }

    return nullptr;
}

void Prop::migrateFrom(const Prop& from) {
    for (const auto& [id, data] : from.mSettingMap) {
        auto iter{mSettingMap.find(id)};
        if (iter == mSettingMap.end()) continue;
        auto *const setting{iter->second};

        if (auto *ptr{dynamic_cast<Toggle *>(setting)}) {
            data::Bool::Context toggle{*ptr};
            if (auto *ptr{dynamic_cast<Toggle *>(data)}) {
                toggle.set(data::Bool::Context{*ptr}.val());
            } else if (auto *ptr{dynamic_cast<Option::Selection *>(data)}) {
                toggle.set(data::Bool::Context{*ptr}.val());
            }
        } else if (auto *ptr{dynamic_cast<Option::Selection *>(setting)}) {
            data::Bool::Context toggle{*ptr};
            if (auto *ptr{dynamic_cast<Option::Selection *>(data)}) {
                toggle.set(data::Bool::Context{*ptr}.val());
            } else if (auto *ptr{dynamic_cast<Toggle *>(data)}) {
                toggle.set(data::Bool::Context{*ptr}.val());
            }
        } else if (auto *ptr{dynamic_cast<Integer *>(setting)}) {
            data::Integer::Context num{*ptr};
            if (auto *ptr{dynamic_cast<Integer *>(data)}) {
                num.set(data::Integer::Context{*ptr}.val());
            } else if (auto *ptr{dynamic_cast<Decimal *>(data)}) {
                const auto fromVal{data::Decimal::Context{*ptr}.val()};
                num.set(static_cast<int32>(fromVal));
            }
        } else if (auto *ptr{dynamic_cast<Decimal *>(setting)}) {
            data::Decimal::Context num{*ptr};
            if (auto *ptr{dynamic_cast<Decimal *>(data)}) {
                num.set(data::Decimal::Context{*ptr}.val());
            } else if (auto *ptr{dynamic_cast<Integer *>(data)}) {
                const auto fromVal{data::Integer::Context{*ptr}.val()};
                num.set(static_cast<float64>(fromVal));
            }
        }
    }
}

Prop::Prop(std::string name, std::string filename, std::string info) :
    data::Node(nullptr),
    name_{std::move(name)},
    filename_{std::move(filename)},
    info_{std::move(info)} {}

void Prop::recalculateRequires() {
    // Anything newly turned-on that results in disables?
    std::set<uint64> disabled;

    const auto onEnum{[&disabled](
        data::Model& model, uint64, const std::string&
    ) {
        if (auto *ptr = dynamic_cast<Toggle *>(&model)) {
            if (not ptr->isActive()) return false;

            for (auto disable : ptr->disables_) {
                disabled.emplace(disable);
            }
        } else if (auto *ptr = dynamic_cast<Option::Selection *>(&model)) {
            if (not ptr->isActive()) return false;

            for (auto disable : ptr->disables_) {
                disabled.emplace(disable);
            }
        }

        return false;
    }};
    enumerate(onEnum);

    // Alright, then disable them
    // If circular dependencies are a problem in the future,
    // just setValue(false) here also to prevent additional logical dependencies,
    // and then just loop back over again to recalculate.
    //
    // Alternatively, disallow them via pre-checking during prop import...
    for (auto disable : disabled) {
        auto iter{mSettingMap.find(disable)};
        if (iter == mSettingMap.end()) continue;

        data::Model::Context{iter->second->model()}.disable();
    }

    // Now handle requirements
    for (const auto& [id, data] : mSettingMap) {
        if (disabled.contains(id)) continue;

        bool satisfied{false};
        if (not data->requiredAny_.empty()) {
            for (auto anyRequire : data->requiredAny_) {
                auto iter{mSettingMap.find(anyRequire)};
                if (iter == mSettingMap.end()) continue;

                if (iter->second->isActive()) {
                    satisfied = true;
                    break;
                }
            }

            if (not satisfied) {
                data->enable(false);
                continue;
            }
        }

        satisfied = true;
        for (auto require : data->required_) {
            auto iter{mSettingMap.find(require)};
            if (iter == mSettingMap.end() or not iter->second->isActive()) {
                satisfied = false;
                break;
            }
        }

        data->enable(satisfied);
    }
}

void Prop::rebuildLookup() {
    mSettingMap.clear();

    for (const auto& setting : mSettings) {
        mSettingMap[strID(setting->define_)] = setting.get();

        if (auto *ptr{dynamic_cast<Option *>(setting.get())}) {
            for (const auto& model : ptr->data()) {
                auto& sel{static_cast<Option::Selection&>(*model)};
                mSettingMap[strID(sel.define_)] = &sel;
            }
        }
    }
}

bool Prop::enumerate(const EnumFunc& func) {
    for (auto& [id, setting] : mSettingMap) {
        if (func(setting->model(), id, setting->define_)) return true;
    }

    return false;
}

data::Model *Prop::find(uint64 id) {
    auto iter{mSettingMap.find(id)};
    if (iter != mSettingMap.end()) return &iter->second->model();
    return nullptr;
}

std::unique_ptr<Prop> Prop::generate(
    const pconf::HashedData& data,
    logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("versions::Prop::generate()", lBranch)};

    const auto nameEntry{data.find("NAME")};
    if (not nameEntry or not nameEntry->value_) {
        logger.error("Missing name.");
        return nullptr;
    }
    const auto name{*nameEntry->value_};

    const auto filenameEntry{data.find("FILENAME")};
    if (not filenameEntry or not filenameEntry->value_) {
        logger.error("Missing filename.");
        return nullptr;
    }
    const auto filename{*filenameEntry->value_};

    std::string info;
    const auto infoEntry{data.find("INFO")};
    if (not infoEntry or not infoEntry->value_) {
        logger.info("No info...");
        info = "Prop has no additional info.";
    } else {
        info = *infoEntry->value_;
    }

    auto prop{std::unique_ptr<Prop>(new Prop(
        name,
        filename,
        info
    ))};

    const auto settingsEntry{data.find("SETTINGS")};
    if (not settingsEntry or not settingsEntry.section()) {
        logger.info("No settings section...");
    } else {
        prop->mSettings = parseSettings(
            settingsEntry.section()->entries_,
            *prop,
            *logger.bdebug("Parsing SETTINGS...")
        );
    }

    prop->rebuildLookup();

    const auto layoutEntry{data.find("LAYOUT")};
    if (not layoutEntry or not layoutEntry.section()) {
        logger.info("No layout section...");
    } else {
        prop->mLayout = parseLayout(
            layoutEntry.section()->entries_,
            prop->mSettingMap,
            *logger.bdebug("Parsing LAYOUT...")
        );
    }

    const auto buttonEntries{data.findAll("BUTTONS")};
    for (const auto& buttonEntry : buttonEntries) {
        if (not buttonEntry.section()) {
            logger.warn("Skipping non-section BUTTONS...");
            continue;
        }

        if (not buttonEntry->labelNum_) {
            logger.warn("Skipping BUTTONS w/o num label...");
            continue;
        }

        const auto numButtons{*buttonEntry->labelNum_};
        if (prop->mButtons.contains(numButtons)) {
            logger.warn("Skipping duplicate BUTTONS{" + std::to_string(numButtons) + "}...");
            continue;
        }

        prop->mButtons[numButtons] = parseButtons(
            buttonEntry.section()->entries_,
            *logger.bdebug("Parsing buttons " + std::to_string(numButtons) + "...")
        );
    }
    if (buttonEntries.empty()) {
        logger.info("No buttons entries...");
    }

    const auto errorsEntry{data.find("ERRORS")};
    if (not errorsEntry or not errorsEntry.section()) {
        logger.info("No errors section...");
    } else {
        prop->mErrors = parseErrors(
            errorsEntry.section()->entries_,
            *logger.bdebug("Parsing errors...")
        );
    }

    prop->recalculateRequires();
    return prop;
}

Versioned::Versioned(
    std::string name,
    std::vector<utils::Version> supportedVersions,
    std::unique_ptr<const Prop> prop
) : name_{std::move(name)},
    supportedVersions_{std::move(supportedVersions)},
    prop_{std::move(prop)} {}

Context::Context() { priv::lock.lock(); }
Context::~Context() { priv::lock.unlock(); }

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
const std::vector<Available>& Context::available() {
    return priv::availableProps;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
const std::vector<std::unique_ptr<Versioned>>& Context::list() {
    return priv::props;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::vector<std::unique_ptr<Prop>> Context::forVersion(
    const utils::Version& ver, data::Node *parent
) {
    std::vector<std::unique_ptr<Prop>> ret;

    for (const auto& prop : priv::props) {
        bool supported{false};

        for (const auto& supVer : prop->supportedVersions_) {
            if (supVer.compare(ver) != 0) continue;

            supported = true;
            break;
        }

        if (not supported) continue;

        ret.emplace_back(new Prop(*prop->prop_, parent));
    }

    return ret;
}

namespace {

Settings parseSettings(
    const pconf::Data& data, Prop& prop, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("PropFile::readSettings()")};

    Settings ret;
    const auto hashedData{pconf::hash(data)};

    const auto parseDisables{[](
        const pconf::HashedData& data
    ) -> std::vector<uint64> {
        const auto disableEntry{data.find("DISABLE")};
        if (not disableEntry or not disableEntry->value_) return {};

        const auto strs{pconf::valueAsList(disableEntry->value_)};
        std::vector<uint64> ret;
        ret.reserve(strs.size());

        for (const auto& str : strs) {
            ret.push_back(data::Model::strID(str));
        }
        return ret;
    }};

    const auto toggleEntries{hashedData.findAll("TOGGLE")};
    for (const auto& toggleEntry : toggleEntries) {
        const auto commonData{parseSettingCommon(
            toggleEntry,
            logger,
            true,
            true
        )};
        if (not commonData) continue;
        const auto& [settingData, entryMap]{*commonData};

        ret.push_back(std::make_unique<Toggle>(
            prop,
            settingData.name_,
            *toggleEntry->label_,
            settingData.description_,
            settingData.required_,
            settingData.requiredAny_,
            parseDisables(entryMap)
        ));
    }

    const auto optionEntries{hashedData.findAll("OPTION")};
    for (const auto& optionEntry : optionEntries) {
        const auto commonData{parseSettingCommon(
            optionEntry,
            logger,
            true,
            false
        )};
        if (not commonData) continue;
        const auto& [settingData, entryMap]{*commonData};

        const auto optionData{pconf::hash(optionEntry.section()->entries_)};
        std::vector<std::unique_ptr<Option::Selection>> selections;

        const auto selectionEntries{optionData.findAll("SELECTION")};
        for (const auto& selectionEntry : selectionEntries) {
            const auto commonData{parseSettingCommon(
                selectionEntry,
                logger,
                false,
                true 
            )};
            if (not commonData)  continue;
            const auto& [settingData, entryMap]{*commonData};

            selections.push_back(std::make_unique<Option::Selection>(
                prop,
                settingData.name_,
                selectionEntry->label_.value_or(""),
                settingData.description_,
                settingData.required_,
                settingData.requiredAny_,
                parseDisables(entryMap)
            ));
        }

        ret.push_back(std::make_unique<Option>(
            prop,
            *optionEntry->label_,
            settingData.name_,
            settingData.description_,
            selections
        ));
    }

    const auto numericEntries{hashedData.findAll("NUMERIC")};
    for (const auto& numericEntry : numericEntries) {
        const auto commonData{parseSettingCommon(
            numericEntry,
            logger,
            true,
            true 
        )};
        if (not commonData)  continue;
        const auto& [settingData, entryMap]{*commonData};

        data::Integer::Params params;
        std::optional<int32> defaultVal;

        const auto minEntry{entryMap.find("MIN")};
        if (minEntry and minEntry->value_) {
            auto val{utils::doStringMath(*minEntry->value_)};
            if (val) params.min_ = static_cast<int32>(*val);
            else logger.warn("Could not parse " + settingData.name_ + " min!");
        }

        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry and maxEntry->value_) {
            auto val{utils::doStringMath(*maxEntry->value_)};
            if (val) params.max_ = static_cast<int32>(*val);
            else logger.warn("Could not parse " + settingData.name_ + " max!");
        }

        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry and incrementEntry->value_) {
            auto val{utils::doStringMath(*incrementEntry->value_)};
            if (val) params.inc_ = static_cast<int32>(*val);
            else logger.warn("Could not parse " + settingData.name_ + " increment!");
        }

        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry and defaultEntry->value_) {
            auto val{utils::doStringMath(*defaultEntry->value_)};
            if (val) defaultVal = static_cast<int32>(*val);
            else logger.warn("Could not parse " + settingData.name_ + " default!");
        }

        if (params.min_ > params.max_) {
            logger.warn("Setting " + settingData.name_ + " has a min value greater than max, this setting will be ignored!");
        } else {
            ret.emplace_back(std::make_unique<Integer>(
                prop,
                settingData.name_,
                *numericEntry->label_,
                settingData.description_,
                settingData.required_,
                settingData.requiredAny_,
                params,
                defaultVal
            ));
        }
    }

    const auto decimalEntries{hashedData.findAll("DECIMAL")};
    for (const auto& decimalEntry : decimalEntries) {
        const auto commonData{parseSettingCommon(
            decimalEntry,
            logger,
            true,
            true 
        )};
        if (not commonData)  continue;
        const auto& [settingData, entryMap]{*commonData};

        data::Decimal::Params params;
        std::optional<float64> defaultVal;

        const auto minEntry{entryMap.find("MIN")};
        if (minEntry and minEntry->value_) {
            auto val{utils::doStringMath(*minEntry->value_)};
            if (val) params.min_ = *val;
            else logger.warn("Could not parse " + settingData.name_ + " min!");
        }

        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry and maxEntry->value_) {
            auto val{utils::doStringMath(*maxEntry->value_)};
            if (val) params.max_ = *val;
            else logger.warn("Could not parse " + settingData.name_ + " max!");
        }

        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry and incrementEntry->value_) {
            auto val{utils::doStringMath(*incrementEntry->value_)};
            if (val) params.inc_ = *val;
            else logger.warn("Could not parse " + settingData.name_ + " increment!");
        }

        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry and defaultEntry->value_) {
            auto val{utils::doStringMath(*defaultEntry->value_)};
            if (val) defaultVal = val;
            else logger.warn("Could not parse " + settingData.name_ + " default!");
        }

        if (params.min_ > params.max_) {
            logger.warn("Setting " + settingData.name_ + " has a min value greater than max, this setting will be ignored!");
        } else {
            ret.emplace_back(std::make_unique<Decimal>(
                prop,
                settingData.name_,
                *decimalEntry->label_,
                settingData.description_,
                settingData.required_,
                settingData.requiredAny_,
                params,
                defaultVal
            ));
        }
    }

    return std::move(ret);
}

std::optional<std::pair<CommonData, pconf::HashedData>> parseSettingCommon(
    const pconf::EntryPtr& entry,
    logging::Logger& logger,
    bool requireLabel,
    bool requireName
) {
    CommonData commonData;

    if (not entry->label_ and requireLabel) {
        logger.warn(entry->name_ + " section has no label, ignoring!");
        return std::nullopt;
    }

    if (not entry.section()) {
        logger.warn(entry->name_ + " is not section, ignoring!");
        return std::nullopt;
    }

    auto data{pconf::hash(entry.section()->entries_)};
    const auto nameEntry{data.find("NAME")};
    if (not nameEntry or not nameEntry->value_) {
        if (requireName) {
            logger.warn(entry->name_ + " section does not have the required \"NAME\" entry, ignoring!");
            return std::nullopt;
        }
    } else {
        commonData.name_ = *nameEntry->value_;
    }
    if (nameEntry) data.erase(nameEntry);

    const auto descEntry{data.find("DESCRIPTION")};
    if (descEntry and descEntry->value_) {
        commonData.description_ = *descEntry->value_;
        data.erase(descEntry);
    }

    const auto requireAnyEntry{data.find("REQUIREANY")};
    if (requireAnyEntry and requireAnyEntry->value_) {
        const auto strList{pconf::valueAsList(requireAnyEntry->value_)};
        commonData.requiredAny_.reserve(strList.size());

        for (const auto& str : strList) {
            commonData.requiredAny_.push_back(data::Model::strID(str));
        }

        data.erase(requireAnyEntry);
    }

    const auto requiredEntry{data.find("REQUIRE")};
    if (requiredEntry and requiredEntry->value_) {
        const auto strList{pconf::valueAsList(requireAnyEntry->value_)};
        commonData.required_.reserve(strList.size());

        for (const auto& str : strList) {
            commonData.required_.push_back(data::Model::strID(str));
        }

        data.erase(requiredEntry);
    }

    return std::pair{commonData, data};
}

Layout parseLayout(
    const pconf::Data& data, const SettingMap& setmap, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("versions::props::parseLayout()")};

    std::set<uint64> usedSettings;

    Layout ret;

    struct Layer {
        Layout& layout_;
        const pconf::Data& data_;
        pconf::Data::const_iterator iter_;
    };
    std::vector<Layer> layers;
    layers.push_back(Layer{
        .layout_=ret,
        .data_=data,
        .iter_=data.begin(),
    });

    while (not false) {
        if (layers.back().iter_ == layers.back().data_.end()) {
            layers.pop_back();

            if (layers.empty()) break;
            continue;
        }

        const auto& entry{*layers.back().iter_};
        ++layers.back().iter_;

        if (entry->name_ == "SETTING") {
            if (not entry->label_) {
                logger.warn("Skipping setting without label...");
                continue;
            }

            const auto id{data::Model::strID(*entry->label_)};
            const auto settingIter{setmap.find(id)};

            if (settingIter == setmap.end()) {
                logger.warn("Skipping unknown setting " + *entry->label_ + "...");
                continue;
            }

            if (usedSettings.contains(id)) {
                logger.warn("Setting " + *entry->label_ + " appeared in layout twice!");
            }

            usedSettings.insert(id);

            layers.back().layout_.children_.emplace_back(id);

            continue;
        } 

        wxOrientation orient{};
        if (entry->name_ == "HORIZONTAL") {
            orient = wxHORIZONTAL;
        } else if (entry->name_ == "VERTICAL") {
            orient = wxVERTICAL;
        } else {
            logger.warn("Skipping " + entry->name_ + " entry in layout...");
            continue;
        }

        if (not entry.section()) {
            logger.warn("Skipping non-section " + entry->name_ + " in layout...");
            continue;
        }

        layers.back().layout_.children_.emplace_back(Layout{
            .orient_ = orient,
            .label_=entry->label_.value_or(""),
        });

        layers.push_back(Layer{
            .layout_ = std::get<Layout>(
                layers.back().layout_.children_.back()
            ),
            .data_ = entry.section()->entries_,
            .iter_ = entry.section()->entries_.begin(),
        });
    }

    return ret;
}

Buttons parseButtons(
    const pconf::Data& data,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Versions::parseButtons()")};
    const auto hashedData{pconf::hash(data)};

    Buttons ret{};

    for (const auto& stateEntry : data) {
        if (stateEntry->name_ != "STATE") {
            logger.warn("Skipping " + stateEntry->name_ + " entry in buttons...");
            continue;
        }

        if (not stateEntry->label_) {
            logger.warn("Skipping buttons state w/o label...");
            continue;
        }

        if (not stateEntry.section()) {
            logger.warn("Skipping non-section buttons state...");
            continue;
        }

        std::vector<Button> buttons;
        const auto& buttonEntries{stateEntry.section()->entries_};
        for (const auto& buttonEntry : buttonEntries) {
            if (buttonEntry->name_ != "BUTTON") {
                logger.warn("Skipping " + stateEntry->name_ + " entry in buttons state...");
                continue;
            }

            if (not buttonEntry->label_) {
                logger.warn("Skipping button w/o label...");
                continue;
            }

            if (not buttonEntry.section()) {
                logger.warn("Skipping non-section button...");
                continue;
            }

            std::unordered_map<uint64, std::string> descriptions;
            const auto& descEntries{buttonEntry.section()->entries_};
            for (const auto& descEntry : descEntries) {
                if (descEntry->name_ != "DESCRIPTION") {
                    logger.warn("Skipping " + stateEntry->name_ + " entry in button...");
                    continue;
                }

                if (not descEntry->value_) {
                    logger.warn("Skipping button description w/o value...");
                    continue;
                }

                const auto [iter, success]{descriptions.try_emplace(
                    data::Model::strID(descEntry->label_.value_or("")),
                    descEntry->value_.value()
                )};

                if (not success) {
                    if (not descEntry->label_) logger.warn("Skipping duplicate base button description...");
                    else logger.warn("Skipping duplicate \"" + *descEntry->label_ + "\" button description...");
                }
            }

            buttons.push_back(Button{
                .name_=buttonEntry->label_.value(),
                .descriptions_=descriptions
            });
        }

        ret.emplace_back(stateEntry->label_.value(), buttons);
    }

    return ret;
}

Errors parseErrors(const pconf::Data& data, logging::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Versions::parseErrors()")};

    Errors ret;
    
    for (const auto& mapEntry : data) {
        if (mapEntry->name_ != "MAP") {
            logger.warn("Skipping " + mapEntry->name_ + " entry in errors...");
            continue;
        }

        if (not mapEntry.section()) {
            logger.warn("Skipping non-section MAP in errors...");
            continue;
        }

        const auto mapData{pconf::hash(mapEntry.section()->entries_)};
        const auto arduinoEntry{mapData.find("ARDUINO")};
        if (not arduinoEntry or not arduinoEntry->value_) {
            logger.warn("Skipping error map w/o arduino error entry...");
            continue;
        }

        const auto displayEntry{mapData.find("DISPLAY")};
        if (not displayEntry or not displayEntry->value_) {
            logger.warn("Skipping error map w/o display error entry...");
            continue;
        }

        ret.emplace_back(*arduinoEntry->value_, *displayEntry->value_);
    }

    return ret;
}

} // namespace

