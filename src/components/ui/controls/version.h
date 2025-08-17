#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/version.h
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

#include "utils/version.h"

#include "base.h"
#include "text.h"
#include "ui_export.h"

namespace PCUI {

struct UI_EXPORT VersionData : ControlData {
    VersionData();
    VersionData(const Utils::Version&);

    operator Utils::Version();

    /**
     * Efficiently set/update value
     */
    void operator=(const Utils::Version&);

    /**
     * Unconditionally set/update value
     */
    void setValue(const Utils::Version&);

    enum {
        ID_VALUE,
    };

private:
    friend class Version;
    friend class VersionDataProxy;
    PCUI::TextData mMajor;
    PCUI::TextData mMinor;
    PCUI::TextData mBugfix;
    PCUI::TextData mTag;
};

struct UI_EXPORT VersionDataProxy : ControlDataProxy<VersionData> {
    void bind(VersionData& data) {
        mMajor.bind(data.mMajor);
        mMinor.bind(data.mMinor);
        mBugfix.bind(data.mBugfix);
        mTag.bind(data.mTag);
        ControlDataProxy::bind(data);
    }
    void unbind() { 
        mMajor.unbind();
        mMinor.unbind();
        mBugfix.unbind();
        mTag.unbind();
        ControlDataProxy::unbind(); 
    }

private:
    friend class Version;
    PCUI::TextDataProxy mMajor;
    PCUI::TextDataProxy mMinor;
    PCUI::TextDataProxy mBugfix;
    PCUI::TextDataProxy mTag;
};

class UI_EXPORT Version : public ControlBase<
                          Version,
                          VersionData,
                          wxPanel,
                          wxEvent> {
public:
    Version(
        wxWindow *parent,
        VersionData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );
    Version(
        wxWindow *parent,
        VersionDataProxy& proxy,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );

private:
    void create(
        const wxString& label,
        wxOrientation orient
    );
    void onUIUpdate(uint32) final;
    void onModify(wxEvent&) final;

    PCUI::Text *mMajor;
    PCUI::Text *mMinor;
    PCUI::Text *mBugfix;
    PCUI::Text *mTag;
};

} // namespace PCUI

