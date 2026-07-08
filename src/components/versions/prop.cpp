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
#include <optional>
#include <unordered_set>
#include <utility>

#include "data/context.hpp"
#include "data/logic/adapter.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"
#include "pconf/utils.hpp"
#include "ui/controls/checkbox.hpp"
#include "ui/controls/radios.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"
#include "utils/string.hpp"
#include "versions/priv/data.hpp"

using namespace versions::props;

namespace {

std::vector<std::unique_ptr<detail::Data>> parseSettings(
    const pconf::Data&, logging::Branch&
);

/**
 * Parse entries that are somewhat common across setting/data sections,
 * and verify that a label is present for the section (see requireLabel).
 *
 * @param requireLabel If the label is actually required for this section
 * @param requireName If the NAME entry is required for this section
 */
std::optional<std::pair<detail::Data, pconf::HashedData>> parseSettingCommon(
    const pconf::EntryPtr&,
    logging::Logger&,
    bool requireLabel,
    bool requireName
);

Layout parseLayout(
    const pconf::Data&,
    logging::Branch&
);

Buttons parseButtons(const pconf::Data&, logging::Branch&);
Errors parseErrors(const pconf::Data&, logging::Branch&);
MenuSupport parseMenuSupport(const pconf::Data&, logging::Branch&);

} // namespace

Require::Require(std::string&& raw) {
    if (raw[0] == '~') {
        inverted_ = true;
        raw.erase(0, 1);
    }

    if (raw[0] == '@') {
        external_ = true;
        raw.erase(0, 1);
    }

    key_ = std::move(raw);
}

detail::Data::Data(
    std::string name,
    std::string define,
    std::string description,
    std::vector<Require> required,
    std::vector<Require> requireAny
) : name_(std::move(name)),
    define_(std::move(define)),
    description_(std::move(description)),
    required_(std::move(required)),
    requireAny_(std::move(requireAny)) {}

ToggleData::ToggleData(
    Data data, detail::Disables disables, detail::Recommends recommends
) :
    Data(std::move(data)),
    disables_(std::move(disables)),
    recommends_(std::move(recommends)) {}

Toggle::Toggle(Prop& prop, ToggleData data) :
    data::hier::Bool(prop.root()),
    detail::Data(std::move(data)),
    ToggleData(std::move(data)) {}

bool Toggle::isActive() const {
    auto ctxt{data::context(*this)};
    return ctxt.enabled() and ctxt.val();
}

bool Toggle::shouldOutputDefine() const {
    return isActive();
}

std::optional<std::string> Toggle::generateDefineString() const {
    if (not shouldOutputDefine())
        return std::nullopt;

    return define_;
}

IntegerData::IntegerData(
    Data data,
    data::base::Integer::Params params,
    std::optional<int32> defaultVal
) : Data(std::move(data)),
    params_(params),
    defaultVal_(defaultVal) {}

Integer::Integer(Prop& prop, IntegerData data) :
    data::hier::Integer(prop.root()),
    detail::Data(std::move(data)),
    IntegerData(std::move(data)) {
    update(params_);
    if (defaultVal_) set(*defaultVal_);
}

bool Integer::isActive() const {
    return data::context(*this).enabled();
}

bool Integer::shouldOutputDefine() const {
    auto ctxt{data::context(*this)};
    return isActive() and ctxt.val() != defaultVal_;
}

std::optional<std::string> Integer::generateDefineString() const {
    if (not shouldOutputDefine()) return std::nullopt;

    auto ctxt{data::context(*this)};
    return define_ + ' ' + std::to_string(ctxt.val());
}

DecimalData::DecimalData(
    Data data, 
    data::base::Decimal::Params params,
    std::optional<float64> defaultVal
) : Data(std::move(data)),
    params_(params),
    defaultVal_(defaultVal) {}

Decimal::Decimal(Prop& prop, DecimalData data) :
    data::hier::Decimal(prop.root()), 
    detail::Data(std::move(data)),
    DecimalData(std::move(data)) {
    update(params_);
    if (defaultVal_) set(*defaultVal_);
}

bool Decimal::isActive() const {
    return data::context(*this).enabled();
}

bool Decimal::shouldOutputDefine() const {
    auto ctxt{data::context(*this)};
    return isActive() and ctxt.val() != defaultVal_;
}

std::optional<std::string> Decimal::generateDefineString() const {
    if (not shouldOutputDefine())
        return std::nullopt;

    auto ctxt{data::context(*this)};
    return define_ + ' ' + std::to_string(ctxt.val());
}

OptionData::OptionData(Data data, std::vector<SelectionData *> selections) :
    Data(std::move(data)), selections_(std::move(selections)) {}

Option::Option(Prop& prop, OptionData data) :
    data::hier::Exclusive(prop.root()),
    detail::Data(std::move(data)),
    OptionData(std::move(data)) {
    init(selections_.size());
}

std::unique_ptr<data::base::Bool> Option::create(size idx) {
    // This is kind of awkward, but when the Option is first created, it'll
    // have ptrs to some extra data which it was copied from.
    //
    // Those are used to create the children, but then the children, which
    // copy the data themselves, are used to replace the ptrs.
    auto obj{std::make_unique<Selection>(root(), *selections_[idx])};
    selections_[idx] = obj.get();
    return obj;
}

bool Option::isActive() const {
    return data::context(*this).enabled();
}

bool Option::shouldOutputDefine() const {
    if (not isActive())
        return false;

    auto ctxt{data::context(*this)};
    auto& cur{dynamic_cast<Selection&>(ctxt[ctxt.selected()])};
    return cur.shouldOutputDefine();
}

std::optional<std::string> Option::generateDefineString() const {
    if (not shouldOutputDefine())
        return std::nullopt;

    // Since this is what goes into the setting list, make it responsible for
    // forwarding the output call to what is actually selected.
    auto ctxt{data::context(*this)};
    auto& cur{dynamic_cast<Selection&>(ctxt[ctxt.selected()])};
    return cur.generateDefineString();
}

OptionData::SelectionData::SelectionData(
    Data data, detail::Disables disables, detail::Recommends recommends
) : Data(std::move(data)),
    disables_(std::move(disables)),
    recommends_(std::move(recommends)) {}

Option::Selection::Selection(data::hier::Root& root, SelectionData data) :
    data::hier::Bool(root),
    detail::Data(std::move(data)),
    SelectionData(std::move(data)) {}

bool Option::Selection::isActive() const {
    auto ctxt{data::context(*this)};
    return ctxt.enabled() and ctxt.val();
}

bool Option::Selection::shouldOutputDefine() const {
    return isActive() and not define_.empty();
}

std::optional<std::string> Option::Selection::generateDefineString() const {
    if (not shouldOutputDefine())
        return std::nullopt;

    return define_;
}

Prop::Prop(
    data::hier::Root& root,
    std::string installName,
    std::string name,
    std::string filename,
    std::string info,
    std::optional<MenuSupport> menuSupport,
    std::map<uint32, Buttons> buttons,
    Layout layout,
    Errors errors
) : data::hier::Model(root),
    installName_(std::move(installName)),
    name_(std::move(name)),
    filename_(std::move(filename)),
    info_(std::move(info)),
    menuSupport_(std::move(menuSupport)),
    mButtons(std::move(buttons)),
    mLayout(std::move(layout)),
    mErrors(std::move(errors)) {}

std::optional<PropData> PropData::generate(
    const pconf::HashedData& data,
    logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("versions::Prop::generate()", lBranch)};

    const auto nameEntry{data.find("NAME")};
    if (not nameEntry or not nameEntry->value_) {
        logger.error("Missing name.");
        return std::nullopt;
    }
    auto name{*nameEntry->value_};

    const auto filenameEntry{data.find("FILENAME")};
    if (not filenameEntry or not filenameEntry->value_) {
        logger.error("Missing filename.");
        return std::nullopt;
    }
    auto filename{*filenameEntry->value_};

    std::string info;
    const auto infoEntry{data.find("INFO")};
    if (not infoEntry or not infoEntry->value_) {
        logger.info("No info...");
        info = "Prop has no additional info.";
    } else {
        info = *infoEntry->value_;
    }

    std::vector<std::unique_ptr<detail::Data>> settings;

    const auto settingsEntry{data.find("SETTINGS")};
    if (not settingsEntry or not settingsEntry.section()) {
        logger.info("No settings section...");
    } else {
        settings = parseSettings(
            settingsEntry.section()->entries_,
            *logger.bdebug("Parsing SETTINGS...")
        );
    }

    Layout layout;

    const auto layoutEntry{data.find("LAYOUT")};
    if (not layoutEntry or not layoutEntry.section()) {
        logger.info("No layout section...");
    } else {
        layout = parseLayout(
            layoutEntry.section()->entries_,
            *logger.bdebug("Parsing LAYOUT...")
        );
    }

    std::map<uint32, Buttons> buttons;

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
        if (buttons.contains(numButtons)) {
            logger.warn("Skipping duplicate BUTTONS{" + std::to_string(numButtons) + "}...");
            continue;
        }

        buttons[numButtons] = parseButtons(
            buttonEntry.section()->entries_,
            *logger.bdebug("Parsing BUTTONS " + std::to_string(numButtons) + "...")
        );

        // Build a temp lookup set
        std::unordered_set<std::string> settingLookup;
        for (auto& data : settings) {
            settingLookup.insert(data->define_);

            if (auto *ptr{dynamic_cast<OptionData *>(data.get())}) {
                for (auto *selData : ptr->selections_)
                    settingLookup.insert(selData->define_);
            }
        }

        // Then check to warn if there's any non-existent predicates.
        for (auto& state : buttons[numButtons]) {
            for (auto& button : state.buttons_) {
                for (auto& [pred, description] : button.descriptions_) {
                    if (pred.external_)
                        // TODO: No way to warn about this.
                        continue;

                    if (pred.key_.empty())
                        continue;

                    if (not settingLookup.contains(pred.key_))
                        logger.warn("Button " + button.name_ + " has description with non-existent predicate " + pred.key_);
                }
            }
        }
    }
    if (buttonEntries.empty()) {
        logger.info("No buttons entries...");
    }

    Errors errors;

    const auto errorsEntry{data.find("ERRORS")};
    if (not errorsEntry or not errorsEntry.section()) {
        logger.info("No errors section...");
    } else {
        errors = parseErrors(
            errorsEntry.section()->entries_,
            *logger.bdebug("Parsing errors...")
        );
    }

    std::optional<MenuSupport> menuSupport;
    const auto menuSupportEntry{data.find("MENU_SUPPORT")};
    if (menuSupportEntry and menuSupportEntry.section()) {
        menuSupport = parseMenuSupport(
            menuSupportEntry.section()->entries_,
            *logger.bdebug("Parsing menu support...")
        );
    } else {
        logger.info("No menu support section...");
    }

    return std::make_optional<PropData>(
        std::move(name),
        std::move(filename),
        std::move(info),
        std::move(menuSupport),
        std::move(settings),
        std::move(buttons),
        std::move(layout),
        std::move(errors)
    );
}

auto Prop::buttons(uint32 numButtons) const -> const Buttons * {
    if (mButtons.contains(numButtons)) return &mButtons.at(numButtons);
    return nullptr;
}

detail::SettingBase *Prop::find(const std::string& key) const {
    auto iter{mMap.find(key)};
    if (iter == mMap.end()) return nullptr;
    return iter->second;
}

pcui::DescriptorPtr Prop::layout() {
    auto& logger{logging::Context::getGlobal().createLogger("versions::props::Prop::layout()")};

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
        .layout_=mLayout,
        .iter_=mLayout.children_.begin(),
        .stack_=pcui::Stack{
            .base_={.expand_=true},
            .orient_=wxVERTICAL,
        }
    });

    while (not false) {
        if (layers.back().iter_ == layers.back().layout_.children_.end()) {
            auto desc{layers.back().makeDesc()};

            layers.pop_back();
            if (layers.empty()) return desc;

            auto& children{layers.back().children()};

            if (not children.empty()) {
                bool groupSpaced{
                    dynamic_cast<pcui::Group *>(children.back().get()) or
                    dynamic_cast<pcui::Group *>(desc.get())
                };
                pcui::Spacer spacer{
                  .size_=groupSpaced
                    ? pcui::interGroupSpacing()
                    : pcui::interControlSpacing()
                };
                children.push_back(spacer());
            }

            children.push_back(std::move(desc));
            continue;
        }

        const auto& child{*layers.back().iter_};
        ++layers.back().iter_;

        if (const auto *id{std::get_if<std::string>(&child)}) {
            auto iter{mMap.find(*id)};
            if (iter == mMap.end()) {
                logger.warn("Unknown setting in layout: " + *id);
                continue;
            }

            pcui::DescriptorPtr desc;
            bool groupSpaced{false};

            auto *setting{iter->second};
            if (auto *ptr{dynamic_cast<Toggle *>(setting)}) {
                desc = pcui::CheckBox{
                  .win_={
                    .base_={.expand_=true},
                    .tooltip_=ptr->description_
                  },
                  .label_=ptr->name_,
                  .data_=*ptr,
                }();
            } else if (auto *ptr{dynamic_cast<Option *>(setting)}) {
                std::vector<pcui::Radios::Radio> radios;
                radios.reserve(ptr->selections_.size());
                for (auto *sel : ptr->selections_) {
                    radios.push_back(pcui::Radios::Radio{.label_=sel->name_});
                }

                groupSpaced = true;
                desc = pcui::Radios{
                  .win_={
                    .base_={.expand_=true},
                    .tooltip_=ptr->description_
                  },
                  .label_=ptr->name_,
                  .data_=*ptr,
                  .radios_=std::move(radios),
                }();
            } else if (auto *ptr{dynamic_cast<Integer *>(setting)}) {
                desc = pcui::Labeled{
                  .win_={
                    .base_={.expand_=true},
                    .enable_=*ptr | data::logic::IsEnabled{},
                    .tooltip_=ptr->description_
                  },
                  .label_=ptr->name_,
                  .orient_=wxHORIZONTAL,
                  .ctrl_=pcui::Stepper{
                    .win_={.base_={.proportion_=1}},
                    .data_=*ptr,
                  }(),
                }();
            } else if (auto *ptr{dynamic_cast<Decimal *>(setting)}) {
                desc = pcui::Labeled{
                  .win_={
                    .base_={.expand_=true},
                    .enable_=*ptr | data::logic::IsEnabled{},
                    .tooltip_=ptr->description_
                  },
                  .label_=ptr->name_,
                  .orient_=wxHORIZONTAL,
                  .ctrl_=pcui::Stepper{
                    .win_={.base_={.proportion_=1}},
                    .data_=*ptr,
                  }(),
                }();
            } else {
                logger.warn("Setting in layout cannot be positioned: " + *id);
            }

            auto& children{layers.back().children()};
            if (not children.empty()) {
                if (dynamic_cast<pcui::Group *>(children.back().get()))
                    groupSpaced = true;

                pcui::Spacer spacer{
                  .size_=groupSpaced
                    ? pcui::interGroupSpacing()
                    : pcui::interControlSpacing()
                };
                children.push_back(spacer());
            }

            children.push_back(std::move(desc));

            continue;
        }

        const auto& layout{std::get<Layout>(child)};
        Layer layer{
            .layout_ = layout,
            .iter_ = layout.children_.begin(),
        };
        if (not layout.label_.empty()) {
            layer.stack_ = pcui::Group{
                .win_={.base_={.expand_=true}},
                .label_ = layout.label_,
                .orient_ = layout.orient_,
            };
        } else {
            layer.stack_ = pcui::Stack {
                .base_={.expand_=true},
                .orient_ = layout.orient_,
            };
        }

        layers.push_back(std::move(layer));
    }
}

void Prop::migrateFrom(const Prop& from) {
    for (const auto& [id, data] : from.mMap) {
        auto iter{mMap.find(id)};
        if (iter == mMap.end()) continue;

        auto *const setting{iter->second};

        if (auto *self{dynamic_cast<Toggle *>(setting)}) {
            if (auto *other{dynamic_cast<Toggle *>(data)}) {
                self->set(data::context(*other).val());
            } else if (auto *other{dynamic_cast<Option::Selection *>(data)}) {
                self->set(data::context(*other).val());
            }
        } else if (auto *self{dynamic_cast<Option::Selection *>(setting)}) {
            if (auto *other{dynamic_cast<Option::Selection *>(data)}) {
                self->set(data::context(*other).val());
            } else if (auto *other{dynamic_cast<Toggle *>(data)}) {
                self->set(data::context(*other).val());
            }
        } else if (auto *self{dynamic_cast<Integer *>(setting)}) {
            if (auto *other{dynamic_cast<Integer *>(data)}) {
                self->set(data::context(*other).val());
            } else if (auto *other{dynamic_cast<Decimal *>(data)}) {
                self->set(static_cast<int32>(data::context(*other).val()));
            }
        } else if (auto *self{dynamic_cast<Decimal *>(setting)}) {
            if (auto *other{dynamic_cast<Decimal *>(data)}) {
                self->set(data::context(*other).val());
            } else if (auto *other{dynamic_cast<Integer *>(data)}) {
                self->set(static_cast<float64>(data::context(*other).val()));
            }
        }
    }
}

void Prop::markExternalModified(std::string_view str) {
    for (auto *setting : mExtReqMap[str])
        recomputeState(*setting);
}

auto Prop::children() const -> std::vector<const Model *> {
    std::vector<const Model *> ret;

    ret.reserve(mSettings.size());
    for (const auto& setting : mSettings)
        ret.push_back(dynamic_cast<Model *>(setting.get()));

    return ret;
}

void Prop::rebuildLookup(logging::Branch *lBranch) {
    auto& logger{logging::Branch::optCreateLogger("versions::props::Prop::rebuildLookup()", lBranch)};

    mMap.clear();
    mRequiredByMap.clear();
    mDisabledByMap.clear();
    mExtReqMap.clear();

    // First build the id->setting map, and then use it to build others

    for (const auto& setting : mSettings) {
        // Maybe a selection w/ empty id. This'll already be validated, so
        // just check the value and not the validity of having an empty define.
        if (setting->define_.empty()) continue;

        // Handle the nesting of Option
        std::vector<detail::SettingBase *> toIterate{setting.get()};

        if (auto *ptr{dynamic_cast<Option *>(setting.get())}) {
            for (auto *selection : ptr->selections_) {
                auto *asBase{dynamic_cast<detail::SettingBase *>(selection)};
                toIterate.push_back(asBase);
            }
        }

        for (auto *setting : toIterate) {
            if (mMap.contains(setting->define_)) {
                logger.warn("Multiple settings registered under identifier \"" + setting->define_ + '"');
            }

            mMap[setting->define_] = setting;
        }
    }

    for (const auto& [define, setting] : mMap) {
        const auto getDisables{[&] -> const std::vector<std::string> * {
            using Selection = Option::Selection;
            if (const auto *ptr{dynamic_cast<const Toggle *>(setting)})
                return &ptr->disables_;

            if (const auto *ptr{dynamic_cast<const Selection *>(setting)})
                return &ptr->disables_;

            return nullptr;
        }};

        if (const auto *disables{getDisables()}) {
            for (const auto& disable : *disables) {
                auto iter{mMap.find(disable)};
                if (iter == mMap.end()) {
                    logger.warn("Unknown disable \"" + disable + "\" for \"" + setting->define_ + '"');
                    continue;
                }

                mDisabledByMap[iter->second].insert(setting);
            }
        }

        const auto processRequire{[&](const Require& req) {
            if (req.external_) {
                auto res{mExtReqProc(root(), req.key_)};
                if (res == ExternalRequireResult::Not_Found) {
                    logger.warn("Unknown external requires \"" + req.key_ + "\" for \"" + setting->define_ + '"');
                    return;
                } 

                mExtReqMap[req.key_].insert(setting);
                return;
            }

            auto iter{mMap.find(req.key_)};
            if (iter == mMap.end()) {
                logger.warn("Unknown requires \"" + req.key_ + "\" for \"" + setting->define_ + '"');
                return;
            }

            mRequiredByMap[iter->second].insert(setting);
        }};

        for (const auto& req : setting->required_)
            processRequire(req);

        for (const auto& req : setting->requireAny_)
            processRequire(req);
    }
}

void Prop::recomputeState(detail::SettingBase& setting) {
    const auto computeRequire{[this](const Require& req) {
        bool reqVal{};

        if (req.external_) {
            auto res{mExtReqProc(root(), req.key_)};
            // If the external require isn't found, consider it inactive.
            reqVal = (res == ExternalRequireResult::Active);
        } else {
            auto iter{mMap.find(req.key_)};

            if (iter == mMap.end())
                // If the require isn't found, consider it inactive.
                reqVal = false;
            else
                reqVal = iter->second->isActive();
        }

        if (req.inverted_)
            reqVal = not reqVal;

        return reqVal;
    }};

    const auto computeRequired{[&] {
        for (const auto& req : setting.required_)
            if (not computeRequire(req))
                return false;

        return true;
    }};

    const auto computeRequireAny{[&] {
        if (setting.requireAny_.empty())
            return true;

        for (const auto& req : setting.requireAny_)
            if (computeRequire(req))
                return true;

        return false;
    }};

    const auto computeDisabledBy{[&] {
        for (const auto& disabledBy : mDisabledByMap[&setting])
            if (disabledBy->isActive())
                return false;

        return true;
    }};

    dynamic_cast<data::base::Model&>(setting).enable(
        computeRequired() and computeRequireAny() and computeDisabledBy()
    );
}

void Prop::onSet(const data::base::Model& model) {
    const auto& setting{dynamic_cast<const detail::SettingBase&>(model)};

    // First, grab all of the settings whose state could be affected by this
    // change.

    std::set<detail::SettingBase *> affectedSet;

    const auto& disables{[&] -> const auto& {
        if (const auto *ptr{dynamic_cast<const Toggle *>(&model)})
            return ptr->disables_;

        if (const auto *ptr{dynamic_cast<const Option::Selection *>(&model)})
            return ptr->disables_;

        std::unreachable();
    }()};

    for (const auto& disable : disables) {
        auto iter{mMap.find(disable)};

        if (iter == mMap.end())
            // Issues were already logged during map generation.
            continue;

        affectedSet.insert(iter->second);
    }

    for (auto *requiredBy : mRequiredByMap[&setting])
        affectedSet.insert(requiredBy);

    // And then fully compute the state for each.
    for (auto *affected : affectedSet)
        recomputeState(*affected);

    if (mRecProc) {
        const auto& recommends{[&] -> const auto& {
            if (const auto *ptr{dynamic_cast<const Toggle *>(&model)})
                return ptr->recommends_;

            using Selection = Option::Selection;
            if (const auto *ptr{dynamic_cast<const Selection *>(&model)})
                return ptr->recommends_;

            std::unreachable();
        }()};

        for (const auto& [key, val] : recommends) {
            mRecProc(root(), key, val);
        }
    }
}

Available::Available(std::string name, std::vector<utils::Version> versions) :
    name_(std::move(name)), supportedVersions_(std::move(versions)) {}

Versioned::Versioned(
    std::string name,
    std::vector<utils::Version> versions,
    PropData data
) : Available(std::move(name), std::move(versions)),
    data_(std::move(data)) {}

const data::prim::Vector& versions::props::available() {
    return priv::availableProps;
}

const data::prim::Vector& versions::props::list() {
    return priv::props;
}

std::vector<std::unique_ptr<Prop>> versions::props::forVersion(
    const utils::Version& ver,
    data::hier::Root& root,
    Prop::RecommendProcessor recProc,
    Prop::ExternalRequireProcessor extReqProc
) {
    std::vector<std::unique_ptr<Prop>> ret;

    auto ctxt{data::context(priv::props)};
    for (const auto& model : ctxt.children()) {
        const auto& versioned{dynamic_cast<Versioned&>(*model)};
        bool supported{false};

        for (const auto& supVer : versioned.supportedVersions_) {
            if (supVer.compare(ver) != 0) continue;

            supported = true;
            break;
        }

        if (not supported) continue;

        const auto& data{versioned.data_};

        auto& prop{*ret.emplace_back(new Prop(
            root,
            versioned.name_,
            data.name_,
            data.filename_,
            data.info_,
            data.menuSupport_,
            data.buttons_,
            data.layout_,
            data.errors_
        ))};

        prop.mRecProc = recProc;
        prop.mExtReqProc = extReqProc;

        for (const auto& set : data.settings_) {
            data::base::Model *model{nullptr};
            detail::SettingBase *setting{nullptr};

            if (auto *ptr{dynamic_cast<ToggleData *>(set.get())}) {
                auto *toggle{new Toggle(prop, *ptr)};
                setting = toggle;
                model = toggle;
            } else if (auto *ptr{dynamic_cast<OptionData *>(set.get())}) {
                auto *option{new Option(prop, *ptr)};
                setting = option;
                model = option;
            } else if (auto *ptr{dynamic_cast<IntegerData *>(set.get())}) {
                auto *integer{new Integer(prop, *ptr)};
                setting = integer;
                model = integer;
            } else if (auto *ptr{dynamic_cast<DecimalData *>(set.get())}) {
                auto *decimal{new Decimal(prop, *ptr)};
                setting = decimal;
                model = decimal;
            }

            if (setting)
                prop.mSettings.emplace_back(setting);

            if (
                    model and 
                    (not set->requireAny_.empty() or
                     not set->required_.empty())
               ) {
                model->disable();
            }
        }

        prop.rebuildLookup();

        for (const auto& setting : prop.mSettings) {
            static const auto table{[] {
                data::base::Bool::RecvTable table;
                table.onSet_ = data::map<&Prop::onSet>();
                return table;
            }()};

            if (auto *ptr{dynamic_cast<Toggle *>(setting.get())}) {
                prop.respondWith(*ptr, table);
            } else if (auto *ptr{dynamic_cast<Option *>(setting.get())}) {
                for (auto *model : ptr->children())
                    prop.respondWith(*model, table);
            }
        }
    }

    return ret;
}

namespace {

std::vector<std::unique_ptr<detail::Data>> parseSettings(
    const pconf::Data& data, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("PropFile::readSettings()")};

    std::vector<std::unique_ptr<detail::Data>> ret;
    const auto hashedData{pconf::hash(data)};

    const auto parseDisables{[](
        const pconf::HashedData& data
    ) -> std::vector<std::string> {
        const auto disableEntry{data.find("DISABLE")};
        if (not disableEntry or not disableEntry->value_) return {};

        return pconf::valueAsList(disableEntry->value_);
    }};

    const auto parseRec{[](const pconf::HashedData& data) {
        std::vector<std::pair<std::string, std::string>> ret;

        const auto setEntry{data.find("RECOMMENDS")};
        if (auto sect{setEntry.section()}) {
            for (auto& entry : sect->entries_)
                ret.emplace_back(entry->name_, entry->value_.value_or(""));
        }

        return ret;
    }};

    const auto toggleEntries{hashedData.findAll("TOGGLE")};
    for (const auto& toggleEntry : toggleEntries) {
        auto commonData{parseSettingCommon(
            toggleEntry,
            logger,
            true,
            true
        )};
        if (not commonData) continue;
        auto& [settingData, entryMap]{*commonData};

        ret.push_back(std::make_unique<ToggleData>(
            std::move(settingData),
            parseDisables(entryMap),
            parseRec(entryMap)
        ));
    }

    const auto optionEntries{hashedData.findAll("OPTION")};
    for (const auto& optionEntry : optionEntries) {
        auto commonData{parseSettingCommon(
            optionEntry,
            logger,
            true,
            false
        )};
        if (not commonData) continue;
        auto& [settingData, entryMap]{*commonData};

        std::vector<OptionData::SelectionData *> selections;

        const auto selectionEntries{entryMap.findAll("SELECTION")};
        for (const auto& selectionEntry : selectionEntries) {
            auto commonData{parseSettingCommon(
                selectionEntry,
                logger,
                false,
                true 
            )};
            if (not commonData) continue;
            auto& [settingData, entryMap]{*commonData};

            auto selData{std::make_unique<Option::SelectionData>(
                std::move(settingData),
                parseDisables(entryMap),
                parseRec(entryMap)
            )};
            selections.push_back(selData.get());
            ret.push_back(std::move(selData));
        }

        if (selections.size() > 1) {
            ret.push_back(std::make_unique<OptionData>(
                std::move(settingData),
                std::move(selections)
            ));
        } else {
            logger.warn("Option \"" + settingData.name_ + "\" doesn't have enough SELECTIONs, ignoring.");
        }
    }

    const auto numericEntries{hashedData.findAll("NUMERIC")};
    for (const auto& numericEntry : numericEntries) {
        auto commonData{parseSettingCommon(
            numericEntry,
            logger,
            true,
            true 
        )};
        if (not commonData)  continue;
        auto& [settingData, entryMap]{*commonData};

        data::base::Integer::Params params;
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
            ret.emplace_back(std::make_unique<IntegerData>(
                std::move(settingData),
                params,
                defaultVal
            ));
        }
    }

    const auto decimalEntries{hashedData.findAll("DECIMAL")};
    for (const auto& decimalEntry : decimalEntries) {
        auto commonData{parseSettingCommon(
            decimalEntry,
            logger,
            true,
            true 
        )};
        if (not commonData)  continue;
        auto& [settingData, entryMap]{*commonData};

        data::base::Decimal::Params params;
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
            ret.emplace_back(std::make_unique<DecimalData>(
                std::move(settingData),
                params,
                defaultVal
            ));
        }
    }

    return std::move(ret);
}

std::optional<std::pair<detail::Data, pconf::HashedData>> parseSettingCommon(
    const pconf::EntryPtr& entry,
    logging::Logger& logger,
    bool requireLabel,
    bool requireName
) {
    if (not entry->label_ and requireLabel) {
        logger.warn(entry->name_ + " section has no label, ignoring!");
        return std::nullopt;
    }

    if (not entry.section()) {
        logger.warn(entry->name_ + " is not section, ignoring!");
        return std::nullopt;
    }

    std::string name;

    auto data{pconf::hash(entry.section()->entries_)};
    const auto nameEntry{data.find("NAME")};
    if (not nameEntry or not nameEntry->value_) {
        if (requireName) {
            logger.warn(entry->name_ + " section does not have the required \"NAME\" entry, ignoring!");
            return std::nullopt;
        }
    } else {
        name = *nameEntry->value_;
    }
    if (nameEntry) data.erase(nameEntry);

    std::string description;

    const auto descEntry{data.find("DESCRIPTION")};
    if (descEntry and descEntry->value_) {
        description = *descEntry->value_;
        data.erase(descEntry);
    }

    std::vector<Require> required;

    const auto requiredEntry{data.find("REQUIRE")};
    if (requiredEntry and requiredEntry->value_) {
        auto rawVec{pconf::valueAsList(requiredEntry->value_)};
        required.reserve(rawVec.size());

        for (auto& raw : rawVec)
            required.emplace_back(std::move(raw));

        data.erase(requiredEntry);
    }

    std::vector<Require> requireAny;

    const auto requireAnyEntry{data.find("REQUIREANY")};
    if (requireAnyEntry and requireAnyEntry->value_) {
        auto rawVec{pconf::valueAsList(requireAnyEntry->value_)};
        required.reserve(rawVec.size());

        for (auto& raw : rawVec)
            required.emplace_back(std::move(raw));

        data.erase(requireAnyEntry);
    }

    return std::pair{
        detail::Data(
            std::move(name),
            // This is allowed to be nullopt here in some cases.
            entry->label_.value_or(""),
            std::move(description),
            std::move(required),
            std::move(requireAny)
        ),
        data
    };
}

Layout parseLayout(
    const pconf::Data& data, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("versions::props::parseLayout()")};

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

            layers.back().layout_.children_.emplace_back(*entry->label_);
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

            std::vector<std::pair<Require, std::string>> descriptions;
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

                Require newReq(descEntry->label_.value_or(""));
                bool duplicate{false};
                for (const auto& [req, desc] : descriptions) {
                    if (req == newReq) {
                        duplicate = true;
                        break;
                    }
                }

                if (duplicate) {
                    if (not descEntry->label_) logger.warn("Skipping duplicate base button description...");
                    else logger.warn("Skipping duplicate \"" + *descEntry->label_ + "\" button description...");
                    continue;
                }

                descriptions.emplace_back(
                    std::move(newReq),
                    descEntry->value_.value()
                );
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

MenuSupport parseMenuSupport(
    const pconf::Data& data, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Versions::parseMenuSupport()")};

    MenuSupport ret;

    auto hashedData{pconf::hash(data)};

    auto defaultEntry{hashedData.find("DEFAULT")};
    if (defaultEntry and defaultEntry->value_) {
        ret.defaultSpecTemplate_ = *defaultEntry->value_;
    } else {
        logger.debug("No default spec template.");
    }

    return ret;
}

} // namespace

