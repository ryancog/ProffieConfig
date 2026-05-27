#include "blades.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/preset/blades.cpp
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

#include <sstream>

#include <wx/translation.h>

#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/blades/bladeconfig.hpp"
#include "config/priv/io.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"
#include "log/branch.hpp"
#include "utils/string.hpp"

using namespace config;
using namespace config::priv;
using namespace config::blades;

namespace {

std::optional<std::string> parseBlade(
    std::string, BladeConfig&, Blade&, logging::Branch&
);

} // namespace

std::optional<std::string> parse::preset::blades(
    const std::string& buf, Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parse::blades()")};
    std::istringstream stream(buf);

    enum {
        eNone,

        eID,
        eBlade_Entry,
        eConfig_Array,
        eConfig_Array_Inner,
        ePost_Config_Array,
        eName,
    } reading{eNone};

    std::string buffer;
    std::vector<char> depth;

    auto bladeConfigs{data::context(config.bladeConfigs_)};

    while (stream.good()) {
        utils::CommentData commentData{.stream_=stream};
        (void)utils::extractComments(commentData);

        const auto chr{stream.get()};
        if (not stream.good())
            return {};

        if (reading == eNone) {
            if (chr == '{') {
                bladeConfigs.append<BladeConfig>(config);
                reading = eID;
                buffer.clear();
            }
        } else if (reading == eID) {
            if (chr == ',') {
                utils::trimWhitespace(buffer);

                auto idVal{buffer == "NO_BLADE"
                    ? NO_BLADE
                    : utils::doStringMath(buffer)
                };

                if (idVal) {
                    auto& lastArray{dynamic_cast<BladeConfig&>(
                        *bladeConfigs.children().back()
                    )};
                    lastArray.id_.set(static_cast<int32>(*idVal));
                } else logger.warn("Invalid blade ID for array " + std::to_string(bladeConfigs.children().size()));

                reading = eBlade_Entry;
                buffer.clear();
                continue;
            }

            buffer += static_cast<char>(chr);
        } else if (reading == eBlade_Entry) {
            if (std::isspace(chr)) continue;

            if (depth.empty()) {
                if (chr == ',') {
                    auto& array{dynamic_cast<BladeConfig&>(
                        *bladeConfigs.children().back()
                    )};

                    auto blades{data::context(array.blades_)};
                    auto& blade{blades.append<Blade>(config)};

                    auto res{parseBlade(
                        buffer,
                        array,
                        blade,
                        *logger.binfo("Parsing blade...")
                    )};

                    auto type{data::context(blade.type())};
                    if (type.choiceIdx() == -1) {
                        logger.debug("Removing blade parser deemed unnecessary.");

                        type.release();
                        blades.remove(blades.children().size() - 1);
                    }

                    // TODO: Shouldn't this be right after parseBlade()...?
                    if (res) return res;

                    buffer.clear();
                    continue;
                }
            }

            if (chr == '<' or chr == '(') depth.push_back(static_cast<char>(chr));
            if (chr == '>' or chr == ')') {
                if (depth.empty()) {
                    return errorMessage(logger, wxTRANSLATE("Found %c before matching open when parsing blade"), chr);
                }

                if (
                        (chr == '>' and depth.back() != '<') or
                        (chr == ')' and depth.back() != '(')
                   ) {
                    return errorMessage(logger, wxTRANSLATE("Found %c when expecting match for %c when parsing blade"), chr, depth.back());
                }

                depth.pop_back();
            }

            buffer += static_cast<char>(chr);
            if (buffer == "CONFIGARRAY") {
                reading = eConfig_Array;
                buffer.clear();
            }
        } else if (reading == eConfig_Array) {
            if (chr == '(') reading = eConfig_Array_Inner;
        } else if (reading == eConfig_Array_Inner) {
            if (chr == ')') {
                utils::trimWhitespace(buffer);

                auto presetArrays{data::context(config.presetArrays_)};
                int32 idx{0};
                for (; idx < presetArrays.children().size(); ++idx) {
                    const auto& model{presetArrays.children()[idx]};
                    auto& presetArray{dynamic_cast<presets::Array&>(*model)};

                    auto name{data::context(presetArray.name_)};
                    if (name.val() == buffer) break;
                }
                if (idx == presetArrays.children().size()) idx = -1;

                auto& array{dynamic_cast<BladeConfig&>(
                    *bladeConfigs.children().back()
                )};

                array.presetArray_.choice().choose(idx);

                reading = ePost_Config_Array;
                buffer.clear();
                continue;
            }
            buffer += static_cast<char>(chr);
        } else if (reading == ePost_Config_Array) {
            if (chr == '}') reading = eNone;
            if (chr == '"') reading = eName;
        } else if (reading == eName) {
            if (chr == '"' or chr == '}') {
                auto& array{dynamic_cast<BladeConfig&>(
                    *bladeConfigs.children().back()
                )};

                array.name_.change(std::move(buffer));

                reading = eNone;
                continue;
            }

            buffer += static_cast<char>(chr);
        }
    }

    return std::nullopt;
}

namespace {

std::optional<std::string> parseBlade(
    std::string data,
    BladeConfig& array,
    Blade& blade,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parseBlade()")};
    logger.verbose("Parsing blade \"" + data + "\"...");

    std::optional<std::string> err;

    static constexpr std::string_view DIMBLADE_STR{"DimBlade("};
    const auto parseDimBlade{[&data, &logger](int32& brightness) -> std::optional<std::string> {
        if (data.starts_with(DIMBLADE_STR)) {
            data.erase(0, DIMBLADE_STR.length());

            const auto brightCommaPos{data.find(',')};
            if (brightCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("DimBlade is missing end comma for brightness"));
            }

            const auto brightStr{data.substr(0, brightCommaPos)};
            data.erase(0, brightCommaPos + 1);

            const auto bright{utils::doStringMath(brightStr)};
            if (not bright) {
                return errorMessage(logger, wxTRANSLATE("DimBlade has malformed brightness: %s"), brightStr);
            }

            brightness = static_cast<int32>(*bright);
        }

        return std::nullopt;
    }};

    int32 firstBrightness{100};
    err = parseDimBlade(firstBrightness);
    if (err) return err;

    static constexpr std::string_view SUBBLADE_STR{"SubBlade"};
    struct {
        WS281X::Split::Type type_{WS281X::Split::eMax};
        int32 start_;
        int32 end_;
        int32 segments_;
        std::string list_;
    } splitData;
    if (data.starts_with(SUBBLADE_STR)) {
        data.erase(0, SUBBLADE_STR.length());
        if (data.empty()) {
            return errorMessage(logger, wxTRANSLATE("Unexpected end when parsing SubBlade"));
        }

        constexpr std::string_view REVERSE_STR{"Reverse("};
        constexpr std::string_view STRIDE_STR{"WithStride("};
        constexpr std::string_view ZIGZAG_STR{"ZZ("};
        constexpr std::string_view LIST_STR{"WithList<"};

        using enum WS281X::Split::Type;
        if (data[0] == '(') {
            splitData.type_ = eStandard;
            data.erase(0, 1);
        } else if (data.starts_with(REVERSE_STR)) {
            splitData.type_ = eReverse;
            data.erase(0, REVERSE_STR.length());
        } else if (data.starts_with(STRIDE_STR)) {
            splitData.type_ = eStride;
            data.erase(0, STRIDE_STR.length());
        } else if (data.starts_with(ZIGZAG_STR)) {
            splitData.type_ = eZig_Zag;
            data.erase(0, ZIGZAG_STR.length());
        } else if (data.starts_with(LIST_STR)) {
            splitData.type_ = eList;
            data.erase(0, LIST_STR.length());
        } else {
            return errorMessage(logger, wxTRANSLATE("Encountered unknown/malformed SubBlade"));
        }

        if (
                splitData.type_ == eStandard or splitData.type_ == eReverse or
                splitData.type_ == eStride or splitData.type_ == eZig_Zag
           ) {
            auto startCommaPos{data.find(',')};
            if (startCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade start"));
            }

            auto startStr{data.substr(0, startCommaPos)};
            data.erase(0, startCommaPos + 1);

            auto start{utils::doStringMath(startStr)};
            if (not start) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade start"));
            }
            splitData.start_ = static_cast<int32>(*start);

            auto endCommaPos{data.find(',')};
            if (endCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade end"));
            }

            auto endStr{data.substr(0, endCommaPos)};
            data.erase(0, endCommaPos + 1);

            auto end{utils::doStringMath(endStr)};
            if (not end) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade end"));
            }
            splitData.end_ = static_cast<int32>(*end);
        } 
        if (splitData.type_ == eStride or splitData.type_ == eZig_Zag) {
            auto segCommaPos{data.find(',')};
            if (segCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade segments"));
            }

            auto segmentsStr{data.substr(0, segCommaPos)};
            data.erase(0, segCommaPos + 1);

            auto segments{utils::doStringMath(segmentsStr)};
            if (not segments) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade segments"));
            }
            splitData.segments_ = static_cast<int32>(*segments);
        } 
        if (splitData.type_ == eZig_Zag) {
            auto columnCommaPos{data.find(',')};
            if (columnCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Failed to find end comma for SubBlade column"));
            }

            auto columnStr{data.substr(0, columnCommaPos)};
            data.erase(0, columnCommaPos + 1);

            auto column{utils::doStringMath(columnStr)};
            if (not column) {
               return errorMessage(logger, wxTRANSLATE("Failed to parse SubBlade column"));
            }

            // Column entry is not used, they're assumed to be in order.
            // Not assuming so makes this parsing more complicated and I don't
            // feel like dealing with it right now.
            //
            // All of 3 people use ZZ subblade anyways. I'll cross that bridge
            // when someone actually gets there.
        }
        if (splitData.type_ == eList) {
            // Find the `>` template closing chevron for the `SubBlade`, which
            // marks the end of the list.
            auto chevronPos{data.find('>')};
            if (chevronPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("SubBlade list unterminated"));
            }

            // The list will be processed for sanity once it's actually placed
            // into the config, just dump the raw string in for now.
            //
            // TODO: Maybe do some proper validation on this? Bad things 
            // could've happened (we've blown past into another template) here!
            splitData.list_ = data.substr(0, chevronPos);
            data.erase(0, chevronPos + 1);

            // List is unique in that it's a variadic arg'd template with the
            // indexes part of the template, so the `(` also needs to be
            // cleared, like how a comma would be cleared for the former types.
            if (data.empty()) {
                return errorMessage(logger, wxTRANSLATE("Missing data after sub blade list indexes"));
            }

            if (data[0] != '(') {
                return errorMessage(logger, wxTRANSLATE("SubBlade list has invalid chars in between '>' and '('"));
            }

            data.erase(0, 1);
        }
    }

    int32 secondBrightness{100};
    err = parseDimBlade(secondBrightness);
    if (err) return err;

    const auto addSplit{[&splitData, &firstBrightness](Blade& blade) {
        auto& ws281x{blade.ws281x()};

        auto splits{data::context(ws281x.splits_)};
        auto& split{splits.append<WS281X::Split>(ws281x)};

        split.brightness_.set(firstBrightness);
        split.type_.select(splitData.type_);

        using enum WS281X::Split::Type;

        if (
                splitData.type_ == eStandard or
                splitData.type_ == eReverse or
                splitData.type_ == eZig_Zag
           ) {
            split.start_.set(splitData.start_);
            split.end_.set(splitData.end_);
        }
        if (splitData.type_ == eStride) {
            split.start_.set(splitData.start_);
            split.end_.set(splitData.end_ + splitData.segments_ - 1);
        }
        if (splitData.type_ == eStride or splitData.type_ == eZig_Zag) {
            split.segments_.set(splitData.segments_);
        }
        if (splitData.type_ == eList) {
            split.list_.change(std::move(splitData.list_));
        }
    }};
    
    const auto parseWS281XLength{[&]() -> std::optional<std::string> {
        const auto lengthCommaPos{data.find(',')};
        if (lengthCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X length"));
        }

        const auto lengthStr{data.substr(0, lengthCommaPos)};
        data.erase(0, lengthCommaPos + 1);

        const auto lengthVal{utils::doStringMath(lengthStr)};
        if (not lengthVal) {
            return errorMessage(logger, wxTRANSLATE("Failed to parse WS281X length"));
        }

        blade.ws281x().length_.set(static_cast<int32>(*lengthVal));

        return std::nullopt;
    }};

    const auto parseWS281XData{[&]() -> std::optional<std::string> {
        const auto dataPinCommaPos{data.find(',')};
        if (dataPinCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X data pin"));
        }

        auto dataPinStr{data.substr(0, dataPinCommaPos)};
        data.erase(0, dataPinCommaPos + 1);

        blade.ws281x().dataPin_.change(std::move(dataPinStr));

        return std::nullopt;
    }};

    const auto parseWS281XPowerPins{[&]() -> std::optional<std::string> {
        if (not data.starts_with(POWER_PINS_STR)) {
            return errorMessage(logger, wxTRANSLATE("Missing WS281X PowerPINS"));
        }
        data.erase(0, POWER_PINS_STR.length());

        std::string buffer;
        for (auto idx{0}; idx < data.length(); ++idx) {
            if (data[idx] == ',' or data[idx] == '>') {
                blade.ws281x().powerPins_.select(std::move(buffer));
                buffer.clear();

                if (data[idx] == '>') break;

                continue;
            }

            buffer += data[idx];
        }

        return std::nullopt;
    }};

    static constexpr std::string_view WS281X_STR{"WS281XBladePtr<"};
    static constexpr std::string_view WS2811_STR{"WS2811BladePtr<"};
    static constexpr std::string_view SIMPLE_STR{"SimpleBladePtr<"};
    static constexpr std::string_view NULL_STR{"NULL"};
    static constexpr std::string_view NULLPTR_STR{"nullptr"};
    if (data.starts_with(WS281X_STR)) {
        data.erase(0, WS281X_STR.length());
        blade.type().choice().choose(Blade::eWS281X);

        err = parseWS281XLength();
        if (err) return err;

        err = parseWS281XData();
        if (err) return err;
        
        static constexpr std::string_view COLOR8_STR{"Color8::"};
        if (not data.starts_with(COLOR8_STR)) {
            return errorMessage(logger, wxTRANSLATE("Malformatted WS281X (missing color order)"));
        }
        data.erase(0, COLOR8_STR.length());

        const auto colorOrderCommaPos{data.find(',')};
        if (colorOrderCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Could not find end comma for WS281X color order"));
        }

        const auto colorOrderStr{data.substr(0, colorOrderCommaPos)};
        data.erase(0, colorOrderCommaPos + 1);

        const auto parse3ColorOrder{[](
            const std::string& str
        ) -> ColorOrder3 {
            size colorOrderIdx{0};
            for (; colorOrderIdx < eOrder3_Max; ++colorOrderIdx) {
                if (str == ORDER_STRS[colorOrderIdx]) break;
            }

            return static_cast<ColorOrder3>(colorOrderIdx);
        }};

        constexpr cstring INVALID_COLOR_MSG{wxTRANSLATE("Invalid/unrecognized WS281X color order: %s")};
        if (colorOrderStr.length() == 3) {
            const auto order{parse3ColorOrder(colorOrderStr)};
            if (order == eOrder3_Max) {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            auto& ws281x{blade.ws281x()};
            ws281x.hasWhite_.set(false);
            ws281x.colorOrder3_.choose(order);
        } else if (colorOrderStr.length() == 4) {
            bool offset{false};
            if (colorOrderStr[0] == 'w' or colorOrderStr[0] == 'W') {
                offset = true;
            } else if (colorOrderStr[3] != 'w' and colorOrderStr[3] != 'W') {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            const auto order3{parse3ColorOrder(
                colorOrderStr.substr(offset ? 1 : 0)
            )};
            if (order3 == eOrder3_Max) {
                return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
            }

            const auto order4Val{order3 + 
                (offset ? eOrder4_White_First_Start : 0)
            };

            auto& ws281x{blade.ws281x()};
            ws281x.hasWhite_.set(true);
            ws281x.colorOrder4_.choose(order4Val);
        } else {
            return errorMessage(logger, INVALID_COLOR_MSG, colorOrderStr);
        }

        err = parseWS281XPowerPins();
        if (err) return err;
    } else if (data.starts_with(WS2811_STR)) {
        data.erase(0, WS2811_STR.length());
        blade.type().choice().choose(Blade::eWS281X);

        err = parseWS281XLength();
        if (err) return err;
        
        const auto configCommaPos{data.find(',')};
        if (configCommaPos == std::string::npos) {
            return errorMessage(logger, wxTRANSLATE("Missing end comma for WS2811 config"));
        }

        auto configStr{data.substr(0, configCommaPos)};
        data.erase(0, configCommaPos + 1);

        // For the config bitmask, all I care to extract is the color order.
        // I don't support the other options for WS281X and no one uses them.
        for (size idx{0}; idx < eOrder3_Max; ++idx) {
            if (configStr.find(ORDER_STRS[idx]) != std::string::npos) {
                blade.ws281x().colorOrder3_.choose(static_cast<int32>(idx));
                break;
            }
        }

        err = parseWS281XData();
        if (err) return err;

        err = parseWS281XPowerPins();
        if (err) return err;
    } else if (data.starts_with(SIMPLE_STR)) {
        if (splitData.type_ != WS281X::Split::eMax) {
            return errorMessage(logger, wxTRANSLATE("Attempted to SubBlade simple blade"));
        }

        data.erase(0, SIMPLE_STR.length());
        blade.type().choice().choose(Blade::eSimple);

        blade.brightness_.set(firstBrightness);
        auto& simple{blade.simple()};

        const auto ledFromIdx{[&](size idx) -> Simple::LED& {
            switch (idx) {
                case 0: return simple.led1_;
                case 1: return simple.led2_;
                case 2: return simple.led3_;
                case 3: return simple.led4_;
                default:
                    assert(0);
                    __builtin_unreachable();
            }
        }};

        const auto parseProfile{[&](size ledIdx) -> std::optional<std::string> {
            auto& led{ledFromIdx(ledIdx)};

            const auto ledCommaPos{data.find(',')};
            if (ledCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing end comma for SimpleBlade LED %u"), ledIdx);
            }

            const auto ledStr{data.substr(0, ledCommaPos)};
            data.erase(0, ledCommaPos + 1);

            size profileIdx{0};
            for (; profileIdx < eLED_Max; ++profileIdx) {
                const auto& testLedStr{LED_STRS[profileIdx]};
                if (not ledStr.starts_with(testLedStr)) continue;

                led.profile_.choose(static_cast<int32>(profileIdx));

                if (
                        profileIdx >= eLED_Use_Resistance_Start and
                        profileIdx <= eLED_Use_Resistance_End
                   ) {
                    // At least long enough for chevrons
                    if (ledStr.length() < testLedStr.length() + 2) {
                        return errorMessage(logger, wxTRANSLATE("Simple blade which uses resistance missing chevrons: %s"), ledStr);
                    }

                    // Make sure chevrons are there
                    if (ledStr[testLedStr.length()] != '<' or ledStr.back() != '>') {
                        return errorMessage(logger, wxTRANSLATE("Simple blade which uses resistance malformed: %s"), ledStr);
                    }

                    // Parse the in-between
                    auto resistanceStr{ledStr.substr(
                        testLedStr.length() + 1,
                        ledStr.length() - testLedStr.length() - 2
                    )};
                    auto resistanceVal{utils::doStringMath(resistanceStr)};
                    
                    if (not resistanceVal) {
                        return errorMessage(logger, wxTRANSLATE("Simple blade has invalid resistance: %s"), ledStr);
                    }

                    led.resistance_.set(static_cast<int32>(*resistanceVal));
                } else {
                    // If it doesn't use resistance, should match length exactly
                    if (ledStr.length() != testLedStr.length()) {
                        return errorMessage(logger, wxTRANSLATE("Invalid/unrecognized LED for SimpleBlade: %s"), ledStr);
                    }
                }

                break;
            }

            if (profileIdx == eLED_Max) {
                return errorMessage(logger, wxTRANSLATE("Unknown/malformed LED in SimpleBlade: %s"), ledStr);
            }

            return std::nullopt;
        }};

        err = parseProfile(0);
        if (err) return err;
        err = parseProfile(1);
        if (err) return err;
        err = parseProfile(2);
        if (err) return err;
        err = parseProfile(3);
        if (err) return err;

        const auto parsePin{[&](size ledIdx) -> std::optional<std::string> {
            auto& led{ledFromIdx(ledIdx)};

            const auto pinCommaPos{data.find(ledIdx == 3 ? '>' : ',')};
            if (pinCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing end comma/chevron for SimpleBlade power pin %u"), ledIdx + 1);
            }

            auto pinStr{data.substr(0, pinCommaPos)};
            data.erase(0, pinCommaPos + 1);

            if (pinStr != "-1") {
                led.powerPin_.change(std::move(pinStr));
            }

            return std::nullopt;
        }};

        err = parsePin(0);
        if (err) return err;
        err = parsePin(1);
        if (err) return err;
        err = parsePin(2);
        if (err) return err;
        err = parsePin(3);
        if (err) return err;
        
        auto led1Profile{data::context(simple.led1_.profile_)};
        auto led2Profile{data::context(simple.led2_.profile_)};
        auto led3Profile{data::context(simple.led3_.profile_)};
        auto led4Profile{data::context(simple.led4_.profile_)};

        if (
                led1Profile.idx() == eLED_None and
                led2Profile.idx() == eLED_None and
                led3Profile.idx() == eLED_None and
                led4Profile.idx() == eLED_None
           ) {
            blade.type().choice().choose(Blade::eUnassigned);
        }
    } else if (data.starts_with(NULL_STR) or data.starts_with(NULLPTR_STR)) {
        blade.type().choice().unchoose();

        auto blades{data::context(array.blades_)};

        if (blades.children().size() == 1) {
            return errorMessage(logger, wxTRANSLATE("SubBlade with no blade found first in array"));
        }

        auto& blade{dynamic_cast<Blade&>(
            *blades.children()[blades.children().size() - 2]
        )};

        auto type{data::context(blade.type().choice())};
        if (type.idx() != Blade::eWS281X) {
            return errorMessage(logger, wxTRANSLATE("Tried to add SubBlade to a non-WS281X blade"));
        }

        auto splits{data::context(blade.ws281x().splits_)};

        if (splits.children().empty()) {
            return errorMessage(logger, wxTRANSLATE("Tried to add SubBlade to a non-split WS281X blade"));
        }

        auto& lastSplit{dynamic_cast<WS281X::Split&>(
            *splits.children().back()
        )};

        auto lastSplitType{data::context(lastSplit.type_)};
        if (lastSplitType.selected() != splitData.type_) {
            addSplit(blade);
        } else { // this split is same type as last split
            if (
                    lastSplitType.selected() == WS281X::Split::eStandard or
                    lastSplitType.selected() == WS281X::Split::eReverse or
                    lastSplitType.selected() == WS281X::Split::eList
               ) {
                // These types aren't segmented, just add.
                addSplit(blade);
            } else if (lastSplitType.selected() == WS281X::Split::eStride) {
                auto segments{data::context(lastSplit.segments_)};
                auto start{data::context(lastSplit.start_)};
                auto end{data::context(lastSplit.end_)};

                if (
                        // Just make sure this split is same segments
                        // and the start falls inside last split
                        segments.val() != splitData.segments_ or
                        start.val() > splitData.start_ or
                        end.val() < splitData.start_
                   ) {
                    addSplit(blade);
                }
                // Last split is same as this. Nothing to do.
            } else if (lastSplitType.selected() == WS281X::Split::eZig_Zag) {
                auto segments{data::context(lastSplit.segments_)};
                auto start{data::context(lastSplit.start_)};
                auto end{data::context(lastSplit.end_)};

                if (
                        segments.val() != splitData.segments_ or
                        start.val() != splitData.start_ or
                        end.val() != splitData.end_
                   ) {
                    addSplit(blade);
                }
                // Last split is same as this. Nothing to do.
            }
        }
    } else {
        return errorMessage(logger, wxTRANSLATE("Unknown/malformed blade"));
    }

    auto type{data::context(blade.type().choice())};

    if (type.idx() == Blade::eWS281X) {
        if (splitData.type_ == WS281X::Split::eMax) {
            blade.brightness_.set(firstBrightness);
        } else {
            blade.brightness_.set(secondBrightness);
            addSplit(blade);
        }
    }

    return std::nullopt;
}

} // namespace

