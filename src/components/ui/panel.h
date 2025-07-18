#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/panel.h
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

#include <wx/panel.h>
#include <wx/statbox.h>

#include "utils/types.h"

#include "private/export.h"

namespace PCUI {

struct PanelData {
    enum class Type {
        BASIC,
        FRAMED,
    };

    PanelData(Type type, string label) :
        mType{type}, mLabel{std::move(label)} {}

    [[nodiscard]] const string& label() const { return mLabel; }
    // Only used for FRAMED type
    void setLabel(string&& label) {
        if (mLabel == label) return;
        mLabel = label;
        mNew = true;
    }

    void enable(bool enable = true) { 
        if (mEnabled == enable) return;
        mEnabled = enable; 
        mNew = true;
    }
    void show(bool show = true) {
        if (mShown == show) return;
        mShown = show;
        mNew = true;
    }

private:
    friend class Panel;
    Type mType;
    bool mNew{false};
    bool mEnabled{true};
    bool mShown{true};
    string mLabel;
};

class UI_EXPORT Panel {
public:
    Panel(wxWindow *parent, PanelData& data);

    wxWindow *asParent() { 
        if (mStaticBox) return mStaticBox;
        return mPanel;
    };

    wxWindow *asChild() { return mPanel; }

    void bind(PanelData& data) {
        assert(data.mType == mData->mType);
        mData = &data;
        onUIUpdate();
    }

private:
    void onUIUpdate();

    wxPanel *mPanel;
    wxStaticBox *mStaticBox;
    PanelData *mData;
};

} // namespace PCUI
