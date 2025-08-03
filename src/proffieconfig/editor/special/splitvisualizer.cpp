#include "splitvisualizer.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * proffieconfig/editor/special/splitvisualizer.cpp
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

#include <limits>
#include <map>
#include <memory>

#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>
#include <wx/settings.h>

#include "config/bladeconfig/ws281x.h"
#include "utils/crypto.h"

struct SplitData {
    uint32 start;
    uint32 length;
    uint32 splitIdx;
    uint32 segments;
};
static vector<SplitData> generateSplitData(const Config::WS281XBlade&);

vector<wxColour> SplitVisualizer::smIndexColors;

constexpr auto MIN_SEGMENT_SIZE{12};

SplitVisualizer::SplitVisualizer(wxWindow *parent, Config::BladeArrays& bladeArrays) :
    wxWindow(parent, wxID_ANY), mBladeArrays{bladeArrays}, PCUI::Notifier(this, bladeArrays.visualizerData) {

    Bind(wxEVT_PAINT, &SplitVisualizer::paintEvent, this);
    Bind(wxEVT_SIZE, &SplitVisualizer::resize, this);
    Bind(wxEVT_MOTION, &SplitVisualizer::mouseMoved, this);
    Bind(wxEVT_LEAVE_WINDOW, &SplitVisualizer::mouseLeave, this);
    Bind(wxEVT_LEFT_UP, &SplitVisualizer::mouseClick, this);

    initializeNotifier();
}

void SplitVisualizer::handleNotification(uint32) {
    auto minSize{calculateSizes()};
    SetMinSize(minSize);
    auto *parent{wxGetTopLevelParent(this)};
    if (parent) parent->Layout();
    Refresh();
}

wxSize SplitVisualizer::calculateSizes() {
    static const wxSize defaultSize{50, 200};
    mSizes.clear();

    if (mBladeArrays.arraySelection == -1) return defaultSize;
    auto& selectedArray{mBladeArrays.array(mBladeArrays.arraySelection)};
    if (selectedArray.bladeSelection == -1) return defaultSize;
    auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
    if (selectedBlade.type != Config::Blade::WS281X) return defaultSize;
    auto& ws281x{selectedBlade.ws281x()};

    const auto windowSize{GetSize()};
    const auto splitData{generateSplitData(ws281x)};
    vector<bool> dilationMap(ws281x.length);
    float64 stdPixelSize{static_cast<float64>(windowSize.y) / ws281x.length};

    uint32 dilatedPixels{0};
    float64 dilatedSize{0};
    for (const auto& data : splitData) {
        if (data.length * stdPixelSize < MIN_SEGMENT_SIZE) {
            float64 requiredPixelSize{static_cast<float64>(MIN_SEGMENT_SIZE) / data.length};
            if (requiredPixelSize > dilatedSize) dilatedSize = requiredPixelSize;
            for (auto idx{0}; idx < data.length; ++idx) {
                dilationMap[data.start + idx] = true;
            }
            dilatedPixels += data.length;
        }
    }

    uint32 dynamicPixels{ws281x.length - dilatedPixels};
    float64 dynamicSize{(windowSize.y - (dilatedPixels * dilatedSize)) / dynamicPixels};
    
    for (const auto& data : splitData) {
        SplitSize size;
        float64 pos{0};
        for (auto idx{0}; idx < data.start + data.length; ++idx) {
            if (idx == data.start) size.start = pos;
            pos += dilationMap[idx] ? dilatedSize : dynamicSize;
        }
        size.length = pos - size.start;

        size.splitIdx = data.splitIdx;
        size.segments = data.segments;
        size.segmentSize = static_cast<float64>(windowSize.x) / size.segments;

        for (const auto& checkData : splitData) {
            if (&data == &checkData) continue;
            auto end{data.start + data.length};
            auto checkEnd{checkData.start + checkData.length};
            if (checkData.start < data.start and checkEnd > data.start) {
                size.overlapStart = true;
            }
            if (checkData.start < end and checkEnd > end) {
                size.overlapEnd = true;
            }
        }
        mSizes.push_back(std::move(size));
    }

    selectedSplit = ws281x.splitSelect;

    uint32 maxSegments{0};
    for (const auto& data : splitData) {
        if (data.segments > maxSegments) maxSegments = data.segments;
    }
    wxSize minSize{
        static_cast<int32>(maxSegments * MIN_SEGMENT_SIZE),
        static_cast<int32>(splitData.size() * MIN_SEGMENT_SIZE)
    };
    minSize.IncTo(defaultSize);
    return minSize;
}

static vector<SplitData> generateSplitData(const Config::WS281XBlade& blade) {
    if (blade.splits().empty()) {
        SplitData ret;
        ret.start = 0;
        ret.length = blade.length;
        ret.splitIdx = 0;
        ret.segments = 0;
        return {ret};
    }

    vector<SplitData> ret;
    for (auto idx{0}; idx < blade.splits().size(); ++idx) {
        auto& split{*blade.splits()[idx]};

        if (split.type == Config::Split::STANDARD or split.type == Config::Split::REVERSE) {
            SplitData data;
            data.start = split.start;
            data.length = split.length;
            data.segments = 0;
            data.splitIdx = idx;
            ret.push_back(std::move(data));
        } else if (split.type == Config::Split::ZIG_ZAG or split.type == Config::Split::STRIDE) {
            SplitData data;
            data.start = split.start;
            data.length = split.length;
            data.segments = split.segments;
            data.splitIdx = idx;
            ret.push_back(std::move(data));
        } else if (split.type == Config::Split::LIST) {
            SplitData data;
            data.start = std::numeric_limits<uint32>::max();
            for (auto val : split.listValues()) {
                if (data.start != std::numeric_limits<uint32>::max() and val == data.start + data.length) {
                    ++data.length;
                    continue;
                } else {
                    if (data.start != std::numeric_limits<uint32>::max()) {
                        ret.push_back(std::move(data));
                    }

                    data.start = val;
                    data.length = 1;
                    data.segments = 0;
                    data.splitIdx = idx;
                }
            }
            if (data.start != std::numeric_limits<uint32>::max()) {
                ret.push_back(std::move(data));
            }
        }
    }

    return ret;
}

void SplitVisualizer::paintEvent(wxPaintEvent&) {
    wxPaintDC dc{this};
    std::unique_ptr<wxGraphicsContext> gc{wxGraphicsContext::Create(dc)};

    constexpr auto CORNER_RADIUS{10};
    auto windowSize{GetSize()};

    gc->SetPen(*wxTRANSPARENT_PEN);
    if (mSizes.empty()) {
        gc->SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
        gc->DrawRoundedRectangle(0, 0, windowSize.x, windowSize.y, CORNER_RADIUS);
        return;
    }

    gc->SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)));
    gc->DrawRectangle(0, 0, windowSize.x, windowSize.y);

    gc->BeginLayer(1);
    for (const auto& size : mSizes) {
        auto splitColor{color(size.splitIdx)};
        bool dark{wxSystemSettings::GetAppearance().AreAppsDark()};
        if (size.splitIdx == hoveredSplit) {
            splitColor = splitColor.ChangeLightness(dark ? 70 : 140);
        }

        gc->SetCompositionMode(wxCOMPOSITION_OVER);
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(wxBrush(splitColor));
        gc->DrawRectangle(0, size.start, windowSize.x, size.length);

        wxPen borderPen{};
        borderPen.SetColour(splitColor.ChangeLightness(dark ? 110 : 90));
        borderPen.SetStyle(wxPENSTYLE_SOLID);
        gc->SetPen(borderPen);
        for (auto idx{1}; idx < size.segments; ++idx) {
            auto segmentX{size.segmentSize * idx};
            gc->StrokeLine(segmentX, size.start, segmentX, size.start + size.length);
        }

        borderPen.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        if (size.start != 0) {
            borderPen.SetStyle(size.overlapStart ? wxPENSTYLE_DOT : wxPENSTYLE_SOLID);
            gc->SetPen(borderPen);
            gc->StrokeLine(0, size.start, windowSize.x, size.start);
        }
        borderPen.SetStyle(size.overlapEnd ? wxPENSTYLE_DOT : wxPENSTYLE_SOLID);
        gc->SetPen(borderPen);
        gc->StrokeLine(0, size.start + size.length, windowSize.x, size.start + size.length);

        auto font{wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT)};
        if (size.splitIdx == selectedSplit) font.MakeBold();
        gc->SetFont(
            font,
            wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)
        );
        wxString numText{std::to_string(size.splitIdx)};
        float64 textWidth;
        float64 textHeight;
        gc->GetTextExtent(numText, &textWidth, &textHeight);
        gc->DrawText(numText, (windowSize.x - textWidth) / 2.0, size.start + ((size.length - textHeight) / 2));
    }
    gc->EndLayer();

    gc->SetBrush(*wxWHITE_BRUSH);
    gc->SetCompositionMode(wxCOMPOSITION_DEST_IN);
    gc->BeginLayer(1);
    gc->DrawRoundedRectangle(0, 0, windowSize.x, windowSize.y, CORNER_RADIUS);
    gc->EndLayer();
}

void SplitVisualizer::resize(wxSizeEvent&) {
    calculateSizes();
    Refresh();
}

void SplitVisualizer::mouseMoved(wxMouseEvent& evt) {
    auto pos{evt.GetPosition()};

    hoveredSplit = -1;
    for (auto iter{mSizes.rbegin()}; iter != mSizes.rend(); ++iter) {
        const auto& size{*iter};
        if (pos.y > size.start and pos.y < size.start + size.length) {
            hoveredSplit = size.splitIdx;
            break;
        }
    }
    Refresh();
}

void SplitVisualizer::mouseLeave(wxMouseEvent&) {
    hoveredSplit = -1;
    Refresh();
}

void SplitVisualizer::mouseClick(wxMouseEvent&) {
    if (hoveredSplit == -1) return;
    if (mBladeArrays.arraySelection == -1) return;
    auto& selectedArray{mBladeArrays.array(mBladeArrays.arraySelection)};
    if (selectedArray.bladeSelection == -1) return;
    auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
    if (selectedBlade.type != Config::Blade::WS281X) return;
    auto& ws281x{selectedBlade.ws281x()};

    if (hoveredSplit >= ws281x.splits().size()) return;
    ws281x.splitSelect = hoveredSplit;
}

const wxColour& SplitVisualizer::color(uint32 idx) {
    while (idx >= smIndexColors.size()) {
        const auto hue{Crypto::random(0.0, 1.0)};
        const auto saturation{Crypto::random(0.4, 0.7)};
        const auto rgbValue{wxImage::HSVtoRGB({hue, saturation, 0.7})};
        smIndexColors.emplace_back(rgbValue.red, rgbValue.green, rgbValue.blue, 0x7F);
    }

    return smIndexColors[idx];
}

