#include "presets.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/generate/presets.cpp
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

#include "config/blades/bladeconfig.hpp"
#include "config/blades/servo.hpp"
#include "config/misc/injection.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/presets/style.hpp"
#include "config/priv/io.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

using namespace config;
using namespace config::priv;

namespace {

void genPresets(std::ostream&, const Config&);
void genBlades(std::ostream&, const Config&);

} // namespace

void gen::presets(std::ostream& out, const Config& config) {
    out << "#ifdef CONFIG_PRESETS\n";

    auto injections{data::context(config.injections_)};
    for (const auto& model : injections.children()) {
        auto& injection{dynamic_cast<Injection&>(*model)};

        out << "#include \"" << priv::INJECTION_STR.data() << '/';
        out << injection.filename_ << "\"\n";
    }
    if (not injections.children().empty()) out << '\n';

    genPresets(out, config);
    genBlades(out, config);

    out << "#endif\n";
}

namespace {

void genPresets(std::ostream& out, const Config& config) {
    auto numBlades{data::context(config.numBlades())};

    auto presetArrays{data::context(config.presetArrays_)};
    for (const auto& model : presetArrays.children()) {
        auto& presetArray{dynamic_cast<presets::Array&>(*model)};

        auto arrayName{data::context(presetArray.name_)};
        out << "Preset " << arrayName.val() << "[] = {\n";

        auto presets{data::context(presetArray.presets_)};
        for (const auto& model : presets.children()) {
            auto& preset{dynamic_cast<presets::Preset&>(*model)};

            auto fontDir{data::context(preset.fontDir_)};
            auto track{data::context(preset.track_)};
            out << "\t{ \"" << fontDir.val() << "\", \""
                << track.val() << "\",\n";

            auto styles{data::context(preset.styles_)};
            for (auto idx{0}; idx < numBlades.val(); ++idx) {
                const auto& model{styles.children()[idx]};
                auto& style{dynamic_cast<presets::Style&>(*model)};

                std::string line;

                auto comment{data::context(style.comment_)};
                auto commentStr{comment.val()};

                utils::trimSurroundingWhitespace(commentStr);

                if (not commentStr.empty()) {
                    std::istringstream commentStream{commentStr};
                    out << "\t\t/*\n";
                    while (std::getline(commentStream, line)) {
                        out << "\t\t * " << line << '\n';
                    }
                    out << "\t\t */\n";
                }

                auto styleStr{style.format(true)};
                std::istringstream styleStream(styleStr);
                while (std::getline(styleStream, line)) {
                    out << "\t\t" << line;

                    if (styleStream.eof())
                        out << ",\n";
                    else
                        out << '\n';
                }
            }

            auto presetName{data::context(preset.name_)};
            out << "\t\t\"" << presetName.val() << "\"\n\t},\n";
        }
        out << "};\n";
    }
}

void genBlades(std::ostream& out, const Config& config) {
    out << "BladeConfig blades[] = {\n";

    auto bladeConfigs{data::context(config.bladeConfigs_)};
    for (const auto& model : bladeConfigs.children()) {
        auto& bladeConfig{dynamic_cast<blades::BladeConfig&>(*model)};

        if (data::context(bladeConfig.noBladeId_).val()) {
            out << "\t{ NO_BLADE,\n";
        } else {
            auto id{data::context(bladeConfig.id_)};
            out << "\t{ " << id.val() << ",\n";
        }

        auto blades{data::context(bladeConfig.blades_)};
        for (const auto& model : blades.children()) {
            auto& blade{dynamic_cast<blades::Blade&>(*model)};

            auto type{data::context(blade.type())};

            if (type.choiceIdx() == blades::Blade::eUnassigned) {
                out << "\t\tSimpleBladePtr<NoLED, NoLED, NoLED, NoLED, -1, -1, -1, -1>(),\n";
                continue;
            }

            if (type.choiceIdx() == blades::Blade::eServo) {
                auto& servo{*type.selected<blades::Servo>()};
                auto pin{data::context(servo.sigPin_)};

                out << "\t\tServoBladePtr<" << pin.val() << ">(),\n";
                continue;
            }

            std::stringstream bladeStr;
            auto brightness{data::context(blade.brightness_)};
            if (brightness.val() != 100) {
                bladeStr << "DimBlade(" << brightness.val() << ", ";
            }

            if (type.choiceIdx() == blades::Blade::eWS281X) {
                auto& ws281x{blade.ws281x()};

                auto length{data::context(ws281x.length_)};
                auto dataPin{data::context(ws281x.dataPin_)};

                bladeStr << "WS281XBladePtr<" << length.val() << ", " << dataPin.val();
                bladeStr << ", Color8::";

                if (data::context(ws281x.hasWhite_).val()) {
                    auto order4{data::context(ws281x.colorOrder4_)};
                    bool whiteFirst{
                        order4.idx() >= eOrder4_White_First_Start and
                        order4.idx() <= eOrder4_White_First_End
                    };

                    if (whiteFirst) {
                        if (data::context(ws281x.useRgbWithWhite_).val()) {
                            bladeStr << 'W';
                        } else bladeStr << 'w';

                        auto strIdx{order4.idx() - eOrder4_White_First_Start};
                        bladeStr << ORDER_STRS[strIdx];
                    } else {
                        bladeStr << ORDER_STRS[order4.idx()];

                        if (data::context(ws281x.useRgbWithWhite_).val()) {
                            bladeStr << 'W';
                        } else bladeStr << 'w';
                    }
                } else {
                    auto order3{data::context(ws281x.colorOrder3_)};
                    bladeStr << ORDER_STRS[order3.idx()];
                }

                bladeStr << ", " << POWER_PINS_STR;

                // Generate a list of selected first and then use that to
                // generate the actual output so that it's known when on the
                // last item for ", "
                std::vector<std::string> selected;

                auto powerPins{data::context(ws281x.powerPins_)};
                for (size idx{0}; idx < powerPins.items().size(); ++idx) {
                    if (not powerPins.selected()[idx]) continue;

                    selected.push_back(powerPins.items()[idx]);
                }

                for (size idx{0}; idx < selected.size(); ++idx) {
                    bladeStr << selected[idx];

                    if (idx + 1 < selected.size())
                        bladeStr << ", ";
                }

                bladeStr << ">>()";
            } else if (type.choiceIdx() == blades::Blade::eSimple) {
                auto& simple{blade.simple()};

                bladeStr << "SimpleBladePtr<";

                auto outputLEDProfile{[&bladeStr](blades::Simple::LED& led) {
                    auto profile{data::context(led.profile_)};

                    bladeStr << LED_STRS[profile.idx()];

                    auto resistance{data::context(led.resistance_)};
                    if (resistance.enabled()) {
                        bladeStr << '<' << resistance.val() << '>';
                    }

                    bladeStr << ", ";
                }};
                outputLEDProfile(simple.led1_);
                outputLEDProfile(simple.led2_);
                outputLEDProfile(simple.led3_);
                outputLEDProfile(simple.led4_);

                auto outputLEDPower{[&bladeStr](blades::Simple::LED& led) {
                    auto powerPin{data::context(led.powerPin_)};
                    
                    if (powerPin.enabled()) {
                        bladeStr << powerPin.val();
                    } else bladeStr << "-1";
                }};
                outputLEDPower(simple.led1_);
                bladeStr << ", ";
                outputLEDPower(simple.led2_);
                bladeStr << ", ";
                outputLEDPower(simple.led3_);
                bladeStr << ", ";
                outputLEDPower(simple.led4_);

                bladeStr << ">()";
            }

            if (brightness.val() != 100) bladeStr << ')';

            auto splits{data::context(blade.ws281x().splits_)};

            if (
                    type.choiceIdx() == blades::Blade::eSimple or
                    (type.choiceIdx() == blades::Blade::eWS281X and
                     splits.children().empty())
               ) {
                out << "\t\t" << bladeStr.str() << ",\n";
                continue;
            }

            for (const auto& model : splits.children()) {
                auto& split{dynamic_cast<blades::WS281X::Split&>(*model)};

                auto type{data::context(split.type_)};

                using enum blades::WS281X::Split::Type;

                if (
                        type.selected() == eStandard or
                        type.selected() == eReverse or
                        type.selected() == eList
                   ) {
                    out << "\t\t";

                    auto brightness{data::context(split.brightness_)};
                    if (brightness.val() != 100) {
                        out << "DimBlade(" << brightness.val() << ", ";
                    }

                    if (type.selected() == eStandard) {
                        auto start{data::context(split.start_)};
                        auto end{data::context(split.end_)};

                        out << "SubBlade(" << start.val() << ", "
                            << end.val() << ", ";
                    } else if (type.selected() == eReverse) {
                        auto start{data::context(split.start_)};
                        auto end{data::context(split.end_)};

                        out << "SubBladeReverse(" << start.val() << ", "
                            << end.val() << ", ";
                    } else if (type.selected() == eList) {
                        auto list{data::context(split.list_)};

                        auto listStr{list.val()};
                        if (listStr.back() == ',') listStr.pop_back();

                        out << "SubBladeWithList<" << listStr << ">(";
                    }

                    
                    if (&split == &*splits.children().front()) {
                        out << bladeStr.str() << ')';                    
                    } else out << "nullptr)";

                    if (brightness.val() != 100) out << ')';

                    out << ",\n";
                }

                if (
                        type.selected() == eStride or
                        type.selected() == eZig_Zag
                   ) {
                    auto segments{data::context(split.segments_)};

                    for (auto idx{0}; idx < segments.val(); ++idx) {
                        out << "\t\t";

                        auto brightness{data::context(split.brightness_)};
                        if (brightness.val() != 100) {
                            out << "DimBlade(" << brightness.val() << ", ";
                        }

                        auto start{data::context(split.start_)};
                        auto end{data::context(split.end_)};

                        if (type.selected() == eStride) {
                            const auto endVal{
                                end.val() - (segments.val() - idx - 1)
                            };

                            out << "SubBladeWithStride(";
                            out << start.val() + idx << ", ";
                            out << endVal << ", ";
                            out << segments.val() << ", ";
                        } else if (type.selected() == eZig_Zag) {
                            out << "SubBladeZZ(";
                            out << start.val() << ", " ;
                            out << end.val() << ", ";
                            out << segments.val() << ", " << idx << ", ";
                        }

                        if (idx == 0 and &split == &*splits.children().front()) {
                            out << bladeStr.str() << ')';                    
                        } else out << "nullptr)";

                        if (brightness.val() != 100) out << ')';

                        out << ",\n";
                    }
                }
            }
        }

        auto arrayChoice{data::context(bladeConfig.presetArray_.choice())};
        auto presetArrays{data::context(config.presetArrays_)};
        const auto& arrayModel{presetArrays.children()[arrayChoice.idx()]};
        auto& presetArray{dynamic_cast<presets::Array&>(*arrayModel)};
        auto presetArrayName{data::context(presetArray.name_)};
        
        out << "\t\tCONFIGARRAY(" << presetArrayName.val() << ")";

        auto bladeConfigName{data::context(bladeConfig.name_)};
        if (not bladeConfigName.val().empty()) {
            out << ", \"";
            out << bladeConfigName.val();
            out << '\"';
        }

        out << "\n\t},\n";
    }
    out << "};\n";
}

} // namespace

