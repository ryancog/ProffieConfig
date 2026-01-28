#include "version.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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
#include "utils/limits.h"

pcui::VersionData::VersionData() {
    mMajor.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto rawValue{static_cast<string>(mMajor)};
        bool jump{not rawValue.empty() and rawValue.back() == '.'};
        auto insertionPoint{mMajor.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimForNumeric(rawValue, &numTrimmed, insertionPoint);

        if (mMajor.empty()) {
            mMajor = "0";
            mMajor.setInsertionPointEnd();
            return;
        }

        string newValue;
        if (not rawValue.empty()) {
            using MajorType = decltype(Utils::Version::major);
            const auto clampedValue{std::clamp<MajorType>(
                strtoul(rawValue.c_str(), nullptr, 10),
                0,
                Utils::MAX<MajorType>
            )};
            newValue = std::to_string(clampedValue);
        }

        if (newValue != static_cast<string>(mMajor)) {
            mMajor = std::move(newValue);
            mMajor.setInsertionPoint(insertionPoint - numTrimmed);
        }

        if (jump) {
            mMinor.setInsertionPointEnd();
            mMinor.setFocus();
        }
        notify(eID_Value);
    });
    mMinor.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto rawValue{static_cast<string>(mMinor)};
        bool jump{not rawValue.empty() and rawValue.back() == '.'};
        auto insertionPoint{mMinor.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimForNumeric(rawValue, &numTrimmed, insertionPoint);

        string newValue;
        if (not rawValue.empty()) {
            using MinorType = decltype(Utils::Version::minor);
            const auto clampedValue{std::clamp<MinorType>(
                static_cast<MinorType>(strtoul(rawValue.c_str(), nullptr, 10)),
                0,
                Utils::MAX<MinorType>
            )};
            newValue = std::to_string(clampedValue);
        }

        if (newValue != static_cast<string>(mMinor)) {
            mMinor = std::move(newValue);
            mMinor.setInsertionPoint(insertionPoint - numTrimmed);
        }

        if (jump) {
            mBugfix.setInsertionPointEnd();
            mBugfix.setFocus();
        }
        notify(eID_Value);
    });
    mBugfix.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto rawValue{static_cast<string>(mBugfix)};
        bool jump{not rawValue.empty() and rawValue.back() == '-'};
        auto insertionPoint{mBugfix.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimForNumeric(rawValue, &numTrimmed, insertionPoint);

        if (not rawValue.empty() and mMinor.empty()) {
            mBugfix.clear();
            mMinor = std::move(rawValue);
            mMinor.setInsertionPoint(insertionPoint - numTrimmed);
            mMinor.setFocus();
            return;
        }

        string newValue;
        if (not rawValue.empty()) {
            using BugfixType = decltype(Utils::Version::bugfix);
            const auto clampedValue{std::clamp<BugfixType>(
                static_cast<BugfixType>(strtoul(rawValue.c_str(), nullptr, 10)),
                0,
                Utils::MAX<BugfixType>
            )};
            newValue = std::to_string(clampedValue);
        }

        if (newValue != static_cast<string>(mBugfix)) {
            mBugfix = std::move(newValue);
            mBugfix.setInsertionPoint(insertionPoint - numTrimmed);
        }

        if (jump) {
            mTag.setInsertionPointEnd();
            mTag.setFocus();
        }
        notify(eID_Value);
    });
    mTag.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto rawValue{static_cast<string>(mTag)};
        auto insertionPoint{mTag.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trim(
            rawValue,
            {.allowAlpha=true, .allowNum=true, .safeList="_"},
            &numTrimmed,
            insertionPoint
        );

        if (numTrimmed != 0) {
            mTag = std::move(rawValue);
            mTag.setInsertionPoint(insertionPoint - numTrimmed);
        }

        notify(eID_Value);
    });

    mMajor = "0";
}

pcui::VersionData::VersionData(const Utils::Version& version) : VersionData() {
    setValue(version);
}

pcui::VersionData::operator Utils::Version() {
    std::scoped_lock scopeLock{getLock()};
    return {
        static_cast<decltype(Utils::Version::major)>(
            strtoul(static_cast<string>(mMajor).c_str(), nullptr, 10)
        ),
        static_cast<decltype(Utils::Version::minor)>(
            mMinor.empty()
                ? Utils::Version::NULL_REV
                : strtoul(static_cast<string>(mMinor).c_str(), nullptr, 10)
        ),
        static_cast<decltype(Utils::Version::bugfix)>(
            mBugfix.empty()
                ? Utils::Version::NULL_REV 
                : strtoul(static_cast<string>(mBugfix).c_str(), nullptr, 10)
        ),
        mTag
    };
}

pcui::VersionData::operator string() {
    std::scoped_lock scopeLock{getLock()};
    return static_cast<string>(static_cast<Utils::Version>(*this));
}

pcui::VersionData& pcui::VersionData::operator=(const Utils::Version& version) {
    std::scoped_lock scopeLock{getLock()};
    if (version == static_cast<Utils::Version>(*this)) return *this;
    setValue(version);
    return *this;
}

void pcui::VersionData::setValue(const Utils::Version& version) {
    std::scoped_lock scopeLock{getLock()};
    mMajor = std::to_string(version.major);
    mMinor = version.minor == Utils::Version::NULL_REV ? "" : std::to_string(version.minor);
    mBugfix = version.bugfix == Utils::Version::NULL_REV ? "" : std::to_string(version.bugfix);
    mTag = string{version.tag};
    notify(eID_Value);
}

pcui::Version::Version(
    wxWindow *parent,
    VersionData& data,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, data) {
    create(label, orient);
}

pcui::Version::Version(
    wxWindow *parent,
    VersionDataProxy& proxy,
    const wxString& label,
    wxOrientation orient
) : ControlBase(parent, proxy) {
    create(label, orient);
}

void pcui::Version::create(const wxString& label, wxOrientation orient) {
    auto *panel{new wxPanel(this)};
    auto *dataProxy{static_cast<VersionDataProxy *>(proxy())};
    if (dataProxy) {
        mMajor = new pcui::Text(panel, dataProxy->mMajor);
        mMinor = new pcui::Text(panel, dataProxy->mMinor);
        mBugfix = new pcui::Text(panel, dataProxy->mBugfix);
        mTag = new pcui::Text(panel, dataProxy->mTag);
    } else {
        mMajor = new pcui::Text(panel, data()->mMajor);
        mMinor = new pcui::Text(panel, data()->mMinor);
        mBugfix = new pcui::Text(panel, data()->mBugfix);
        mTag = new pcui::Text(panel, data()->mTag);
    }

#   if defined(__WXGTK__) or defined(__WXMSW__)
    constexpr auto TEXT_PADDING{20};
#   else
    constexpr auto TEXT_PADDING{5};
#   endif
    const auto textWidth{GetTextExtent("255").GetWidth() + TEXT_PADDING};
    mMajor->SetMinSize({textWidth, -1}, false);
    mMinor->SetMinSize({textWidth, -1}, false);
    mBugfix->SetMinSize({textWidth, -1}, false);
    mTag->SetMinSize({textWidth * 2, -1}, false);

    auto *panelSizer{new wxBoxSizer(wxHORIZONTAL)};
    panelSizer->Add(mMajor);
    panelSizer->Add(
        new wxStaticText(panel, wxID_ANY, "."),
        wxSizerFlags().Bottom()
    );
    panelSizer->Add(mMinor);
    panelSizer->Add(
        new wxStaticText(panel, wxID_ANY, "."),
        wxSizerFlags().Bottom()
    );
    panelSizer->Add(mBugfix);
    panelSizer->Add(
        new wxStaticText(panel, wxID_ANY, "-"),
        wxSizerFlags().Center()
    );
    panelSizer->Add(mTag, 1);
    panel->SetSizerAndFit(panelSizer);

    init(panel, wxEVT_ANY, label, orient);
}

void pcui::Version::onUIUpdate(uint32) {}

void pcui::Version::onModify(wxEvent& evt) { evt.Skip(); }

