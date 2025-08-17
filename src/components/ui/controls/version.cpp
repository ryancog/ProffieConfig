#include "version.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/version.cpp
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

#include "utils/string.h"

PCUI::VersionData::VersionData() {
    mMajor.setUpdateHandler([this](uint32 id) {
        if (id != mMajor.ID_VALUE) return;
        
        auto rawValue{static_cast<string>(mMajor)};
        auto insertionPoint{mMajor.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimForNumeric(rawValue, &numTrimmed, insertionPoint);

        if (numTrimmed == 0) return;

        if (mMajor.empty()) {
            mMajor = "0";
            mMajor.setInsertionPointEnd();
            return;
        }

        mMajor = std::move(rawValue);
        mMajor.setInsertionPoint(insertionPoint - numTrimmed);
    });
    mMinor.setUpdateHandler([this](uint32 id) {
        if (id != mMinor.ID_VALUE) return;

        auto rawValue{static_cast<string>(mMinor)};
        auto insertionPoint{mMinor.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimForNumeric(rawValue, &numTrimmed, insertionPoint);

        if (numTrimmed == 0) return;

        mMinor = std::move(rawValue);
        mMinor.setInsertionPoint(insertionPoint - numTrimmed);
    });
    mBugfix.setUpdateHandler([this](uint32 id) {
        if (id != mBugfix.ID_VALUE) return;

        auto rawValue{static_cast<string>(mBugfix)};
        auto insertionPoint{mBugfix.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimForNumeric(rawValue, &numTrimmed, insertionPoint);

        if (numTrimmed == 0) return;

        if (mMinor.empty()) {
            mBugfix.clear();
            mMinor = std::move(rawValue);
            mMinor.setInsertionPoint(insertionPoint - numTrimmed);
            return;
        }

        mBugfix = std::move(rawValue);
        mBugfix.setInsertionPoint(insertionPoint - numTrimmed);
    });
    mTag.setUpdateHandler([this](uint32 id) {
        if (id != mTag.ID_VALUE) return;

        auto rawValue{static_cast<string>(mTag)};
        auto insertionPoint{mTag.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimUnsafe(rawValue, &numTrimmed, insertionPoint);

        if (numTrimmed == 0) return;

        mTag = std::move(rawValue);
        mTag.setInsertionPoint(insertionPoint - numTrimmed);
    });
}

PCUI::VersionData::VersionData(const Utils::Version& version) : VersionData() {
    setValue(version);
}

PCUI::VersionData::operator Utils::Version() {
    std::scoped_lock scopeLock{getLock()};
    return Utils::Version(
        std::stoi(mMajor),
        mMinor.empty() ? Utils::Version::NULL_REV : std::stoi(mMinor),
        mBugfix.empty() ? Utils::Version::NULL_REV : std::stoi(mBugfix),
        mTag
    );
}

void PCUI::VersionData::operator=(const Utils::Version& version) {
    std::scoped_lock scopeLock{getLock()};
    if (version == *this) return;
    setValue(version);
}

void PCUI::VersionData::setValue(const Utils::Version& version) {
    std::scoped_lock scopeLock{getLock()};
    mMajor = std::to_string(version.major);
    mMinor = std::to_string(version.minor);
    mBugfix = std::to_string(version.bugfix);
    mTag = string{version.tag};
    notify(ID_VALUE);
}

PCUI::Version::Version(
    wxWindow *parent,
    VersionData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    mMajor = new PCUI::Text(this, data.mMajor);
    mMinor = new PCUI::Text(
        this, data.mMinor, 0, false, ".", wxHORIZONTAL
    );
    mMinor = new PCUI::Text(
        this, data.mMinor, 0, false, ".", wxHORIZONTAL
    );
    mMinor = new PCUI::Text(
        this, data.mMinor, 0, false, "-", wxHORIZONTAL
    );

    create(label, orient);
}

PCUI::Version::Version(
    wxWindow *parent,
    VersionDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    mMajor = new PCUI::Text(this, proxy.mMajor);
    mMinor = new PCUI::Text(
        this, proxy.mMinor, 0, false, ".", wxHORIZONTAL
    );
    mMinor = new PCUI::Text(
        this, proxy.mMinor, 0, false, ".", wxHORIZONTAL
    );
    mMinor = new PCUI::Text(
        this, proxy.mMinor, 0, false, "-", wxHORIZONTAL
    );
    create(label, orient);
}

void PCUI::Version::create(const wxString& label, wxOrientation orient) {
    auto *panel{new wxPanel(this)};
    auto *panelSizer{new wxBoxSizer(wxHORIZONTAL)};
    panelSizer->Add(mMajor);
    panelSizer->Add(mMinor);
    panelSizer->Add(mBugfix);
    panelSizer->Add(mTag, 1);
    panel->SetSizerAndFit(panelSizer);

    init(panel, wxEVT_ANY, label, orient);
}

void PCUI::Version::onUIUpdate(uint32) {}

void PCUI::Version::onModify(wxEvent& evt) { evt.Skip(); }

