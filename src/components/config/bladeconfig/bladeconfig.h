#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/bladeconfig.h
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

#include "ui/controls/numeric.h"
#include "ui/controls/text.h"
#include "utils/types.h"

#include "config_export.h"
#include "ws281x.h"
#include "simple.h"

namespace Config {

struct Config;

constexpr uint32 NO_BLADE{1000000000};

struct CONFIG_EXPORT Blade {
    Blade(Config&);

    enum Type {
        WS281X,
        SIMPLE,
        UNASSIGNED,

        INVALID = -1,
    };
    PCUI::ChoiceData type;

    PCUI::NumericData brightness;

    WS281XBlade& ws281x() { return mPixelBlade; }
    SimpleBlade& simple() { return mSimpleBlade; }

    Config& config() const { return mConfig; }

private:
    Config& mConfig;
    WS281XBlade mPixelBlade;
    SimpleBlade mSimpleBlade;
};

struct CONFIG_EXPORT BladeConfig {
    BladeConfig(Config&);

    [[nodiscard]] const vector<std::unique_ptr<Blade>>& blades() const { return mBlades; }
    [[nodiscard]] Blade& blade(uint32 idx) const { 
        return **std::next(mBlades.begin(), idx);
    }

    Blade& addBlade();
    void removeBlade(uint32 idx);

    PCUI::ChoiceData bladeSelection;

    // Notify of issues
    PCUI::Notifier notifyData;

    enum Issue {
        ISSUE_NONE = 0,
        ISSUE_NO_PRESETARRAY  = 1UL << 0,
        ISSUE_DUPLICATE_ID    = 1UL << 1,
        ISSUE_DUPLICATE_NAME  = 1UL << 2,
    };
    static constexpr auto ISSUE_WARNINGS{
        ISSUE_DUPLICATE_ID
    };
    static constexpr auto ISSUE_ERRORS{
        ISSUE_DUPLICATE_NAME | ISSUE_NO_PRESETARRAY
    };
    [[nodiscard]] uint32 computeIssues() const;

    /**
     * @param Issue or bitor'd Issue's
     * @return untranslated string
     */
    [[nodiscard]] static string issueString(uint32 issues);

    PCUI::TextData name;
    PCUI::ChoiceData presetArray;
    PCUI::NumericData id;
    PCUI::ToggleData noBladeID;

private:
    Config& mConfig;
    vector<std::unique_ptr<Blade>> mBlades;
};

} // namespace Config
