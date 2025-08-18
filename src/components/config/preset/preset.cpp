#include "preset.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/preset/preset.h
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
#include "../config.h"

Config::Preset::Preset(Config& config, PresetArray& presetArray) :
    mConfig{config}, mParent{presetArray} {
    name = "newpreset";
    styleSelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

    name.setUpdateHandler([this](uint32 id) {
        if (id != name.ID_VALUE) return;

        auto rawValue{static_cast<string>(name)};
        uint32 numTrimmed{};
        auto insertionPoint{name.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            "\\"
        );
        if (rawValue.length() > 20) rawValue.resize(20);
        // Only allow \n inserted by control or programmatically.
        if (rawValue.back() == '\\') rawValue.pop_back();

        if (rawValue == static_cast<string>(name)) {
            auto idx{0};
            for (const auto& preset : mParent.presets()) {
                if (&*preset == this) break;
                ++idx;
            }
            if (idx < mParent.presets().size()) {
                auto choices{mParent.selection.choices()};
                choices[idx] = name;
                mParent.selection.setChoices(std::move(choices));
            }
        }

        name = std::move(rawValue);
        name.setInsertionPoint(insertionPoint - numTrimmed);
    });
    fontDir.setUpdateHandler([this](uint32 id) {
        if (id != fontDir.ID_VALUE) return;

        auto rawValue{static_cast<string>(fontDir)};
        uint32 numTrimmed{};
        auto insertionPoint{fontDir.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            "/;"
        );

        if (rawValue == static_cast<string>(fontDir)) return;

        fontDir = std::move(rawValue);
        fontDir.setInsertionPoint(insertionPoint - numTrimmed);
    });
    track.setUpdateHandler([this](uint32 id) {
        if (id != track.ID_VALUE) return;

        auto rawValue{static_cast<string>(track)};
        uint32 numTrimmed{};
        auto insertionPoint{track.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            "./"
        );

        if (rawValue == static_cast<string>(track)) {
            mConfig.presetArrays.notifyData.notify(PresetArrays::NOTIFY_TRACK_INPUT);
            return;
        }

        track = std::move(rawValue);
        track.setInsertionPoint(insertionPoint - numTrimmed);
    });
    styleSelection.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::NotifyReceiver::ID_REBOUND and id != styleSelection.ID_SELECTION) return;

        if (mConfig.presetArrays.selection == -1) return;
        auto& selectedArray{mConfig.presetArrays.array(mConfig.presetArrays.selection)};
        if (selectedArray.selection == -1) return;
        auto& selectedPreset{selectedArray.preset(selectedArray.selection)};
        if (this != &selectedPreset) return;

        if (styleSelection == -1) {
            mConfig.presetArrays.commentProxy.bind(PresetArrays::dummyCommentData);
            mConfig.presetArrays.styleProxy.bind(PresetArrays::dummyStyleData);
        } else {
            auto& selectedStyle{style(styleSelection)};
            mConfig.presetArrays.commentProxy.bind(selectedStyle.comment);
            mConfig.presetArrays.styleProxy.bind(selectedStyle.style);
        }
    });
}

Config::Preset::Style& Config::Preset::addStyle() {
    return *mStyles.emplace_back(std::make_unique<Style>());
}

void Config::Preset::popBackStyle() {
    mStyles.pop_back();
}

Config::Preset::Style::Style() {
    comment.setUpdateHandler([this](uint32 id) {
        if (id != comment.ID_VALUE) return;

        auto rawValue{static_cast<string>(comment)};
        auto insertionPoint{comment.getInsertionPoint()};
        size_t illegalPos{};
        while (
                (illegalPos = rawValue.find("/*")) != string::npos or
                (illegalPos = rawValue.find("*/")) != string::npos
              ) {
            if (illegalPos < insertionPoint) insertionPoint -= 2;
            rawValue.erase(illegalPos, 2);
        }

        if (comment != rawValue) {
            comment = std::move(rawValue);
            comment.setInsertionPoint(insertionPoint);
        }
    });
    style.setUpdateHandler([this](uint32 id) {
        if (id != style.ID_VALUE) return;

        auto rawValue{static_cast<string>(style)};
        auto insertionPoint{style.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            "/*<>()&:,\"\n\t ",
            true
        );
        insertionPoint -= numTrimmed;

        size_t illegalPos{0};
        bool commentMove{false};
        while ((illegalPos = rawValue.find("/*", illegalPos)) != string::npos) {
            const auto terminatorPos{rawValue.find("*/", illegalPos)};
            const auto eraseEnd{terminatorPos == string::npos ? string::npos : terminatorPos + 2};
            if (eraseEnd < insertionPoint) insertionPoint -= eraseEnd - illegalPos;
            else if (illegalPos < insertionPoint) insertionPoint = illegalPos;

            const auto begin{illegalPos + 2};
            auto substr{rawValue.substr(begin, terminatorPos - begin)};
            Utils::trimSurroundingWhitespace(substr);
            if (not substr.empty()) {
                if (not comment.empty()) comment += '\n';
                comment += substr;
            }

            rawValue.erase(illegalPos, eraseEnd - illegalPos);
            commentMove = true;
        }

        // If comment terminator but no opener, move everything before terminator into comment
        if ((illegalPos = rawValue.rfind("*/")) != string::npos and rawValue.find("/*") == string::npos) {
            auto substr{rawValue.substr(0, illegalPos)};
            Utils::trimSurroundingWhitespace(substr);
            if (not substr.empty()) {
                if (not comment.empty()) comment += '\n';
                comment += substr;
            }
            const auto eraseEnd{illegalPos + 2};
            if (eraseEnd < insertionPoint) insertionPoint -= eraseEnd;
            else insertionPoint = 0;
            rawValue.erase(0, eraseEnd);
        }

        if ((illegalPos = rawValue.find(')')) != string::npos) {
            rawValue.erase(illegalPos + 1);
            if (insertionPoint > illegalPos + 1) insertionPoint = illegalPos + 1;
        }

        if (style != rawValue) {
            style = std::move(rawValue);
            style.setInsertionPoint(insertionPoint);
            if (commentMove) {
                comment.setFocus();
                comment.setInsertionPointEnd();
            }
        }
    });
    comment = "ProffieConfig Default Blue AudioFlicker";
    style = "StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()";
}

