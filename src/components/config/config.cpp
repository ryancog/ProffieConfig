#include "config.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/config/config.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include <filesystem>
#include <mutex>

#include "config/blades/bladeconfig.hpp"
#include "config/blades/simple.hpp"
#include "config/blades/ws281x.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/priv/data.hpp"
#include "config/priv/generate/generate.hpp"
#include "config/priv/io.hpp"
#include "config/priv/parse/parse.hpp"
#include "log/context.hpp"
#include "log/severity.hpp"
#include "utils/files.hpp"
#include "utils/types.hpp"
#include "utils/paths.hpp"

namespace {

fs::path savePath(const std::string&);

constexpr uint64 SETTINGS_ID{0};
constexpr uint64 PROPS_ID{1};
constexpr uint64 PROPSEL_ID{2};
constexpr uint64 PRESET_ARRAYS_ID{3};
constexpr uint64 BLADE_CONFIGS_ID{4};
constexpr uint64 BUTTONS_ID{5};

} // namespace

data::Vector config::list;

struct config::Config::SavedReceiver : Root::Receiver {
    SavedReceiver(Config& cfg) : cfg_{cfg} {
        Root::Receiver::attach(cfg_);
    }

    void onActionIdx(size idx) override {
        data::Bool::Context{cfg_.isSaved_}.set(idx == cfg_.mSavedAction);
    }

    void onActionClear(size lastIdx) override {
        if (lastIdx == cfg_.mSavedAction) {
            cfg_.mSavedAction = Root::ACT_IDX_FIRST;
        }
    }

    Config& cfg_;
};

config::Config::Config() :
    settings_{*this},
    props_{this},
    propSel_{this},
    presetArrays_{this},
    bladeConfigs_{this},
    buttons_{this} {

    data::Vector::Context props{props_};
    for (auto& prop : priv::propGenerator(&props_)) {
        props.add(std::move(prop));
    }

    const auto propSelFilt{[](const data::Choice::Context& ctxt, int32& idx) {
        if (idx == -1 and ctxt.numChoices()) idx = 0;
    }};
    propSel_.choice_.setFilter(propSelFilt);

    mRcvr = new SavedReceiver(*this);
}

config::Config::~Config() {
    delete mRcvr;
}

bool config::Config::enumerate(const EnumFunc& func) {
    if (func(settings_, SETTINGS_ID, {})) return true;
    if (func(props_, PROPS_ID, {})) return true;
    if (func(propSel_, PROPSEL_ID, {})) return true;
    if (func(presetArrays_, PRESET_ARRAYS_ID, {})) return true;
    if (func(bladeConfigs_, BLADE_CONFIGS_ID, {})) return true;
    if (func(buttons_, BUTTONS_ID, {})) return true;
    return false;
}

data::Model *config::Config::find(uint64 id) {
    if (id == SETTINGS_ID) return &settings_;
    if (id == PROPS_ID) return &props_;
    if (id == PROPSEL_ID) return &propSel_;
    if (id == PRESET_ARRAYS_ID) return &presetArrays_;
    if (id == BLADE_CONFIGS_ID) return &bladeConfigs_;
    if (id == BUTTONS_ID) return &buttons_;
    return nullptr;
}

size config::Config::numBlades() {
    data::Vector::Context bladeConfigs{bladeConfigs_};

    size num{0};
    for (const auto& model : bladeConfigs.children()) {
        auto& bladeConfig{static_cast<blades::BladeConfig&>(*model)};

        size sum{0};

        data::Vector::Context blades{bladeConfig.blades_};
        for (const auto& model : blades.children()) {
            auto& blade{static_cast<blades::Blade&>(*model)};

            data::Choice::Context typeChoice{blade.type_.choice_};
            data::Vector::Context types{blade.types_};
            auto *typeModel{types.children()[typeChoice.choice()].get()};

            if (auto *ptr = dynamic_cast<blades::WS281X *>(typeModel)) {
                data::Vector::Context splits{ptr->splits_};

                if (splits.children().size() == 0) {
                    ++sum;
                    continue;
                }

                for (const auto& model : splits.children()) {
                    auto& split{static_cast<blades::WS281X::Split&>(*model)};

                    const auto type{static_cast<blades::WS281X::Split::Type>(
                        split.type_.selected()
                    )};
                    switch (type) {
                        using enum blades::WS281X::Split::Type;
                        case eStandard:
                        case eReverse:
                        case eList:
                            ++sum;
                            break;
                        case eStride:
                        case eZig_Zag:
                            sum += data::Integer::Context{
                                split.segments_
                            }.val();
                            break;
                        default:
                            assert(0);
                            __builtin_unreachable();
                    }
                }

                continue;
            } 

            if (auto *ptr = dynamic_cast<blades::Simple *>(typeModel)) {
                ++sum;
                continue;
            }
        }
    }

    return num;
}

void config::Config::syncStyles() {
    std::lock_guard scopeLock{pLock};

    const auto numBlades{this->numBlades()};
    data::Vector::Context presetArrays{presetArrays_};

    for (const auto& model : presetArrays.children()) {
        auto& arrayModel{static_cast<presets::Array&>(*model)};
        data::Vector::Context presets{arrayModel.presets_};

        for (const auto& model : presets.children()) {
            auto& presetModel{static_cast<presets::Preset&>(*model)};
            data::Vector::Context styles{presetModel.styles_};

            const auto newSize{std::max<uint32>(
                styles.children().size(), numBlades
            )};

            while (styles.children().size() < newSize) {
                styles.insert(
                    styles.children().size(),
                    std::make_unique<presets::Style>(
                        &styles.model<data::Vector>()
                    )
                );
            }
        }
    }
}

config::Info::Info() = default;

fs::path config::Info::path() {
    return ::savePath(data::String::Context{name_}.val());
}

std::optional<std::string> config::Info::save(
    const fs::path& path, logging::Branch *lBranch
) {
    std::lock_guard scopeLock{pLock};
    auto& logger{logging::Branch::optCreateLogger("config::Info::save()", lBranch)};

    if (not mConfig) {
        return priv::errorMessage(logger, wxTRANSLATE("Config not loaded"));
    }

    // Create context to lock.
    Config::Context ctxt{*mConfig};
    data::String::Context name{name_};

    std::optional<std::string> err;
    std::error_code errCode;
    if (not path.empty()) {
        err = priv::generate(
            path,
            *mConfig,
            logger.binfo("Exporting \"" + name.val() + "\" to \"" + path.string() + "\"...")
        );
        if (err) fs::remove(path, errCode);
        return err;
    } 

    const auto finalPath{this->path()};
    const fs::path tmpPath{finalPath.string() + ".tmp"};
    err = priv::generate(
        tmpPath,
        *mConfig,
        logger.binfo("Saving \"" + name.val() + "\"...")
    );
    if (err) {
        fs::remove(tmpPath, errCode);
        return err;
    }

    if (not files::copyOverwrite(tmpPath, finalPath, errCode)) {
        err = priv::errorMessage(logger, wxTRANSLATE("Failed to move temp file: %s"), tmpPath.string(), finalPath.string(), errCode.message());
    }
    fs::remove(tmpPath, errCode);

    mConfig->mSavedAction = ctxt.actionIndex();

    return err;
}

std::optional<std::string> config::Info::load() {
    std::lock_guard scopeLock{pLock};
    auto& logger{logging::Context::getGlobal().createLogger("config::Info::load()")};

    if (mConfig) return std::nullopt;

    mConfig.reset(new Config);

    const auto cfgPath{path()};
    if (fs::exists(cfgPath)) { 
        auto err{priv::parse(
            cfgPath,
            *mConfig,
            logger.binfo("Config (" + cfgPath.string() + ") exists, parsing...")
        )};
        if (err) return err;
    } else {
        logger.warn("Config (" + cfgPath.string() + ") does not exist, creating new...");
    }

    return std::nullopt;
}

void config::Info::unload() {
    data::Vector::Context list{config::list};

    data::String::Context name{name_};

    bool found{false};
    std::error_code err;
    for (const auto& entry : fs::directory_iterator{paths::configDir(), err}) {
        if (not entry.is_regular_file()) continue;
        if (entry.path().extension() != RAW_FILE_EXTENSION) continue;

        if (name.val() == entry.path().stem().string()) {
            found = true;
            break;
        }
    }

    if (not found) {
        size idx{0};
        for (; idx < list.children().size(); ++idx) {
            if (&*list.children()[idx] == this) break;
        }
        list.remove(idx);
    }
}

auto config::Info::config() -> const std::unique_ptr<Config>& {
    return mConfig;
}

void config::update() {
    data::Vector::Context list{config::list};

    std::vector<std::string> files;

    std::error_code err;
    for (const auto& entry : fs::directory_iterator{paths::configDir(), err}) {
        if (not entry.is_regular_file()) continue;
        if (entry.path().extension() != RAW_FILE_EXTENSION) continue;

        files.emplace_back(entry.path().stem().string());
    }

    for (size idx{0}; idx < list.children().size(); ++idx) {
        auto& info{static_cast<Info&>(*list.children()[idx])};

        data::String::Context name{info.name_};

        bool found{false};
        for (const auto& file : files) {
            if (file == name.val()) {
                found = true;
                break;
            }
        }

        if (not found) {
            list.remove(idx);
            --idx;
        }
    }

    for (const auto& file : files) {
        bool found{false};
        for (size idx{0}; idx < list.children().size(); ++idx) {
            auto& info{static_cast<Info&>(*list.children()[idx])};

            data::String::Context name{info.name_};
            if (name.val() == file) {
                found = true;
                break;
            }
        }

        if (found) continue;

        auto info{std::unique_ptr<Info>{new Info}};
        data::String::Context{info->name_}.change(std::string{file}, 0);
        list.insert(list.children().size(), std::move(info));
    }

    if (err) {
        logging::Context::getGlobal().quickLog(
            logging::Severity::Err,
            "config::update()",
            "Failed to get path list: " + err.message()
        );
    }
}

bool config::remove(Info& info) {
    data::Vector::Context vec{list};

    size idx{0};
    for (;idx < vec.children().size(); ++idx) {
        if (&static_cast<Info&>(*vec.children()[idx]) == &info) {
            break;
        }
    }
    if (idx == vec.children().size()) return false;

    std::error_code err;
    auto res{fs::remove(info.path(), err)};
    if (not res) return false;

    vec.remove(idx);
    return true;
}

std::optional<std::string> config::import(
    const std::string& name, const fs::path& path
) {
    auto& logger{logging::Context::getGlobal().createLogger("config::import()")};

    data::Vector::Context vec{list};

    for (size idx{0}; idx < vec.children().size(); ++idx) {
        auto& info{static_cast<Info&>(*vec.children()[idx])};
        if (data::String::Context{info.name_}.val() == name) {
            return priv::errorMessage(logger, wxTRANSLATE("Config with name already open"));
        }
    }

    std::error_code err;
    if (not files::copyOverwrite(path, savePath(name), err)) {
        return priv::errorMessage(logger, wxTRANSLATE("Could not copy in config file: %s"), err.message());
    }

    update();

    return std::nullopt;
}

namespace {

fs::path savePath(const std::string& name) {
    return 
        paths::configDir() /
        (static_cast<std::string>(name) + config::RAW_FILE_EXTENSION);
}

} // namespace

