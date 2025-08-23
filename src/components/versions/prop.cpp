#include "prop.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "log/branch.h"
#include "log/context.h"
#include "log/logger.h"
#include "pconf/utils.h"

namespace {

vector<std::unique_ptr<Versions::PropSettingBase>> parseSettings(
    const PConf::Data&,
    Versions::Prop&,
    Log::Branch&
);

struct CommonData {
    string name;
    string description;
    vector<string> required;
    vector<string> requiredAny;
};

/**
 * Parse entries that are somewhat common across setting/data sections,
 * and verify that a label is present for the section (see requireLabel).
 *
 * @param requireLabel If the label is actually required for this section
 * @param requireName If the NAME entry is required for this section
 */
optional<std::pair<CommonData, PConf::HashedData>> parseSettingCommon(
    const PConf::EntryPtr&,
    Log::Logger&,
    bool requireLabel,
    bool requireName
);
Versions::PropButtons parseButtons(const PConf::Data&, Log::Branch&);
Versions::PropErrors parseErrors(const PConf::Data&, Log::Branch&);

} // namespace

Versions::PropOption::PropOption(
    Prop& prop,
    string id,
    string name,
    string description,
    const vector<PropSelectionData>& selectionDatas
) : PropSettingBase(PropSettingType::OPTION),
    idLabel{std::move(id)},
    name{std::move(name)},
    description{std::move(description)}, selection{static_cast<uint32>(selectionDatas.size())} {
    for (const auto& selectionData : selectionDatas) {
        mSelections.push_back(std::make_unique<PropSelection>(
            prop,
            *this,
            selectionData.name,
            selectionData.define,
            selectionData.description,
            selectionData.required,
            selectionData.requiredAny,
            selectionData.disables
        ));
    }
}

Versions::PropOption::PropOption(const PropOption& other, Prop& prop) :
    PropSettingBase(PropSettingType::OPTION),
    idLabel{other.idLabel},
    name{other.name},
    description{other.description},
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

Versions::PropLayout::PropLayout(const PropLayout& other, const PropSettingMap& map) :
    axis{other.axis},
    label{other.label} {

    const auto processChildren{[map](
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
            } else if (const auto *ptr = std::get_if<PropSettingBase *>(&child)){
                const auto settingIter{map.find((*ptr)->id())};
                assert(settingIter != map.end());
                children.emplace_back(settingIter->second);
            } else {
                assert(0);
            }
        }
        return children;
    }};

    children = processChildren(processChildren, other.children);
}

void Versions::PropLayout::generate(
    const PConf::Data& data,
    const PropSettingMap& map,
    PropLayout& out,
    std::unordered_set<string>& usedSettings,
    Log::Logger *logger
) {
    if (not logger) logger = &Log::Context::getGlobal().createLogger("PropLayout::generate()");

    for (const auto& entry : data) {
        if (entry->name == "SETTING") {
            if (not entry->label) {
                logger->warn("Skipping setting without label...");
                continue;
            }

            const auto settingIter{map.find(*entry->label)};
            if (settingIter == map.end()) {
                logger->warn("Skipping unknown setting " + *entry->label + "...");
                continue;
            }

            if (usedSettings.contains(*entry->label)) {
                logger->warn("Setting " + *entry->label + " appeared in layout twice!");
                continue;
            }

            usedSettings.insert(*entry->label);
            out.children.emplace_back(settingIter->second);
            continue;
        } 

        wxOrientation orient{};
        if (entry->name == "HORIZONTAL") {
            orient = wxHORIZONTAL;
        } else if (entry->name == "VERTICAL") {
            orient = wxVERTICAL;
        } else {
            logger->warn("Skipping " + entry->name + " entry in layout...");
            continue;
        }

        if (not entry.section()) {
            logger->warn("Skipping non-section " + entry->name + " in layout...");
            continue;
        }
        auto& child{std::get<PropLayout>(
            out.children.emplace_back(
                std::in_place_type<PropLayout>,
                orient,
                entry->label.value_or("")
            )
        )};

        generate(
            entry.section()->entries,
            map,
            child,
            usedSettings,
            logger
        );
    }
}

Versions::PropButtonState::PropButtonState(string stateName, vector<PropButton> buttons) : 
    stateName{std::move(stateName)}, buttons{std::move(buttons)} {}

Versions::PropErrorMapping::PropErrorMapping(string arduinoError, string displayError) :
    arduinoError{std::move(arduinoError)}, displayError{std::move(displayError)} {}

Versions::Prop::Prop(const Prop& other) : 
    Prop{other.name, other.filename, other.info} {

    for (const auto& setting : other.mSettings) {
        switch (setting->settingType) {
            case TOGGLE:
                mSettings.emplace_back(new PropToggle(
                    static_cast<PropToggle&>(*setting), *this
                ));
                break;
            case NUMERIC:
                mSettings.emplace_back(new PropNumeric(
                    static_cast<PropNumeric&>(*setting), *this
                ));
                break;
            case DECIMAL:
                mSettings.emplace_back(new PropDecimal(
                    static_cast<PropDecimal&>(*setting), *this
                ));
                break;
            case OPTION:
                mSettings.emplace_back(new PropOption(
                    static_cast<PropOption&>(*setting), *this
                ));
                break;
        }
    }

    rebuildMaps();

    mLayout = PropLayout(other.mLayout, mSettingMap);

    mButtons.~array();
    new(&mButtons) array(other.mButtons);
    mErrors.~PropErrors();
    new(&mErrors) PropErrors(other.mErrors);
}

void Versions::Prop::migrateFrom(const Prop& from) {
    for (const auto& [fromID, fromData] : from.dataMap()) {
        auto iter{mDataMap.find(fromID)};
        if (iter == mDataMap.end()) continue;
        auto *const setting{iter->second};

        if (PropDataType::TOGGLE == fromData->dataType) {
            const auto *fromToggleData{static_cast<PropToggle *>(fromData)};

            switch (setting->dataType) {
                case PropDataType::TOGGLE:
                    static_cast<PropToggle *>(setting)->value =
                        static_cast<bool>(fromToggleData->value);
                    break;
                case PropDataType::SELECTION:
                    if (fromToggleData->value) {
                        static_cast<PropSelection *>(setting)->select();
                    }
                    break;
                case PropDataType::NUMERIC:
                case PropDataType::DECIMAL:
                    // These don't convert from a toggle
                    break;
            }
        } else if (PropDataType::SELECTION == fromData->dataType) {
            const auto *fromSelectionData{static_cast<PropSelection *>(fromData)};

            switch (setting->dataType) {
                case PropDataType::TOGGLE:
                    static_cast<PropToggle *>(setting)->value = fromSelectionData->value();
                    break;
                case PropDataType::SELECTION:
                    if (fromSelectionData->value()) {
                        static_cast<PropSelection *>(setting)->select();
                    }
                    break;
                case PropDataType::NUMERIC:
                case PropDataType::DECIMAL:
                    // These don't convert from a toggle
                    break;
            }
        } else if (PropDataType::NUMERIC == fromData->dataType) {
            const auto *fromNumericData{static_cast<PropNumeric *>(fromData)};

            switch (setting->dataType) {
                case PropDataType::TOGGLE:
                case PropDataType::SELECTION:
                    // These don't convert from numeric
                    break;
                case PropDataType::NUMERIC:
                    static_cast<PropNumeric *>(setting)->value =
                        static_cast<int32>(fromNumericData->value);
                    break;
                case PropDataType::DECIMAL:
                    static_cast<PropDecimal *>(setting)->value =
                        static_cast<int32>(fromNumericData->value);
                    break;
            }
        } else if (PropDataType::DECIMAL == fromData->dataType) {
            const auto *fromDecimalData{static_cast<PropDecimal *>(fromData)};

            switch (setting->dataType) {
                case PropDataType::TOGGLE:
                case PropDataType::SELECTION:
                    // These don't convert from numeric
                    break;
                case PropDataType::NUMERIC:
                    static_cast<PropNumeric *>(setting)->value =
                        static_cast<int32>(fromDecimalData->value);
                    break;
                case PropDataType::DECIMAL:
                    static_cast<PropDecimal *>(setting)->value =
                        static_cast<float64>(fromDecimalData->value);
                    break;
            }
        }
    }
}

void Versions::Prop::rebuildMaps(
    optional<std::unordered_set<string>> pruneList,
    Log::Branch *lBranch
) {
    auto& logger{Log::Branch::optCreateLogger("Versions::Prop::rebuildSettingMap()", lBranch)};
    mDataMap.clear();
    mSettingMap.clear();

    for (auto iter{mSettings.begin()}; iter != mSettings.end();) {
        auto *setting{(*iter).get()};
        if (pruneList and not pruneList->contains(setting->id())) {
            logger.warn("Removing unused setting \"" + setting->id() + "\"...");
            iter = mSettings.erase(iter);
            continue;
        } 

        if (PropSettingType::OPTION == setting->settingType) {
            const auto& option{static_cast<const PropOption *>(setting)};
            if (option->selections().empty()) {
                logger.warn("Removing empty option...");
                iter = mSettings.erase(iter);
                continue;
            }

            for (const auto& selection : option->selections()) {
                mDataMap.emplace(selection->define, selection.get());
            }
        } else if (PropSettingType::TOGGLE == setting->settingType) {
            auto *toggle{static_cast<PropToggle *>(setting)};
            mDataMap.emplace(toggle->define, toggle);
        } else if (PropSettingType::NUMERIC == setting->settingType) {
            auto *numeric{static_cast<PropNumeric *>(setting)};
            mDataMap.emplace(numeric->define, numeric);
        } else if (PropSettingType::DECIMAL == setting->settingType) {
            auto *decimal{static_cast<PropDecimal *>(setting)};
            mDataMap.emplace(decimal->define, decimal);
        } else {
            assert(0);
        }

        mSettingMap.emplace(setting->id(), setting);
        ++iter;
    }
}

std::unique_ptr<Versions::Prop> Versions::Prop::generate(
    const PConf::HashedData& data,
    Log::Branch *lBranch,
    bool forDefault
) {
    auto& logger{Log::Branch::optCreateLogger("Versions::Prop::generate()", lBranch)};

    string name;
    if (forDefault) {
        name = "Default";
    } else {
        const auto nameEntry{data.find("NAME")};
        if (not nameEntry or not nameEntry->value) {
            logger.error("Missing name.");
            return nullptr;
        }
        name = *nameEntry->value;
    }

    string filename;
    if (not forDefault) {
        const auto filenameEntry{data.find("FILENAME")};
        if (not filenameEntry or not filenameEntry->value) {
            logger.error("Missing filename.");
            return nullptr;
        }
        filename = *filenameEntry->value;
    }

    string info;
    const auto infoEntry{data.find("INFO")};
    if (not infoEntry or not infoEntry->value) {
        logger.info("No info...");
        if (forDefault) {
            info = "The ProffieOS default prop.";
        } else {
            info = "Prop has no additional info.";
        }
    } else {
        info = *infoEntry->value;
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
            settingsEntry.section()->entries,
            *prop,
            *logger.bdebug("Parsing SETTINGS...")
        );
    }

    prop->rebuildMaps(nullopt, logger.bdebug("Building maps..."));

    std::unordered_set<string> settingsUsed{};
    const auto layoutEntry{data.find("LAYOUT")};
    if (not layoutEntry or not layoutEntry.section()) {
        logger.info("No layout section...");
    } else {
        PropLayout::generate(
            layoutEntry.section()->entries,
            prop->mSettingMap,
            prop->mLayout,
            settingsUsed,
            &logger.bdebug("Parsing LAYOUT...")->createLogger("PropLayout::generate()")
        );
    }

    prop->rebuildMaps(settingsUsed, logger.bdebug("Rebuilding setting map..."));

    const auto buttonEntries{data.findAll("BUTTONS")};
    for (const auto& buttonEntry : buttonEntries) {
        if (not buttonEntry.section()) {
            logger.warn("Skipping non-section BUTTONS...");
            continue;
        }

        if (not buttonEntry->labelNum) {
            logger.warn("Skipping BUTTONS w/o num label...");
            continue;
        }

        const auto numButtons{*buttonEntry->labelNum};
        if (numButtons < 0 or numButtons > prop->mButtons.size()) {
            logger.warn("Skipping BUTTONS with out of range num (" + std::to_string(numButtons) + ")... ");
            continue;
        }

        if (not prop->mButtons[numButtons].empty()) {
            logger.warn("Skipping duplicate BUTTONS{" + std::to_string(numButtons) + "}...");
            continue;
        }

        prop->mButtons[numButtons] = parseButtons(
            buttonEntry.section()->entries,
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
            errorsEntry.section()->entries,
            *logger.bdebug("Parsing errors...")
        );
    }

    return prop;
}


bool Versions::PropDataBase::isActive() const {
    switch (dataType) {
        case PropDataType::TOGGLE:
            return 
                static_cast<const PropToggle *>(this)->value.isEnabled() and
                static_cast<const PropToggle *>(this)->value;
        case PropDataType::SELECTION:
            return 
                static_cast<const PropSelection *>(this)->enabled() and
                static_cast<const PropSelection *>(this)->value();
        case PropDataType::NUMERIC:
            return static_cast<const PropNumeric *>(this)->value.isEnabled();
        case PropDataType::DECIMAL:
            return static_cast<const PropDecimal *>(this)->value.isEnabled();
    }
    assert(0);
}

bool Versions::PropDataBase::shouldOutputDefine() const {
    if (not isActive()) return false;

    if (PropDataType::SELECTION == dataType) {
        return not static_cast<const PropSelection *>(this)->define.empty();
    } 

    if (dataType == PropDataType::NUMERIC) {
        const auto *numeric{static_cast<const PropNumeric *>(this)};
        return numeric->defaultVal != numeric->value;
    }

    if (dataType == PropDataType::DECIMAL) {
        const auto *decimal{static_cast<const PropDecimal *>(this)};
        return decimal->defaultVal != decimal->value;
    }

    return true;
}

optional<string> Versions::PropDataBase::generateDefineString() const {
    if (not shouldOutputDefine()) return nullopt;

    switch (dataType) {
        case PropDataType::TOGGLE:
            return static_cast<const PropToggle *>(this)->value ?
                optional{define} :
                nullopt;
        case PropDataType::SELECTION:
            return static_cast<const PropSelection *>(this)->value() ?
                optional{define} :
                nullopt;
        case PropDataType::NUMERIC:
            return 
                define + " " +
                std::to_string(static_cast<const PropNumeric *>(this)->value);
        case PropDataType::DECIMAL:
            return 
                define + " " +
                std::to_string(static_cast<const PropDecimal *>(this)->value);
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

vector<std::unique_ptr<Versions::PropSettingBase>> parseSettings(
    const PConf::Data& data,
    Versions::Prop& prop,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("PropFile::readSettings()")};

    vector<std::unique_ptr<Versions::PropSettingBase>> ret;
    const auto hashedData{PConf::hash(data)};

    const auto parseDisables{[](const PConf::HashedData& data) -> vector<string> {
        const auto disableEntry{data.find("DISABLE")};
        if (not disableEntry or not disableEntry->value) return {};
        return PConf::valueAsList(disableEntry->value);
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

        ret.emplace_back(new Versions::PropToggle(
            prop,
            settingData.name,
            *toggleEntry->label,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
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

        const auto optionData{PConf::hash(optionEntry.section()->entries)};
        vector<Versions::PropOption::PropSelectionData> selectionDatas;

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

            Versions::PropOption::PropSelectionData selectionData{
                .name = settingData.name,
                .define = selectionEntry->label.value_or(""),
                .description = settingData.description,
                .required = settingData.required,
                .requiredAny = settingData.requiredAny,
                .disables = parseDisables(entryMap),
            };

            selectionDatas.emplace_back(std::move(selectionData));
        }

        ret.emplace_back(new Versions::PropOption(
            prop,
            *optionEntry->label,
            settingData.name,
            settingData.description,
            selectionDatas
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

        int32 min{0};
        int32 max{100};
        int32 increment{1};
        optional<int32> defaultVal;

        const auto minEntry{entryMap.find("MIN")};
        if (minEntry and minEntry->value) {
            try {
                min = std::stoi(*minEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " min: " + e.what());
            }
        }
        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry and maxEntry->value) {
            try {
                max = std::stoi(*maxEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " max: " + e.what());
            }
        }
        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry and incrementEntry->value) {
            try {
                increment = std::stoi(*incrementEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " increment: " + e.what());
            }
        }
        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry and defaultEntry->value) {
            try {
                defaultVal = std::stoi(*defaultEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " default: " + e.what());
            }
        }

        ret.emplace_back(new Versions::PropNumeric(
            prop,
            settingData.name,
            *numericEntry->label,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            min,
            max,
            increment,
            defaultVal
        ));
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

        float64 min{0};
        float64 max{10};
        float64 increment{0.1};
        float64 defaultVal{0};

        const auto minEntry{entryMap.find("MIN")};
        if (minEntry and minEntry->value) {
            try {
                min = std::stod(*minEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " min: " + e.what());
            }
        }
        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry and maxEntry->value) {
            try {
                max = std::stod(*maxEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " max: " + e.what());
            }
        }
        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry and incrementEntry->value) {
            try {
                increment = std::stod(*incrementEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " increment: " + e.what());
            }
        }
        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry and defaultEntry->value) {
            try {
                defaultVal = std::stod(*defaultEntry->value);
            } catch (const std::exception& e) {
                logger.warn("Could not parse " + settingData.name + " default: " + e.what());
            }
        }

        ret.emplace_back(new Versions::PropDecimal(
            prop,
            settingData.name,
            *decimalEntry->label,
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

optional<std::pair<CommonData, PConf::HashedData>> parseSettingCommon(
    const PConf::EntryPtr& entry,
    Log::Logger& logger,
    bool requireLabel,
    bool requireName
) {
    CommonData commonData;

    if (not entry->label and requireLabel) {
        logger.warn(entry->name + " section has no label, ignoring!");
        return nullopt;
    }

    if (not entry.section()) {
        logger.warn(entry->name + " is not section, ignoring!");
        return nullopt;
    }

    auto data{PConf::hash(entry.section()->entries)};
    const auto nameEntry{data.find("NAME")};
    if (not nameEntry or not nameEntry->value) {
        if (requireName) {
            logger.warn(entry->name + " section does not have the required \"NAME\" entry, ignoring!");
            return nullopt;
        }
    } else {
        commonData.name = *nameEntry->value;
    }
    if (nameEntry) data.erase(nameEntry);

    const auto descEntry{data.find("DESCRIPTION")};
    if (descEntry and descEntry->value) {
        commonData.description = *descEntry->value;
        data.erase(descEntry);
    }

    const auto requireAnyEntry{data.find("REQUIREANY")};
    if (requireAnyEntry and requireAnyEntry->value) {
        commonData.requiredAny = PConf::valueAsList(requireAnyEntry->value);
        data.erase(requireAnyEntry);
    }

    const auto requiredEntry{data.find("REQUIRE")};
    if (requiredEntry and requiredEntry->value) {
        commonData.required = PConf::valueAsList(requiredEntry->value);
        data.erase(requiredEntry);
    }

    return std::pair{commonData, data};
}

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

        if (not stateEntry.section()) {
            logger.warn("Skipping non-section buttons state...");
            continue;
        }

        vector<Versions::PropButton> buttons;
        const auto& buttonEntries{stateEntry.section()->entries};
        for (const auto& buttonEntry : buttonEntries) {
            if (buttonEntry->name != "BUTTON") {
                logger.warn("Skipping " + stateEntry->name + " entry in buttons state...");
                continue;
            }

            if (not buttonEntry->label) {
                logger.warn("Skipping button w/o label...");
                continue;
            }

            if (not buttonEntry.section()) {
                logger.warn("Skipping non-section button...");
                continue;
            }

            std::unordered_map<string, string> descriptions;
            const auto& descEntries{buttonEntry.section()->entries};
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

        if (not mapEntry.section()) {
            logger.warn("Skipping non-section MAP in errors...");
            continue;
        }

        const auto mapData{PConf::hash(mapEntry.section()->entries)};
        const auto arduinoEntry{mapData.find("ARDUINO")};
        if (not arduinoEntry or not arduinoEntry->value) {
            logger.warn("Skipping error map w/o arduino error entry...");
            continue;
        }

        const auto displayEntry{mapData.find("DISPLAY")};
        if (not displayEntry or not displayEntry->value) {
            logger.warn("Skipping error map w/o display error entry...");
            continue;
        }

        ret.emplace_back(*arduinoEntry->value, *displayEntry->value);
    }

    return ret;
}

} // namespace

