#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/addconfig.hpp
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

#include "data/primitive/models/bool.hpp"
#include "data/primitive/models/exclusive.hpp"
#include "data/primitive/models/string.hpp"
#include "ui/dialog.hpp"
#include "ui/types.hpp"

struct MainMenu;

struct AddConfigDialog : pcui::Dialog, data::Receiver {
    AddConfigDialog(MainMenu *);
    ~AddConfigDialog() override;

    MainMenu *parent_{nullptr};

    struct Result {
        enum class Mode {
            Create,
            Import,
        };
        Mode mode_;
        std::string path_;
        std::string name_;
    };

    Result getResult();

protected:
    void onActivate() override;

private:
    pcui::DescriptorPtr ui();
    void bindEvents();

    void onName();
    void onPath();

    data::prim::Bool mNameValid;
    data::prim::Bool mDupName;
    data::prim::Bool mNeedImportPath;

    enum {
        eMode_Create,
        eMode_Import,
        eMode_Max,
    };
    data::prim::Exclusive mMode{eMode_Max};
    data::prim::String mImportPath;
    data::prim::String mConfigName;
};

