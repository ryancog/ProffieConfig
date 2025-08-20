#include "prop.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <memory>

#include "log/branch.h"
#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"
#include "utils/string.h"

namespace {

vector<std::unique_ptr<Versions::PropSettingVariant>> parseSettings(const PConf::Data&, Versions::Prop&, Log::Branch&);
optional<std::pair<Versions::PropCommonSettingData, PConf::HashedData>> parseSettingCommon(
    const std::shared_ptr<PConf::Entry>&,
    Log::Logger&
);
Versions::PropButtons parseButtons(const PConf::Data&, Log::Branch&);
Versions::PropErrors parseErrors(const PConf::Data&, Log::Branch&);

} // namespace

Versions::PropOption::PropOption(Prop& prop, const vector<PropSelectionData>& selectionDatas) :
    selection{static_cast<uint32>(selectionDatas.size())} {
    for (const auto& selectionData : selectionDatas) {
        mSelections.push_back(std::make_unique<PropSelection>(
            prop,
            *this,
            selectionData.name,
            selectionData.define,
            selectionData.description,
            selectionData.required,
            selectionData.requiredAny,
            selectionData.disables,
            selectionData.shouldOutput
        ));
    }
}

Versions::PropOption::PropOption(const PropOption& other, Prop& prop) :
    selection{other.selection.numSelections()} {
    for (const auto& sel : other.mSelections) {
        mSelections.push_back(std::make_unique<PropSelection>(*sel, prop, *this));
    }
}

void Versions::PropSelection::select() {
    auto selectionIter{mOption.mSelections.begin()};
    const auto selectionEndIter{mOption.mSelections.end()};
    for (auto idx{0}; selectionIter != selectionEndIter; ++idx, ++selectionIter) { 
        if (&**selectionIter == this) {
            mOption.selection = idx;
            break;
        }
    }
}

bool Versions::PropSelection::value() const {
    // TODO: Make sure this works correctly
    auto selectionIter{mOption.mSelections.begin()};
    for (auto idx{0}; idx < mOption.selection; ++idx, ++selectionIter);
    return &**selectionIter == this;
}

bool Versions::PropSelection::enabled() const {
    uint32 idx{0};
    for (auto iter{mOption.mSelections.begin()}; &**iter != this; ++iter, ++idx);
    return mOption.selection.enabledChoices()[idx];
}

bool Versions::PropSelection::isDefault() const {
    return &*mOption.mSelections.front() == this;
}

Versions::PropLayout::PropLayout(const PropLayout& other, const PropSettingMap& settingMap) :
    axis{other.axis},
    label{other.label} {

    const auto processChildren{[settingMap](
        const auto& self,
        const Children& otherChildren
    ) -> Children {
        Children children;
        for (const auto& child : otherChildren) {
            if (const auto *ptr = std::get_if<PropLayout>(&child)) {
                children.emplace_back(
                    std::in_place_type<PropLayout>,
                    ptr->axis,
                    ptr->label,
                    self(self, ptr->children)
                );
            } else if (const auto *ptr = std::get_if<PropSetting *>(&child)) {
                const auto settingIter{settingMap.find((*ptr)->define)};
                assert(settingIter != settingMap.end());
                children.emplace_back(settingIter->second);
            }
        }
        return children;
    }};

    children = processChildren(processChildren, other.children);
}

std::set<Versions::PropSetting *> Versions::PropLayout::generate(
    const PConf::Data& data,
    const PropSettingMap& settings,
    PropLayout& out,
    Log::Logger *logger
) {
    if (not logger) logger = &Log::Context::getGlobal().createLogger("PropLayout::generate()");
    std::set<PropSetting *> usedSettings;

    for (const auto& entry : data) {
        if (entry->name == "SETTING") {
            if (not entry->label) {
                logger->warn("Skipping setting without label...");
                continue;
            }

            const auto settingIter{settings.find(*entry->label)};
            if (settingIter == settings.end()) {
                logger->warn("Skipping unknown setting " + *entry->label + "...");
                continue;
            }

            usedSettings.emplace(settingIter->second);
            out.children.emplace_back(settingIter->second);
            continue;
        } 

        PropLayout::Axis axis{};
        if (entry->name == "HORIZONTAL") {
            axis = PropLayout::Axis::HORIZONTAL;
        } else if (entry->name == "VERTICAL") {
            axis = PropLayout::Axis::VERTICAL;
        } else {
            logger->warn("Skipping " + entry->name + " entry in layout...");
            continue;
        }

        if (entry->getType() != PConf::Type::SECTION) continue;
        if (entry->label) out.label = entry->label.value();
        auto& child{std::get<PropLayout>(
            out.children.emplace_back(PropLayout{axis})
        )};
        const auto childUsedSettings{generate(
            std::static_pointer_cast<PConf::Section>(entry)->entries,
            settings,
            child,
            logger
        )};
        usedSettings.insert(childUsedSettings.begin(), childUsedSettings.end());
    }

    return usedSettings;
}

Versions::PropButtonState::PropButtonState(string stateName, vector<PropButton> buttons) : 
    stateName{std::move(stateName)}, buttons{std::move(buttons)} {}

Versions::PropErrorMapping::PropErrorMapping(string arduinoError, string displayError) :
    arduinoError{std::move(arduinoError)}, displayError{std::move(displayError)} {}

Versions::Prop::Prop(const Prop& other) : 
    Prop{other.name, other.filename, other.info} {

    for (const auto& setting : other.mSettings) {
        if (auto *ptr = std::get_if<PropToggle>(&*setting)) {
            mSettings.emplace_back(std::make_unique<PropSettingVariant>(
                std::in_place_type<PropToggle>, *ptr, *this
            ));
        } else if (auto *ptr = std::get_if<PropOption>(&*setting)) {
            mSettings.emplace_back(std::make_unique<PropSettingVariant>(
                std::in_place_type<PropOption>, *ptr, *this
            ));
        } else if (auto *ptr = std::get_if<PropNumeric>(&*setting)) {
            mSettings.emplace_back(std::make_unique<PropSettingVariant>(
                std::in_place_type<PropNumeric>, *ptr, *this
            ));
        } else if (auto *ptr = std::get_if<PropDecimal>(&*setting)) {
            mSettings.emplace_back(std::make_unique<PropSettingVariant>(
                std::in_place_type<PropDecimal>, *ptr, *this
            ));
        } else {
            assert(0);
        }
    }

    rebuildSettingMap();

    mLayout = PropLayout(other.mLayout, mSettingMap);

    mButtons.~array();
    new(&mButtons) array(other.mButtons);
    mErrors.~PropErrors();
    new(&mErrors) PropErrors(other.mErrors);
}

void Versions::Prop::migrateFrom(const Prop& from) {
    for (const auto& fromSetting : from.settings()) {
        if (auto *fromPtr = std::get_if<PropToggle>(&*fromSetting)) {
            auto iter{mSettingMap.find(fromPtr->define)};
            if (iter == mSettingMap.end()) continue;
            auto *const setting{iter->second};

            switch (setting->type) {
                case PropSetting::Type::TOGGLE:
                    static_cast<PropToggle *>(setting)->value = static_cast<bool>(fromPtr->value);
                    break;
                case PropSetting::Type::SELECTION:
                    if (fromPtr->value) static_cast<PropSelection *>(setting)->select();
                    break;
                case PropSetting::Type::NUMERIC:
                case PropSetting::Type::DECIMAL:
                    // These don't convert from a toggle
                    break;
            }
        } else if (auto *fromPtr = std::get_if<PropNumeric>(&*fromSetting)) {
            auto iter{mSettingMap.find(fromPtr->define)};
            if (iter == mSettingMap.end()) continue;
            auto *const setting{iter->second};

            switch (setting->type) {
                case PropSetting::Type::TOGGLE:
                case PropSetting::Type::SELECTION:
                    // These don't convert from numeric
                    break;
                case PropSetting::Type::NUMERIC:
                    static_cast<PropNumeric *>(setting)->value = static_cast<int32>(fromPtr->value);
                    break;
                case PropSetting::Type::DECIMAL:
                    static_cast<PropDecimal *>(setting)->value = static_cast<int32>(fromPtr->value);
                    break;
            }
        } else if (auto *fromPtr = std::get_if<PropDecimal>(&*fromSetting)) {
            auto iter{mSettingMap.find(fromPtr->define)};
            if (iter == mSettingMap.end()) continue;
            auto *const setting{iter->second};

            switch (setting->type) {
                case PropSetting::Type::TOGGLE:
                case PropSetting::Type::SELECTION:
                    // These don't convert from a decimal
                    break;
                case PropSetting::Type::NUMERIC:
                    static_cast<PropNumeric *>(setting)->value = static_cast<int32>(static_cast<float64>(fromPtr->value));
                    break;
                case PropSetting::Type::DECIMAL:
                    static_cast<PropDecimal *>(setting)->value = static_cast<float64>(fromPtr->value);
                    break;
            }
        } else if (auto *fromPtr = std::get_if<PropOption>(&*fromSetting)) {
            for (const auto& fromSelection : fromPtr->mSelections) {
                auto iter{mSettingMap.find(fromSelection->define)};
                if (iter == mSettingMap.end()) continue;
                auto *const setting{iter->second};

                switch (setting->type) {
                    case PropSetting::Type::TOGGLE:
                        static_cast<PropToggle *>(setting)->value = fromSelection->value();
                        break;
                    case PropSetting::Type::SELECTION:
                        if (fromSelection->value()) static_cast<PropSelection *>(setting)->select();
                        break;
                    case PropSetting::Type::NUMERIC:
                    case PropSetting::Type::DECIMAL:
                        // These don't convert from a selection
                        break;
                }
            }
        }
    }
}

void Versions::Prop::rebuildSettingMap(optional<std::set<PropSetting *>> pruneList, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Versions::Prop::rebuildSettingMap()", lBranch)};
    mSettingMap.clear();

    for (auto setting{mSettings.begin()}; setting != mSettings.end();) {
        PropSetting *ptr{};
        if (
                (ptr = std::get_if<PropToggle>(&**setting)) or
                (ptr = std::get_if<PropNumeric>(&**setting)) or
                (ptr = std::get_if<PropDecimal>(&**setting))
           ) {
            if (pruneList and not pruneList->contains(ptr)) {
                logger.warn("Removing unused setting \"" + ptr->name + "\"...");
                setting = mSettings.erase(setting);
                continue;
            } 

            mSettingMap.emplace(ptr->define, ptr);
        } else if (auto *ptr = std::get_if<PropOption>(&**setting)) {
            for (auto selIter{ptr->mSelections.begin()}; selIter != ptr->mSelections.end();) {
                if (pruneList and not pruneList->contains(&**selIter)) {
                    logger.warn("Removing unused setting \"" + (*selIter)->name + "\"...");
                    selIter = ptr->mSelections.erase(selIter);
                    continue;
                }
                ++selIter;
            }

            if (ptr->mSelections.empty()) {
                logger.warn("Removing empty option...");
                setting = mSettings.erase(setting);
                continue;
            } 

            for (auto& selection : ptr->mSelections) {
                mSettingMap.emplace(selection->define, &*selection);
            }
        }

        ++setting;
    }
}

std::shared_ptr<Versions::Prop> Versions::Prop::generate(const PConf::HashedData& data, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Versions::Prop::generate()", lBranch)};

    const auto nameIter{data.find("NAME")};
    if (nameIter == data.end() or not nameIter->second->value) {
        logger.error("Missing NAME entry.");
        return nullptr;
    }

    const auto filenameIter{data.find("FILENAME")};
    if (filenameIter == data.end() or not filenameIter->second->value) {
        logger.error("Missing FILENAME entry.");
        return nullptr;
    }

    const auto infoIter{data.find("INFO")};
    if (infoIter == data.end() or not infoIter->second->value) {
        logger.info("Skipping missing INFO entry...");
    } 

    auto prop{std::shared_ptr<Prop>(new Prop(
        nameIter->second->value.value(),
        filenameIter->second->value.value(),
        infoIter->second->value.value_or("Prop has no additional info.")
    ))};

    const auto settingsIter {data.find("SETTINGS")};
    if (settingsIter == data.end() or settingsIter->second->getType() != PConf::Type::SECTION) {
        logger.info("Skipping missing SETTINGS section...");
    } else {
        prop->mSettings = parseSettings(
            std::static_pointer_cast<PConf::Section>(settingsIter->second)->entries,
            *prop,
            *logger.bdebug("Parsing SETTINGS...")
        );
    }

    prop->rebuildSettingMap(nullopt, logger.bdebug("Building setting map..."));

    std::set<PropSetting *> settingsUsed{};
    const auto layoutIter{data.find("LAYOUT")};
    if (layoutIter == data.end() or layoutIter->second->getType() != PConf::Type::SECTION) {
        logger.info("Skipping missing LAYOUT section...");
    } else {
        settingsUsed = PropLayout::generate(
            std::static_pointer_cast<PConf::Section>(layoutIter->second)->entries,
            prop->mSettingMap,
            prop->mLayout,
            &logger.bdebug("Parsing LAYOUT...")->createLogger("PropLayout::generate()")
        );
    }

    prop->rebuildSettingMap(settingsUsed, logger.bdebug("Rebuilding setting map..."));

    const auto buttonsRange{data.equal_range("BUTTONS")};
    if (buttonsRange.first == buttonsRange.second) {
        logger.info("Skipping missing BUTTONS section...");
    } else {
        for (auto it{buttonsRange.first}; it != buttonsRange.second; ++it) {
            const auto& entry{it->second};
            if (entry->getType() != PConf::Type::SECTION) {
                logger.warn("Skipping non-section BUTTONS...");
                continue;
            }

            if (not entry->labelNum) {
                logger.warn("Skipping BUTTONS w/o num label...");
                continue;
            }

            const auto numButtons{entry->labelNum.value()};
            if (numButtons < 0 or numButtons > prop->mButtons.size()) {
                logger.warn( "Skipping BUTTONS with out of range num (" + 
                    std::to_string(numButtons) + ")... ");
                continue;
            }

            if (not prop->mButtons[numButtons].empty()) {
                logger.warn("Skipping duplicate BUTTONS{" + std::to_string(numButtons) + "}...");
                continue;
            }

            prop->mButtons[numButtons] = parseButtons(
                std::static_pointer_cast<PConf::Section>(entry)->entries,
                *logger.bdebug("Parsing buttons " + std::to_string(numButtons) + "...")
            );
        }
    }

    const auto errorsIter{data.find("ERRORS")};
    if (errorsIter == data.end() or errorsIter->second->getType() != PConf::Type::SECTION) {
        logger.info("Skipping missing ERRORS section...");
    } else {
        prop->mErrors = parseErrors(
            std::static_pointer_cast<PConf::Section>(errorsIter->second)->entries,
            *logger.bdebug("Parsing errors...")
        );
    }

    return prop;
}


bool Versions::PropSetting::isActive() const {
    switch (type) {
        case Type::TOGGLE:
            return 
                static_cast<const PropToggle *>(this)->value.isEnabled() and
                static_cast<const PropToggle *>(this)->value;
        case Type::SELECTION:
            return 
                static_cast<const PropSelection *>(this)->enabled() and
                static_cast<const PropSelection *>(this)->value();
        case Type::NUMERIC:
            return static_cast<const PropNumeric *>(this)->value.isEnabled();
        case Type::DECIMAL:
            return static_cast<const PropDecimal *>(this)->value.isEnabled();
    }
    assert(0);
}

bool Versions::PropSetting::shouldOutputDefine() const {
    if (not isActive()) return false;

    if (type == Type::SELECTION) {
        return static_cast<const PropSelection *>(this)->shouldOutput;
    }

    return true;
}

optional<string> Versions::PropSetting::generateDefineString() const {
    if (not shouldOutputDefine()) return nullopt;

    switch (type) {
        case Type::TOGGLE:
            return static_cast<const PropToggle *>(this)->value ? optional{define} : nullopt;
        case Type::SELECTION:
            return static_cast<const PropSelection *>(this)->value() ? optional{define} : nullopt;
        case Type::NUMERIC:
            return define + " " + std::to_string(static_cast<const PropNumeric *>(this)->value);
        case Type::DECIMAL:
            return define + " " + std::to_string(static_cast<const PropDecimal *>(this)->value);
    }

    return {};
}

// bool PropFile::Setting::checkRequiredSatisfied(const std::unordered_map<string, Setting>& settings) const {
//   if (not requiredAny.empty()) {
//     for (const auto& require : requiredAny) {
//       auto key = settings.find(require);
//       if (key == settings.end()) continue;
//       if (!key->second.getOutput().empty()) return true;
//     }
// 
//     return false;
//   }      
// 
//   for (const auto& require : required) {
//       auto key = settings.find(require);
//       if (key == settings.end()) return false;
//       if (key->second.getOutput().empty()) return false;
//   }
// 
//   return true;
// }

namespace {

vector<std::unique_ptr<Versions::PropSettingVariant>> parseSettings(
    const PConf::Data& data,
    Versions::Prop& prop,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("PropFile::readSettings()")};

    vector<std::unique_ptr<Versions::PropSettingVariant>> ret;
    const auto hashedData{PConf::hash(data)};

    const auto parseDisables{[](const PConf::HashedData& data) -> vector<string> {
        const auto disableEntry{data.find("DISABLE")};
        if (disableEntry == data.end()) return {};
        if (not disableEntry->second->value) return {};
        return PConf::valueAsList(disableEntry->second->value);
    }};

    const auto toggleRange{hashedData.equal_range("TOGGLE")};
    for (auto it{toggleRange.first}; it != toggleRange.second; ++it) {
        const auto commonData{parseSettingCommon(it->second, logger)};
        if (not commonData) continue;
        const auto& [settingData, entryMap]{*commonData};

        ret.emplace_back(std::make_unique<Versions::PropSettingVariant>(
            std::in_place_type<Versions::PropToggle>,
            prop,
            settingData.name,
            settingData.define,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            parseDisables(entryMap)
        ));
    }

    const auto optionRange{hashedData.equal_range("OPTION")};
    for (auto it{optionRange.first}; it != optionRange.second; ++it) {
        if (it->second->getType() != PConf::Type::SECTION) {
            logger.warn(it->second->name + " is not section, ignoring!");
            continue;
        }

        vector<Versions::PropOption::PropSelectionData> selectionDatas;

        const auto optionData{PConf::hash(std::static_pointer_cast<PConf::Section>(it->second)->entries)};
        const auto selectionRange{optionData.equal_range("SELECTION")};
        for (auto selectionIt{selectionRange.first}; selectionIt != selectionRange.second; ++selectionIt) {
            const auto commonData{parseSettingCommon(selectionIt->second, logger)};
            if (not commonData)  continue;
            const auto& [settingData, entryMap]{*commonData};

            Versions::PropOption::PropSelectionData selectionData{
                {
                    .name = settingData.name,
                    .define = settingData.define,
                    .description = settingData.description,
                    .required = settingData.required,
                    .requiredAny = settingData.requiredAny,
                },
                parseDisables(entryMap),
                entryMap.contains("NO_OUTPUT"),
            };

            selectionDatas.emplace_back(std::move(selectionData));
        }

        ret.emplace_back(std::make_unique<Versions::PropSettingVariant>(
            std::in_place_type<Versions::PropOption>,
            prop,
            selectionDatas
        ));
    }

    const auto numericRange{hashedData.equal_range("NUMERIC")};
    for (auto it{numericRange.first}; it != numericRange.second; ++it) {
        const auto commonData{parseSettingCommon(it->second, logger)};
        if (not commonData) continue;
        const auto& [settingData, entryMap]{*commonData};

        int32 min{0};
        int32 max{100};
        int32 increment{1};
        int32 defaultVal{0};

        const auto minEntry{entryMap.find("MIN")};
        if (minEntry != entryMap.end() and minEntry->second->value) {
            min = static_cast<int32>(strtol(minEntry->second->value->c_str(), nullptr, 10));
        }
        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry != entryMap.end() and maxEntry->second->value) {
            max = static_cast<int32>(strtol(maxEntry->second->value->c_str(), nullptr, 10));
        }
        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry != entryMap.end() and incrementEntry->second->value) {
            increment = static_cast<int32>(strtol(incrementEntry->second->value->c_str(), nullptr, 10));
        }
        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry != entryMap.end() and defaultEntry->second->value) {
            defaultVal = static_cast<int32>(strtol(defaultEntry->second->value->c_str(), nullptr, 10));
        }

        ret.emplace_back(std::make_unique<Versions::PropSettingVariant>(
            std::in_place_type<Versions::PropNumeric>,
            prop,
            settingData.name,
            settingData.define,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            min,
            max,
            increment,
            defaultVal
        ));
    }

    const auto decimalRange{hashedData.equal_range("DECIMAL")};
    for (auto it{decimalRange.first}; it != decimalRange.second; ++it) {
        const auto commonData{parseSettingCommon(it->second, logger)};
        if (not commonData) continue;
        const auto& [settingData, entryMap]{*commonData};

        float64 min{0};
        float64 max{10};
        float64 increment{0.1};
        float64 defaultVal{0};

        const auto minEntry{entryMap.find("MIN")};
        if (minEntry != entryMap.end() and minEntry->second->value) {
            min = strtod(minEntry->second->value->c_str(), nullptr);
        }
        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry != entryMap.end() and maxEntry->second->value) {
            max = strtod(maxEntry->second->value->c_str(), nullptr);
        }
        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry != entryMap.end() and incrementEntry->second->value) {
            increment = strtod(incrementEntry->second->value->c_str(), nullptr);
        }
        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry != entryMap.end() and defaultEntry->second->value) {
            defaultVal = strtod(defaultEntry->second->value->c_str(), nullptr);
        }

        ret.emplace_back(std::make_unique<Versions::PropSettingVariant>(
            std::in_place_type<Versions::PropDecimal>,
            prop,
            settingData.name,
            settingData.define,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            min,
            max,
            increment,
            defaultVal
        ));
    }

    return std::move(ret);
}

optional<std::pair<Versions::PropCommonSettingData, PConf::HashedData>> parseSettingCommon(
    const std::shared_ptr<PConf::Entry>& entry,
    Log::Logger& logger
) {
    Versions::PropCommonSettingData commonData;

    if (not entry->label) {
        logger.warn(entry->name + " section has no label, ignoring!");
        return nullopt;
    }

    commonData.define = *entry->label;
    Utils::trimWhitespace(commonData.define);
    if (commonData.define.empty()) {
        logger.warn(entry->name + " section has empty define/label, ignoring!");
        return nullopt;
    }

    if (entry->getType() != PConf::Type::SECTION) {
        logger.warn(entry->name + " is not section, ignoring!");
        return nullopt;
    }

    auto data{PConf::hash(std::static_pointer_cast<PConf::Section>(entry)->entries)};

    const auto nameIter{data.find("NAME")};
    if (nameIter == data.end() or not nameIter->second->value) {
        logger.warn(entry->name + " section does not have the required \"NAME\" entry, ignoring!");
        return nullopt;
    }
    commonData.name = *nameIter->second->value;
    data.erase(nameIter);

    const auto descIter{data.find("DESCRIPTION")};
    if (descIter != data.end() and descIter->second->value) {
        commonData.description = *descIter->second->value;
        data.erase(descIter);
    }

    const auto requireAnyIter{data.find("REQUIREANY")};
    if (requireAnyIter != data.end() and requireAnyIter->second->value) {
        commonData.requiredAny = PConf::valueAsList(requireAnyIter->second->value);
        data.erase(requireAnyIter);
    }

    const auto requiredIter{data.find("REQUIRE")};
    if (requiredIter != data.end() and requiredIter->second->value) {
        commonData.required = PConf::valueAsList(requiredIter->second->value);
        data.erase(requiredIter);
    }

    return std::pair{commonData, data};
}

// void PropFile::readLayout(const PConf::Data& data, Log::Branch& lBranch) {
// #   define ITEMBORDER wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 5)
//     auto& logger{lBranch.createLogger("PropFile::readLayout()")};
//     auto createToggle = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
//         setting.control = new wxCheckBox(parent, wxID_ANY, setting.name);
//         static_cast<wxCheckBox*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
//         sizer->Add(static_cast<wxCheckBox*>(setting.control), ITEMBORDER);
//     };
//     auto createNumeric = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
//         auto *entry{new PCUI::Numeric(parent, wxID_ANY, static_cast<int32>(setting.min), static_cast<int32>(setting.max), static_cast<int32>(setting.defaultVal), static_cast<int32>(setting.increment), wxSP_ARROW_KEYS, setting.name)};
//         setting.control = entry;
//         static_cast<PCUI::Numeric*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
//         sizer->Add(entry, ITEMBORDER);
//     };
//     auto createDecimal = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
//         auto *entry{new PCUI::NumericDec(parent, wxID_ANY, setting.min, setting.max, setting.defaultVal, setting.increment, wxSP_ARROW_KEYS, setting.name)};
//         setting.control = entry;
//         static_cast<PCUI::NumericDec*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
//         sizer->Add(entry, ITEMBORDER);
//     };
//     auto createOption = [](Setting& setting, wxWindow* parent, wxSizer* sizer) {
//         setting.control = new wxRadioButton(parent, wxID_ANY, setting.name);
//         static_cast<wxRadioButton*>(setting.control)->SetToolTip(new wxToolTip(setting.description));
//         sizer->Add(static_cast<wxRadioButton*>(setting.control), ITEMBORDER);
//     };
// 
//     vector<wxWindow *> parentStack;
//     vector<wxSizer*> sizerStack;
//     mSizer = new wxBoxSizer(wxVERTICAL);
// 
//     // <Current Iterator, End Iterator>
//     vector<std::pair<decltype(data.begin()), decltype(data.end())>> entryStack;
//     if (not data.empty()) entryStack.emplace_back(data.begin(), data.end());
// 
//     while (true) {
//         if (entryStack.empty()) break;
//         const auto& entry{*entryStack.back().first};
// 
//         if (entry->name == "HORIZONTAL" or entry->name == "VERTICAL") {
//             if (entry->getType() == PConf::Type::SECTION) {
//                 wxSizer *nextSizer{nullptr};
//                 auto *previousSizer{sizerStack.empty() ? mSizer : sizerStack.back()};
//                 if (entry->label) {
//                     auto *sectionSizer{new wxStaticBoxSizer(entry->name == "HORIZONTAL" ? wxHORIZONTAL : wxVERTICAL, parentStack.empty() ? this : parentStack.back(), *entry->label)};
//                     nextSizer = sectionSizer;
//                     parentStack.push_back(sectionSizer->GetStaticBox());
//                     previousSizer->Add(nextSizer, ITEMBORDER.Expand());
//                 } else {
//                     auto *sectionSizer{new wxBoxSizer(entry->name == "HORIZONTAL" ? wxHORIZONTAL : wxVERTICAL)};
//                     nextSizer = sectionSizer;
//                     previousSizer->Add(nextSizer, wxSizerFlags{}.Expand());
//                 }
// 
//                 const auto& entries{std::static_pointer_cast<PConf::Section>(entry)->entries};
//                 if (not entries.empty()) {
//                     // Decrement to past-begin since this will be prematurely incremented
//                     // (instead of the "intended" current stack, due to the new addition)
//                     entryStack.emplace_back(--entries.begin(), entries.end());
//                     sizerStack.push_back(nextSizer);
//                 }
//             }
//         } else if (entry->name == "SETTING") {
//             if (entry->getType() == PConf::Type::ENTRY and entry->label) {
//                 auto define{mSettings->find(entry->label.value_or(""))};
//                 if (define == mSettings->end()) {
//                     logger.warn("Setting \"" + *entry->label + "\" not found in settings, skipping...");
//                 } else {
//                     auto *parent{parentStack.empty() ? this : parentStack.back()};
//                     auto *sectionSizer{sizerStack.empty() ? mSizer : sizerStack.back()};
//                     switch (define->second.type) {
//                         case Setting::SettingType::TOGGLE: createToggle(define->second, parent, sectionSizer); break;
//                         case Setting::SettingType::NUMERIC: createNumeric(define->second, parent, sectionSizer); break;
//                         case Setting::SettingType::DECIMAL: createDecimal(define->second, parent, sectionSizer); break;
//                         case Setting::SettingType::OPTION: createOption(define->second, parent, sectionSizer); break;
//                     }
//                     if (define->second.isDefault) define->second.setValue(true);
//                 }
//             }
//         }
// 
//         while (true) {
//             ++entryStack.back().first;
//             if (entryStack.back().first == entryStack.back().second) {
//                 entryStack.pop_back();
//                 if (not sizerStack.empty()) {
//                     if (auto *staticBoxSizer = dynamic_cast<wxStaticBoxSizer *>(sizerStack.back())) {
//                         if (not parentStack.empty() and staticBoxSizer->GetStaticBox() == parentStack.back()) parentStack.pop_back();
//                     }
//                     sizerStack.pop_back();
//                 }
//                 if (entryStack.empty()) break;
//                 continue;
//             }
//             break;
//         }
//     }
//     
//     if (mSizer->IsEmpty()) {
//         mSizer->AddStretchSpacer();
//         mSizer->Add(new wxStaticText(this, wxID_ANY, _("This prop file has no options")), wxSizerFlags{}.Expand());
//         mSizer->AddStretchSpacer();
//     }
// 
//     SetSizerAndFit(mSizer);
// #   undef ITEMBORDER
// }

Versions::PropButtons parseButtons(
    const PConf::Data& data,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Versions::parseButtons()")};
    const auto hashedData{PConf::hash(data)};

    Versions::PropButtons ret{};
    for (const auto& stateEntry : data) {
        if (stateEntry->name != "STATE") {
            logger.warn("Skipping " + stateEntry->name + " entry in buttons...");
            continue;
        }

        if (not stateEntry->label) {
            logger.warn("Skipping buttons state w/o label...");
            continue;
        }

        if (stateEntry->getType() != PConf::Type::SECTION) {
            logger.warn("Skipping non-section buttons state...");
            continue;
        }

        vector<Versions::PropButton> buttons;
        const auto& buttonEntries{
            std::static_pointer_cast<PConf::Section>(stateEntry)->entries
        };
        for (const auto& buttonEntry : buttonEntries) {
            if (buttonEntry->name != "BUTTON") {
                logger.warn("Skipping " + stateEntry->name + " entry in buttons state...");
                continue;
            }

            if (not buttonEntry->label) {
                logger.warn("Skipping button w/o label...");
                continue;
            }

            if (buttonEntry->getType() != PConf::Type::SECTION) {
                logger.warn("Skipping non-section button...");
                continue;
            }

            std::unordered_map<string, string> descriptions;
            const auto& descEntries{
                std::static_pointer_cast<PConf::Section>(buttonEntry)->entries
            };
            for (const auto& descEntry : descEntries) {
                if (descEntry->name != "DESCRIPTION") {
                    logger.warn("Skipping " + stateEntry->name + " entry in button...");
                    continue;
                }

                if (not descEntry->value) {
                    logger.warn("Skipping button description w/o value...");
                    continue;
                }

                const auto [iter, success]{descriptions.try_emplace(
                    descEntry->label.value_or(""),
                    descEntry->value.value()
                )};
                if (not success) {
                    if (not descEntry->label) logger.warn("Skipping duplicate base button description...");
                    else logger.warn("Skipping duplicate \"" + *descEntry->label + "\" button description...");
                }
            }

            buttons.emplace_back(Versions::PropButton{.name=buttonEntry->label.value(), .descriptions=descriptions});
        }

        ret.emplace_back(stateEntry->label.value(), buttons);
    }

    return ret;
}

Versions::PropErrors parseErrors(const PConf::Data& data, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Versions::parseErrors()")};
    Versions::PropErrors ret;
    
    for (const auto& mapEntry : data) {
        if (mapEntry->name != "MAP") {
            logger.warn("Skipping " + mapEntry->name + " entry in errors...");
            continue;
        }

        if (mapEntry->getType() != PConf::Type::SECTION) {
            logger.warn("Skipping non-section MAP in errors...");
            continue;
        }

        const auto mapEntries{
            PConf::hash(std::static_pointer_cast<PConf::Section>(mapEntry)->entries)
        };
        auto arduinoError{mapEntries.find("ARDUINO")};
        if (arduinoError == mapEntries.end()) {
            logger.warn("Skipping error map w/o arduino error entry...");
            continue;
        }
        if (not arduinoError->second->value) {
            logger.warn("Skipping error map w/o arduino error entry value...");
            continue;
        }

        auto displayError{mapEntries.find("DISPLAY")};
        if (displayError == mapEntries.end()) {
            logger.warn("Skipping error map w/o display error entry...");
            continue;
        }
        if (not displayError->second->value) {
            logger.warn("Skipping error map w/o display error entry value...");
            continue;
        }

        ret.emplace_back(*arduinoError->second->value, *displayError->second->value);
    }

    return ret;
}

} // namespace

