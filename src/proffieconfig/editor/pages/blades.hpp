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

#include "config/config.hpp"
#include "config/blades/ws281x.hpp"
#include "config/blades/simple.hpp"
#include "ui/types.hpp"

#include "../dialogs/bladearray.hpp"

struct BladesPage {
    BladesPage(config::Config&);
    void deinit();

    pcui::DescriptorPtr ui();

private:
    pcui::DescriptorPtr selection();
    pcui::DescriptorPtr blades();
    pcui::DescriptorPtr simple(config::blades::Simple&);
    pcui::DescriptorPtr ws281x(config::blades::WS281X&);
    pcui::DescriptorPtr splits(config::blades::WS281X&);
    pcui::DescriptorPtr split(config::blades::WS281X::Split&);

    config::Config& mConfig;

    data::Selector mArraySel;
    data::Selector mBladeSel;
    data::Selector mSubBladeSel;

    // To try and preserve choice across various changes.
    int32 mLastBladeChoice{-1};
    int32 mLastSubChoice{-1};

    struct IssueReceiver : data::Integer::Receiver {
        void onSet() override;
        void onAttach() override;
        void preDetach() override;
        void updateLabel();
    } mIssueReceiver;
    data::String mIssueLabel;

    data::String mPowerPinAddField;

    BladeArrayDlg *mDlg{nullptr};
};

