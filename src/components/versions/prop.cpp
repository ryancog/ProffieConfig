#include "prop.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <fstream>
#include <memory>

#include <unordered_set>

#include "log/branch.h"
#include "log/context.h"
#include "log/logger.h"
#include "pconf/pconf.h"
#include "pconf/utils.h"
#include "utils/string.h"

namespace Versions {
    list<PropSettingVariant> parseSettings(const PConf::Data&, Prop&, Log::Branch&);
    optional<std::pair<PropCommonSettingData, PConf::HashedData>> parseSettingCommon(const std::shared_ptr<PConf::Entry>&, Log::Logger&);
    PropButtons parseButtons(const PConf::Data&, const PropSettingMap&, Log::Branch&);
    PropErrors parseErrors(const PConf::Data& data, Log::Branch& lBranch);
}

Versions::PropButtonState::PropButtonState(string stateName, vector<PropButton> buttons) : 
    stateName{std::move(stateName)}, buttons{std::move(buttons)} {}

Versions::PropErrorMapping::PropErrorMapping(string arduinoError, string displayError) :
    arduinoError{std::move(arduinoError)}, displayError{std::move(displayError)} {}

bool Versions::PropSetting::shouldOutputDefine() const {
    switch (type) {
        case Type::TOGGLE:
            return mEnabled and static_cast<const PropToggle *>(this)->value;
            break;
        case Type::SELECTION:
            return mEnabled and static_cast<const PropSelection *>(this)->value();
            break;
        case Type::NUMERIC:
        case Type::DECIMAL:
            return mEnabled;
            break;
    }
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

optional<Versions::Prop> Versions::Prop::generate(const PConf::HashedData& data) {
    auto& logger{Log::Context::getGlobal().createLogger("Versions::Prop::generate()")};

    const auto nameIter{data.find("NAME")};
    if (nameIter == data.end() or not nameIter->second->value) {
        logger.error("Missing NAME entry.");
        return nullopt;
    }

    const auto filenameIter{data.find("FILENAME")};
    if (filenameIter == data.end() or not filenameIter->second->value) {
        logger.error("Missing FILENAME entry.");
        return nullopt;
    }

    const auto infoIter{data.find("INFO")};
    if (infoIter == data.end() or not infoIter->second->value) {
        logger.info("Skipping missing INFO entry...");
    } 

    Prop prop{
        nameIter->second->value.value(),
        filenameIter->second->value.value(),
        infoIter->second->value.value_or("Prop has no additional info.")
    };

    const auto settingsIter {data.find("SETTINGS")};
    if (settingsIter == data.end() or settingsIter->second->getType() != PConf::Type::SECTION) {
        logger.info("Skipping missing SETTINGS section...");
    } else {
        prop.mSettings = parseSettings(
            std::static_pointer_cast<PConf::Section>(settingsIter->second)->entries,
            prop,
            *logger.bdebug("Parsing SETTINGS...")
        );
    }

    std::set<PropSetting *> settingsUsed{};
    const auto layoutIter{data.find("LAYOUT")};
    if (layoutIter == data.end() or layoutIter->second->getType() != PConf::Type::SECTION) {
        logger.info("Skipping missing LAYOUT section...)");
    } else {
        settingsUsed = PropLayout::generate(
            std::static_pointer_cast<PConf::Section>(layoutIter->second)->entries,
            prop.mSettings,
            prop.mLayout,
            &logger.binfo("Parsing LAYOUT...")->createLogger("PropLayout::generate()")
        );
    }

    // Purge unused settings and generate setting map
    for (auto setting{prop.mSettings.begin()}; setting != prop.mSettings.end();) {
        PropSetting *ptr;
        if (
                (ptr = std::get_if<PropToggle>(&*setting)) or
                (ptr = std::get_if<PropNumeric>(&*setting)) or
                (ptr = std::get_if<PropDecimal>(&*setting))
           ) {
            if (settingsUsed.find(ptr) == settingsUsed.end()) {
                logger.warn("Removing unused setting \"" + ptr->name + "\"...");
                setting = prop.mSettings.erase(setting);
                continue;
            } else {
                prop.mSettingMap.emplace(ptr->define, ptr);
            }
        } else if (auto *ptr = std::get_if<PropOption>(&*setting)) {
            for (auto selection{ptr->mSelections.begin()}; selection != ptr->mSelections.end();) {
                if (settingsUsed.find(&*selection) == settingsUsed.end()) {
                    logger.warn("Removing unused setting \"" + selection->name + "\"...");
                    selection = ptr->mSelections.erase(selection);
                    continue;
                }
                ++selection;
            }

            if (ptr->mSelections.empty()) {
                logger.warn("Removing empty option...");
                setting = prop.mSettings.erase(setting);
                continue;
            } else {
                for (auto& selection : ptr->mSelections) {
                    prop.mSettingMap.emplace(selection.define, &selection);
                }
            }
        }

        ++setting;
    }

    const auto buttonsRange{data.equal_range("BUTTONS")};
    if (buttonsRange.first == buttonsRange.second) {
        logger.info("Skipping missing BUTTONS section...)");
    } else {
        for (auto it{buttonsRange.first}; it != buttonsRange.second; ++it) {
            auto& entry{it->second};
            if (entry->getType() != PConf::Type::SECTION) {
                logger.warn("Skipping non-section BUTTONS...");
                continue;
            }

            if (not entry->labelNum) {
                logger.warn("Skipping BUTTONS w/o num label...");
                continue;
            }

            const auto numButtons{entry->labelNum.value()};
            if (numButtons < 0 or numButtons > prop.mButtons.size()) {
                logger.warn( "Skipping BUTTONS with out of range num (" + 
                    std::to_string(numButtons) + ")... ");
                continue;
            }

            if (not prop.mButtons[numButtons].empty()) {
                logger.warn("Skipping duplicate BUTTONS{" + std::to_string(numButtons) + "}...");
                continue;
            }

            prop.mButtons[numButtons] = parseButtons(
                std::static_pointer_cast<PConf::Section>(entry)->entries,
                prop.mSettingMap,
                *logger.binfo("Parsing buttons " + std::to_string(numButtons) + "...")
            );
        }
    }

    const auto errorsIter{data.find("ERRORS")};
    if (errorsIter == data.end() or errorsIter->second->getType() != PConf::Type::SECTION) {
        logger.info("Skipping missing ERRORS section...)");
    } else {
        prop.mErrors = parseErrors(
            std::static_pointer_cast<PConf::Section>(errorsIter->second)->entries,
            *logger.binfo("Parsing errors...")
        );
    }

    return prop;
}

list<Versions::PropSettingVariant> Versions::parseSettings(
    const PConf::Data& data,
    Prop& prop,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("PropFile::readSettings()")};

    list<PropSettingVariant> ret;
    const auto hashedData{PConf::hash(data)};

    const auto parseDisables{[](const PConf::HashedData& data) -> vector<string> {
        const auto disableEntry{data.find("DISABLE")};
        if (disableEntry == data.end()) return {};
        if (not disableEntry->second->value) return {};
        return PConf::valueAsList(*disableEntry->second->value);
    }};

    const auto toggleRange{hashedData.equal_range("TOGGLE")};
    for (auto it{toggleRange.first}; it != toggleRange.second; ++it) {
        const auto commonData{parseSettingCommon(it->second, logger)};
        if (not commonData) continue;
        const auto& [settingData, entryMap]{*commonData};

        PropToggle toggle{
            prop,
            settingData.name,
            settingData.define,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            parseDisables(entryMap),
        };

        ret.emplace_back(toggle);
    }

    const auto optionRange{hashedData.equal_range("OPTION")};
    for (auto it{optionRange.first}; it != optionRange.second; ++it) {
        if (it->second->getType() != PConf::Type::SECTION) {
            logger.warn(it->second->name + " is not section, ignoring!");
            continue;
        }

        vector<PropOption::PropSelectionData> selectionDatas;

        const auto optionData{PConf::hash(std::static_pointer_cast<PConf::Section>(it->second)->entries)};
        const auto selectionRange{optionData.equal_range("SELECTION")};
        for (auto selectionIt{selectionRange.first}; selectionIt != selectionRange.second; ++selectionIt) {
            const auto commonData{parseSettingCommon(selectionIt->second, logger)};
            if (not commonData)  continue;
            const auto& [settingData, entryMap]{*commonData};

            PropOption::PropSelectionData selectionData{
                {
                    settingData.name,
                    settingData.define,
                    settingData.description,
                    settingData.required,
                    settingData.requiredAny,
                },
                parseDisables(entryMap),
                entryMap.find("NO_OUTPUT") != entryMap.end(),
            };

            selectionDatas.emplace_back(selectionData);
        }

        ret.emplace_back(PropOption{prop, selectionDatas});
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
            min = strtol(minEntry->second->value->c_str(), nullptr, 10);
        }
        const auto maxEntry{entryMap.find("MAX")};
        if (maxEntry != entryMap.end() and maxEntry->second->value) {
            max = strtol(maxEntry->second->value->c_str(), nullptr, 10);
        }
        const auto incrementEntry{entryMap.find("INCREMENT")};
        if (incrementEntry != entryMap.end() and incrementEntry->second->value) {
            increment = strtol(incrementEntry->second->value->c_str(), nullptr, 10);
        }
        const auto defaultEntry{entryMap.find("DEFAULT")};
        if (defaultEntry != entryMap.end() and defaultEntry->second->value) {
            defaultVal = strtol(defaultEntry->second->value->c_str(), nullptr, 10);
        }

        PropNumeric numeric{
            prop,
            settingData.name,
            settingData.define,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            min,
            max,
            increment,
            defaultVal,
        };

        ret.emplace_back(numeric);
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

        PropDecimal decimal{
            prop,
            settingData.name,
            settingData.define,
            settingData.description,
            settingData.required,
            settingData.requiredAny,
            min,
            max,
            increment,
            defaultVal,
        };

        ret.emplace_back(decimal);
    }

    return ret;
}

optional<std::pair<Versions::PropCommonSettingData, PConf::HashedData>> Versions::parseSettingCommon(
    const std::shared_ptr<PConf::Entry>& entry,
    Log::Logger& logger
) {
    PropCommonSettingData commonData;

    if (not entry->label) {
        logger.warn(entry->name + " section has no label, ignoring!");
        return nullopt;
    }

    commonData.define = *entry->label;
    Utils::trimWhiteSpace(commonData.define);
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
        commonData.requiredAny = PConf::valueAsList(*requireAnyIter->second->value);
        data.erase(requireAnyIter);
    }

    const auto requiredIter{data.find("REQUIRE")};
    if (requiredIter != data.end() and requiredIter->second->value) {
        commonData.required = PConf::valueAsList(*requiredIter->second->value);
        data.erase(requiredIter);
    }

    return std::pair{commonData, data};
}

std::set<Versions::PropSetting *> Versions::PropLayout::generate(
    const PConf::Data& data,
    const list<PropSettingVariant>& settings,
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

            const PropSetting *foundSetting{nullptr};
            for (const auto& setting : settings) {
                const PropSetting *ptr;
                if (
                        (ptr = std::get_if<PropToggle>(&setting)) or
                        (ptr = std::get_if<PropNumeric>(&setting)) or
                        (ptr = std::get_if<PropDecimal>(&setting))
                   ) {
                    if (ptr->define == *entry->label) {
                        foundSetting = ptr;
                        break;
                    }
                } else {
                    for (const auto& sel : std::get<PropOption>(setting).selections()) {
                        if (sel.define == *entry->label) {
                            foundSetting = &sel;
                            break;
                        }
                    }
                    if (foundSetting) break;
                }
            }
            if (not foundSetting) {
                logger->warn("Skipping unknown setting " + *entry->label + "...");
                continue;
            }

            usedSettings.emplace(const_cast<PropSetting *>(foundSetting));
            out.children.emplace_back(const_cast<PropSetting *>(foundSetting));
            continue;
        } 

        PropLayout::Axis axis;
        if (entry->name == "HORIZONTAL") {
            axis = PropLayout::Axis::HORIZONTAL;
        } else if (entry->name == "VERTICAL") {
            axis = PropLayout::Axis::VERTICAL;
        } else {
            logger->warn("Skipping " + entry->name + " entry in layout...");
            continue;
        }

        if (entry->getType() != PConf::Type::SECTION) continue;
        if (entry->label) out.label = *entry->label;
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

Versions::PropButtons Versions::parseButtons(
    const PConf::Data& data,
    const PropSettingMap& settingMap,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Versions::parseButtons()")};
    const auto hashedData{PConf::hash(data)};

    PropButtons ret;
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

        vector<PropButton> buttons;
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

            buttons.emplace_back(PropButton{buttonEntry->label.value(), descriptions});
        }

        ret.emplace_back(PropButtonState{stateEntry->label.value(), buttons});
    }

    return ret;
}

Versions::PropErrors Versions::parseErrors(const PConf::Data& data, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Versions::parseErrors()")};
    PropErrors ret;
    
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

