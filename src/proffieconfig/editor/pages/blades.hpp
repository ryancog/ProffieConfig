#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/blades.hpp
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

#include "config/blades/servo.hpp"
#include "config/blades/ws281x.hpp"
#include "config/blades/simple.hpp"
#include "config/config.hpp"
#include "data/primitive/models/selector.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/types.hpp"

#include "../dialogs/awareness.hpp"
#include "../dialogs/bladearray.hpp"

struct BladesPage : data::Receiver {
    BladesPage(config::Config&);
    void deinit();

    pcui::DescriptorPtr ui();

protected:
    void onActivate() override;
    void preDeactivate() override;

private:
    pcui::DescriptorPtr selection();
    pcui::DescriptorPtr blades();
    static pcui::DescriptorPtr simple(config::blades::Simple&);
    pcui::DescriptorPtr ws281x(config::blades::WS281X&);
    pcui::DescriptorPtr splits(config::blades::WS281X&);
    static pcui::DescriptorPtr split(config::blades::WS281X::Split&);
    static pcui::DescriptorPtr servo(config::blades::Servo&);

    void onAwarenessButton(const pcui::CallbackContext&);
    void onEditButton(const pcui::CallbackContext&);
    void onAddButton(const pcui::CallbackContext&);
    void onRemoveButton();

    void onAddBladeButton();
    void onRemoveBladeButton();

    void onAddSplitButton(config::blades::WS281X&);
    void onRemoveSplitButton(config::blades::WS281X&);

    void onArrayChoice();
    void onBladeChoice();
    void onSubChoice();

    void attachIssues(const data::base::Integer&);
    void detachIssues();

    void onIssues();

    config::Config& mConfig;

    data::prim::Selector mArraySel;
    data::prim::Selector mBladeSel;
    data::prim::Selector mSubBladeSel;

    // To try and preserve choice across various changes.
    int32 mLastBladeChoice{-1};
    int32 mLastSubChoice{-1};

    data::prim::String mIssueLabel;
    const data::base::Integer *mIssues{nullptr};

    data::prim::String mPowerPinAddField;

    BladeArrayDlg *mArrayDlg{nullptr};
    BladeAwarenessDlg *mAwarenessDlg{nullptr};
};

