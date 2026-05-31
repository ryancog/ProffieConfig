#include "config.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/config/config.cpp
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

#include <filesystem>
#include <mutex>

#include "config/blades/servo.hpp"
#include "config/presets/style.hpp"
#include "config/blades/bladeconfig.hpp"
#include "config/blades/simple.hpp"
#include "config/blades/ws281x.hpp"
#include "config/presets/array.hpp"
#include "config/presets/preset.hpp"
#include "config/priv/data.hpp"
#include "config/priv/io.hpp"
#include "config/settings/define.hpp"
#include "data/context.hpp"
#include "log/context.hpp"
#include "log/severity.hpp"
#include "utils/files.hpp"
#include "utils/types.hpp"
#include "utils/paths.hpp"
#include "versions/os.hpp"

using namespace config;

namespace {

fs::path savePath(const std::string&);

} // namespace

Config::Config() :
    settings_(*this),
    presetArrays_(*this),
    bladeConfigs_(*this),
    buttons_(*this),
    injections_(*this),
    styles_(*this),
    mOsChoice(*this),
    mPropChoice(*this),
    mBoardChoice(*this) {
    CreationScope createScope(this);

    { 
        auto osCtxt{data::context(versions::os::list())};

        mOsVec.reserve(osCtxt.children().size());
        for (const auto& model : osCtxt.children()) {
            auto& os{dynamic_cast<versions::os::OS&>(*model)};
            mOsVec.emplace_back(new versions::os::OS(os));

            auto& propVec{mPropMap[os.version_]};
            propVec = versions::props::forVersion(
                os.version_, *this, &Config::processPropRecommend
            );
        }
    }

    static const auto selfTable{[] {
        data::hier::Root::RecvTable table;
        table.onAction_ = data::map(&Config::onAction);
        return table;
    }()};
    observeWith(*this, selfTable);

    static const auto numBladesTable{[] {
        data::base::Integer::RecvTable table;
        table.onSet_ = data::map(&Config::onNumBlades);
        return table;
    }()};
    observeWith(mNumBlades, numBladesTable);

    static const auto osChoiceTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_ = data::map(&Config::onOSChoice);
        return table;
    }()};
    respondWith(mOsChoice, osChoiceTable);
    mOsChoice.update(mOsVec.size());

    // Set the default OS, if there is one, for parsing the board if the config
    // doesn't have a OS ProffieConfig option, and as a convenience for new
    // configs created by the user.
    { auto ctxt{data::context(mOsChoice)};
        if (ctxt.num() > 0)
            ctxt.choose(0);
    }

    activate();
}

Config::~Config() {
    deactivate();
}

void Config::onActivate() {
    onOSChoice();
}

auto Config::coreChildren() const -> std::vector<const Model *> {
    return {
        &settings_,
        &mPropChoice,
        &mOsChoice,
        &mBoardChoice,
        &presetArrays_,
        &bladeConfigs_,
        &buttons_,
        &injections_,
        &styles_,
    };
}

auto Config::children() const -> std::vector<const Model *> {
    auto ret{coreChildren()};

    for (const auto& [ver, vec] : mPropMap) {
        for (const auto& prop : vec) {
            ret.push_back(prop.get());
        }
    }

    return ret;
}

auto Config::childrenToHash() const -> std::vector<const Model *> {
    auto ret{coreChildren()};

    if (const auto *ptr{prop()})
        ret.push_back(ptr);

    return ret;
}

std::span<const std::unique_ptr<versions::os::OS>> Config::osVec() const {
    return mOsVec;
}

data::hier::Choice& Config::osChoice() {
    return mOsChoice;
}

const data::hier::Choice& Config::osChoice() const {
    return mOsChoice;
}

const versions::os::OS *Config::os() const {
    auto ctxt{data::context(mOsChoice)};
    if (ctxt.idx() == -1) return nullptr;
    return mOsVec[ctxt.idx()].get();
}

data::base::Choice& Config::boardChoice() {
    return mBoardChoice;
}

const data::hier::Choice& Config::boardChoice() const {
    return mBoardChoice;
}

const versions::os::Board *Config::board() const {
    const auto *os{this->os()};
    if (os == nullptr) return nullptr;

    auto ctxt{data::context(mBoardChoice)};
    if (ctxt.idx() == -1) return nullptr;

    auto iter{std::next(os->boards_.begin(), ctxt.idx())};
    return &iter->second;
}

std::optional<std::span<const std::unique_ptr<versions::props::Prop>>>
Config::propVec() const {
    const auto *os{this->os()};
    if (os == nullptr) return std::nullopt;

    return mPropMap.find(os->version_)->second;
}

data::hier::Choice& Config::propChoice() {
    return mPropChoice;
}

const data::hier::Choice& Config::propChoice() const {
    return mPropChoice;
}

const versions::props::Prop *Config::prop() const {
    auto propVec{this->propVec()};
    if (not propVec) return nullptr;

    auto ctxt{data::context(mPropChoice)};
    if (ctxt.idx() == -1) return nullptr;

    return (*propVec)[ctxt.idx()].get();
}

const data::prim::Bool& Config::isSaved() const {
    return mIsSaved;
}

const data::prim::Integer& Config::numBlades() const {
    return mNumBlades;
}

void Config::calcNumBlades() {
    auto bladeConfigs{data::context(bladeConfigs_)};

    size num{0};
    for (const auto& model : bladeConfigs.children()) {
        auto& bladeConfig{dynamic_cast<blades::BladeConfig&>(*model)};

        size sum{0};

        auto blades{data::context(bladeConfig.blades_)};
        for (const auto& model : blades.children()) {
            auto& blade{dynamic_cast<blades::Blade&>(*model)};

            auto typeChoice{data::context(blade.type().choice())};

            // This can be set to -1 via unchoose() when a blade is nullptr
            // during parsing w/ subblades, as that's how the blade is deemed
            // unnecessary.
            if (typeChoice.idx() == -1)
                continue;

            auto types{data::context(blade.types())};
            auto *typeModel{types.children()[typeChoice.idx()].get()};

            if (auto *ptr{dynamic_cast<blades::WS281X *>(typeModel)}) {
                auto splits{data::context(ptr->splits_)};

                if (splits.children().size() == 0) {
                    ++sum;
                    continue;
                }

                for (const auto& model : splits.children()) {
                    auto& split{dynamic_cast<blades::WS281X::Split&>(*model)};
                    auto splitType{data::context(split.type_)};

                    const auto type{static_cast<blades::WS281X::Split::Type>(
                        splitType.selected()
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
                            sum += data::context(split.segments_).val();
                            break;
                        default:
                            assert(0);
                            __builtin_unreachable();
                    }
                }
            } else {
                // Simple, Servo, or Unassigned
                ++sum;
            }
        }

        num = std::max(num, sum);
    }

    mNumBlades.set(static_cast<int32>(num));
}

void Config::syncStyles() {
    auto numBlades{data::context(mNumBlades)};
    auto presetArrays{data::context(presetArrays_)};

    for (const auto& model : presetArrays.children()) {
        auto& array{dynamic_cast<presets::Array&>(*model)};
        auto presets{data::context(array.presets_)};

        for (const auto& model : presets.children()) {
            auto& preset{dynamic_cast<presets::Preset&>(*model)};
            auto styles{data::context(preset.styles_)};

            const auto newSize{std::max<uint32>(
                styles.children().size(), numBlades.val()
            )};

            while (styles.children().size() < newSize) {
                styles.insert(
                    styles.children().size(),
                    std::make_unique<presets::Style>(*this)
                );
            }
        }
    }
}

void Config::cache(std::unique_ptr<utils::Data>&& data) {
    mCache[hash()] = std::move(data);
}

utils::Data *Config::cache() const {
    auto iter{mCache.find(hash())};

    if (iter == mCache.end())
        return nullptr;

    return iter->second.get();
}

void Config::onAction() {
    mIsSaved.set(hash() == mSavedHash);
}

void Config::onNumBlades() {
    syncStyles();
}

void Config::onOSChoice() {
    if (const auto *ptr{os()}) {
        mBoardChoice.update(ptr->boards_.size());
        mPropChoice.update(propVec()->size());
    } else {
        mBoardChoice.update(0);
        mPropChoice.update(0);
    }
}

void Config::processPropRecommend(
    data::hier::Root& root, std::string_view key, std::string_view val
) {
    auto& config{dynamic_cast<Config&>(root)};

    auto ctxt{data::context(config.settings_.defines_)};
    ctxt.append<settings::Define>(config, std::string(key), std::string(val));

    config.settings_.processDefines();
}


data::logic::Element config::operator|(
    Config& config, Config::OSIsOrOverVersion data
) {
    struct Adapter : data::logic::detail::Base, data::Receiver {
        Adapter(const Config& config, utils::Version version) :
            config_{config}, ver_(std::move(version)) {
            static const auto osChoiceTable{[] {
                data::base::Choice::RecvTable table;
                table.onChoice_ = map(&Adapter::onChoice);
                return table;
            }()};
            observeWith(config.osChoice(), osChoiceTable);
        }

        ~Adapter() override { deactivate(); }

        bool tryLock() override {
            return config_.tryLock();
        }

        void unlock() override {
            config_.unlock();
        }

        bool doActivate() override {
            std::lock_guard scopeLock(config_);
            Receiver::activate();
            return isTrue();
        }

        void onChoice() {
            std::lock_guard scopeLock(*pLock);
            onChange(isTrue());
        }

        bool isTrue() {
            std::lock_guard scopeLock(config_);

            const auto *os{config_.os()};
            if (not os)
                return false;

            return ver_.compare(os->version_) <= 0;
        }

        const Config& config_;
        const utils::Version ver_;
    };

    return std::make_unique<Adapter>(config, std::move(data.ver_));
}

Info::Info() = default;

const data::base::String& Info::name() {
    return mName;
}

fs::path Info::path() {
    return ::savePath(data::context(mName).val());
}

std::optional<std::string> Info::save(
    logging::Branch *lBranch
) {
    std::lock_guard scopeLock(*this);

    auto& logger{logging::Branch::optCreateLogger("Info::save()", lBranch)};

    if (not mConfig) {
        return priv::errorMessage(logger, wxTRANSLATE("Config not loaded"));
    }

    Config::Context ctxt{*mConfig};
    auto name{data::context(mName)};

    std::optional<std::string> err;
    std::error_code errCode;
    const auto finalPath{this->path()};
    const fs::path tmpPath{finalPath.string() + ".tmp"};
    err = priv::io::generate(
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

    mConfig->mSavedHash = mConfig->hash();
    mConfig->mIsSaved.set(true);

    return err;
}

std::optional<std::string> Info::load() {
    std::lock_guard scopeLock(*this);

    auto& logger{logging::Context::getGlobal().createLogger("Info::load()")};

    if (mConfig) return std::nullopt;

    mConfig.reset(new Config);

    mConfig->suppressActions();

    const auto cfgPath{path()};
    if (fs::exists(cfgPath)) { 
        auto err{priv::io::parse(
            cfgPath,
            *mConfig,
            logger.binfo("Config (" + cfgPath.string() + ") exists, parsing...")
        )};
        if (err) {
            mConfig.reset();
            return err;
        }
    } else {
        logger.warn("Config (" + cfgPath.string() + ") does not exist, creating new...");
    }

    mConfig->unsuppressActions();

    mConfig->mSavedHash = mConfig->hash();
    mConfig->mIsSaved.set(true);

    return std::nullopt;
}

void Info::unload() {
    auto list{data::context(priv::list)};
    auto name{data::context(mName)};

    mConfig.reset();

    // Check if the file still exists and remove this from the list if not.
    { bool found{false};
        std::error_code err;
        fs::directory_iterator iter{paths::configDir(), err};
        for (const auto& entry : iter) {
            if (not entry.is_regular_file()) continue;
            if (entry.path().extension() != RAW_FILE_EXTENSION) continue;

            if (name.val() == entry.path().stem().string()) {
                found = true;
                break;
            }
        }

        if (not found) {
            // `this` is about to be deleted, it can't be accessed by the
            // context anymore
            name.release();

            list.remove(*this);
        }
    }
}

auto Info::config() -> const std::unique_ptr<Config>& {
    return mConfig;
}

const data::base::Vector& config::list() {
    return priv::list;
}

void config::update() {
    auto list{data::context(priv::list)};

    std::vector<std::string> files;

    std::error_code err;
    fs::directory_iterator iter(paths::configDir(), err);
    for (const auto& entry : iter) {
        if (not entry.is_regular_file()) continue;
        if (entry.path().extension() != RAW_FILE_EXTENSION) continue;

        files.emplace_back(entry.path().stem().string());
    }

    for (size idx{0}; idx < list.children().size(); ++idx) {
        auto& info{dynamic_cast<Info&>(*list.children()[idx])};
        auto name{data::context(info.mName)};

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
            auto& info{dynamic_cast<Info&>(*list.children()[idx])};
            auto name{data::context(info.mName)};

            if (name.val() == file) {
                found = true;
                break;
            }
        }

        if (found) continue;

        auto info{std::unique_ptr<Info>{new Info}};
        info->mName.change(std::string(file));
        list.append(std::move(info));
    }

    if (err) {
        logging::Context::getGlobal().quickLog(
            logging::Severity::Err,
            "update()",
            "Failed to get path list: " + err.message()
        );
    }
}

std::optional<std::string> config::import(
    const std::string& name, const fs::path& path
) {
    auto& logger{logging::Context::getGlobal().createLogger("import()")};

    auto vec{data::context(priv::list)};

    for (size idx{0}; idx < vec.children().size(); ++idx) {
        auto& info{dynamic_cast<Info&>(*vec.children()[idx])};
        if (data::context(info.name()).val() == name) {
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

std::optional<std::string> config::generate(
    const Config& config, const fs::path& path, logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("generate()", lBranch)};

    std::optional<std::string> err;
    std::error_code errCode;

    err = priv::io::generate(
        path,
        config,
        logger.binfo("Generating config at \"" + path.string() + "\"...")
    );

    if (err) fs::remove(path, errCode);

    return err;
}

namespace {

fs::path savePath(const std::string& name) {
    return 
        paths::configDir() /
        (static_cast<std::string>(name) + RAW_FILE_EXTENSION);
}

} // namespace

