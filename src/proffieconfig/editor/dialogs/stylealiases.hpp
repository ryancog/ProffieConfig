#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/stylealiases.hpp
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
#include "data/primitive/models/selector.hpp"
#include "ui/dialog.hpp"
#include "ui/types.hpp"

struct StyleAliasesDlg : pcui::Dialog {
    StyleAliasesDlg(wxWindow *, config::Config&);

    void deinit();

private:
    pcui::DescriptorPtr ui();

    pcui::DescriptorPtr selection();
    pcui::DescriptorPtr fields();

    void onAddButton();
    void onRemoveButton();
    void onMoveUpButton();
    void onMoveDownButton();
    void onDuplicateButton();

    config::Config& mConfig;

    data::prim::Selector mStyleSel;
};

