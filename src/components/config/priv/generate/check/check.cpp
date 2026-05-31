#include "check.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/generate/check/check.cpp
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

#include <optional>

#include "config/blades/bladeconfig.hpp"
#include "config/blades/servo.hpp"
#include "config/buttons/button.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "config/priv/io.hpp"
#include "config/strings.hpp"
#include "config/styles/style.hpp"
#include "data/context.hpp"

using namespace config;
using namespace config::priv;
using namespace config::blades;

namespace {

std::optional<std::string> getStyleImbalance(const std::string&);

} // namespace

std::optional<std::string> gen::preCheck(
    const Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::gen::preCheck()")};

    // Right now, the only way to save is in `.h`, but serializing to pconf
    // would remove this restriction.
    //
    // It's a semi-arbitrary restriction, but there might be data loss and
    // things like this.
    if (config.os() == nullptr) {
        return errorMessage(logger, wxTRANSLATE("Config must have an OS Version selected."));
    }

    // Similar to os above
    if (config.board() == nullptr) {
        return errorMessage(logger, wxTRANSLATE("Config must have a board selected."));
    }

    const auto& settings{config.settings_};
    const auto& awareness{config.settings_.bladeAwareness_};

    auto bladeDetectEnable{data::context(awareness.bladeDetect_.enable_)};
    auto bladeDetectPin{data::context(awareness.bladeDetect_.pin_)};
    if (bladeDetectEnable.val() and bladeDetectPin.val().empty()) {
        return errorMessage(logger, wxTRANSLATE("Blade Detect Pin cannot be empty."));
    }

    auto bladeIDEnable{data::context(awareness.bladeId_.enable_)};
    auto bladeIDPin{data::context(awareness.bladeId_.pin_)};
    if (bladeIDEnable.val()) {
        if (bladeIDPin.val().empty()) {
            return errorMessage(logger, wxTRANSLATE("Blade ID Pin cannot be empty."));
        }

        auto mode{data::context(awareness.bladeId_.mode_)};
        auto bridgePin{data::context(awareness.bladeId_.bridgePin_)};
        if (mode.idx() == eBIDMode_Bridged and bridgePin.val().empty()) {
            return errorMessage(logger, wxTRANSLATE("Pullup Pin cannot be empty."));
        }
    }

    if (
            bladeIDEnable.val() and
            bladeDetectEnable.val() and
            bladeDetectPin.val() == bladeIDPin.val()
       ) {
        return errorMessage(logger, wxTRANSLATE("Blade ID Pin and Blade Detect Pin cannot be the same."));
    }

    auto bladeConfigs{data::context(config.bladeConfigs_)};
    for (const auto& model : bladeConfigs.children()) {
        auto& bladeConfig{dynamic_cast<BladeConfig&>(*model)};

        auto issues{data::context(bladeConfig.issues())};
        if (issues.val() != BladeConfig::eIssue_None) {
            auto name{data::context(bladeConfig.name_)};
            const auto arrayName{name.val().empty()
                ? _("[default]")
                : name.val()
            };
            return errorMessage(logger, wxTRANSLATE("Blade array %s has issues, and isn't ready yet."), arrayName);
        }
    }

    for (
            auto arrayIdx{0};
            arrayIdx < bladeConfigs.children().size();
            ++arrayIdx
        ) {
        const auto& model{bladeConfigs.children()[arrayIdx]};
        auto& bladeConfig{dynamic_cast<BladeConfig&>(*model)};

        auto name{data::context(bladeConfig.name_)};
        auto presetArray{data::context(bladeConfig.presetArray_)};

        const auto arrayName{name.val().empty()
            ? _("[default]")
            : name.val()
        };

        if (presetArray.choiceIdx() == -1) {
            return errorMessage(logger, wxTRANSLATE("Blade array %s has no preset array selection"), arrayName);
        }

        auto blades{data::context(bladeConfig.blades_)};

        for (
                auto bladeIdx{0};
                bladeIdx < blades.children().size();
                ++bladeIdx
            ) {
            const auto& model{blades.children()[bladeIdx]};
            auto& blade{dynamic_cast<Blade&>(*model)};

            auto type{data::context(blade.type())};

            if (type.choiceIdx() == Blade::eWS281X) {
                auto dataPin{data::context(blade.ws281x().dataPin_)};

                if (dataPin.val().empty()) {
                    return errorMessage(logger, wxTRANSLATE("Blade %d in array %s missing data pin"), bladeIdx, arrayName);
                }
                // Subblade overlap
            }

            if (type.choiceIdx() == Blade::eSimple) {
                auto& led1{blade.simple().led1_};
                auto& led2{blade.simple().led2_};
                auto& led3{blade.simple().led3_};
                auto& led4{blade.simple().led4_};

                auto led1Profile{data::context(led1.profile_)};
                auto led2Profile{data::context(led2.profile_)};
                auto led3Profile{data::context(led3.profile_)};
                auto led4Profile{data::context(led4.profile_)};

                auto led1Pin{data::context(led1.powerPin_)};
                auto led2Pin{data::context(led2.powerPin_)};
                auto led3Pin{data::context(led3.powerPin_)};
                auto led4Pin{data::context(led4.powerPin_)};

                constexpr auto SIMPLE_ERR_MSG{wxTRANSLATE("LED %d of blade %d in array %s missing power pin.")};
                if (led1Profile.idx() != eLED_None and led1Pin.val().empty()) {
                    return errorMessage(logger, SIMPLE_ERR_MSG, 1, bladeIdx, arrayName);
                }
                if (led2Profile.idx() != eLED_None and led2Pin.val().empty()) {
                    return errorMessage(logger, SIMPLE_ERR_MSG, 2, bladeIdx, arrayName);
                }
                if (led3Profile.idx() != eLED_None and led3Pin.val().empty()) {
                    return errorMessage(logger, SIMPLE_ERR_MSG, 3, bladeIdx, arrayName);
                }
                if (led4Profile.idx() != eLED_None and led4Pin.val().empty()) {
                    return errorMessage(logger, SIMPLE_ERR_MSG, 4, bladeIdx, arrayName);
                }

                if (
                        led1Profile.idx() == eLED_None or
                        led1Profile.idx() == eLED_None or
                        led1Profile.idx() == eLED_None or
                        led1Profile.idx() == eLED_None
                   ) {
                    return errorMessage(logger, wxTRANSLATE("Blade %d in array %s has no LEDs"), bladeIdx, arrayName);
                }
            }

            if (type.choiceIdx() == Blade::eServo) {
                auto& servo{*type.selected<Servo>()};
                auto sigPin{data::context(servo.sigPin_)};

                if (sigPin.val().empty())
                    return errorMessage(logger, wxTRANSLATE("Blade %d in array %s missing signal pin"), bladeIdx, arrayName);
            }
        }
    }

    auto presetArrays{data::context(config.presetArrays_)};

    for (const auto& model : presetArrays.children()) {
        auto& presetArray{dynamic_cast<presets::Array&>(*model)};

        auto arrayName{data::context(presetArray.name_)};
        if (arrayName.val().empty()) {
            return errorMessage(logger, wxTRANSLATE("Preset array has no name"));
        }

        for (const auto& model : presetArrays.children()) {
            auto& checkArray{dynamic_cast<presets::Array&>(*model)};

            if (&checkArray == &presetArray) continue;

            auto checkName{data::context(checkArray.name_)};
            if (checkName.val() == arrayName.val()) {
                return errorMessage(logger, wxTRANSLATE("Duplicate preset arrays %s"), arrayName.val());
            }
        }
    }

    constexpr cstring STYLE_ERR_STR{wxTRANSLATE("Bladestyle %d in preset %d (%s) in array %s has mismatched %s")};
    for (const auto& model : presetArrays.children()) {
        auto& presetArray{dynamic_cast<presets::Array&>(*model)};

        auto arrayName{data::context(presetArray.name_)};
        auto presets{data::context(presetArray.presets_)};

        for (
                auto presetIdx{0};
                presetIdx < presets.children().size();
                ++presetIdx
            ) {
            const auto& model{presets.children()[presetIdx]};
            auto& preset{dynamic_cast<presets::Preset&>(*model)};

            auto presetName{data::context(preset.name_)};
            auto styles{data::context(preset.styles_)};

            for (
                    auto styleIdx{0};
                    styleIdx < styles.children().size();
                    ++styleIdx
                ) {
                const auto& model{styles.children()[styleIdx]};
                auto& style{dynamic_cast<presets::Style&>(*model)};

                auto content{data::context(style.content_)};
                if (auto imbalance{getStyleImbalance(content.val())}) {
                    return errorMessage(
                        logger, STYLE_ERR_STR, styleIdx, presetIdx,
                        presetName.val(), arrayName.val(),
                        *imbalance
                    );
                }
            }
        }
    }

    auto styles{data::context(config.styles_)};
    constexpr cstring ALIAS_ERR_STR{wxTRANSLATE("Style Alias %s has mismatched %s")};
    for (const auto& model : styles.children()) {
        auto& style{dynamic_cast<styles::Style&>(*model)};

        auto name{data::context(style.name_)};

        auto content{data::context(style.content_)};
        if (auto imbalance{getStyleImbalance(content.val())}) {
            return errorMessage(
                logger, STYLE_ERR_STR, name.val(), *imbalance
            );
        }
    }

    auto buttons{data::context(config.buttons_)};
    for (auto idx{0}; idx < buttons.children().size(); ++idx) {
        auto& model{*buttons.children()[idx]};
        auto& button{dynamic_cast<buttons::Button&>(model)};

        auto type{data::context(button.type_)};
        auto event{data::context(button.event_)};
        auto pin{data::context(button.pin_)};

        if (type.idx() == -1) {
            return errorMessage(logger, wxTRANSLATE("Button %u doesn't have a type set."), idx);
        }
        if (event.idx() == -1) {
            return errorMessage(logger, wxTRANSLATE("Button %u doesn't have its event set."), idx);
        }
        if (pin.val().empty()) {
            return errorMessage(logger, wxTRANSLATE("Button %u doesn't have a pin set."), idx);
        }
    }

    return std::nullopt;
}

namespace {

std::optional<std::string> getStyleImbalance(const std::string& style) {
    std::vector<char> depth;

    for (const char chr : style) {
        if (chr == '<' or chr == '(') {
            depth.push_back(chr);
            continue;
        }
        if (chr == '>' or chr == ')') {
            if (depth.empty())
                return std::string{chr};

            if (
                    (chr == '>' and depth.back() != '<') or
                    (chr == ')' and depth.back() != '(')
               ) return std::string{depth.back(), chr};

            depth.pop_back();
            continue;
        }
        if (chr == '"') {
            if (not depth.empty() and depth.back() == '"') depth.pop_back();
            else depth.push_back(chr);
        }
    }

    if (not depth.empty())
        return std::string{depth.back()};

    return std::nullopt;
}

} // namespace

