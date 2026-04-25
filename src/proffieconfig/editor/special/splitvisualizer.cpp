#include "splitvisualizer.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

#include <algorithm>
#include <limits>
#include <map>
#ifndef __WXMSW__
#include <memory>
#endif

#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>
#include <wx/peninfobase.h>
#include <wx/settings.h>
#include <wx/window.h>

#include "config/blades/ws281x.hpp"
#include "data/helpers/exclusive.hpp"
#include "data/number.hpp"
#include "data/selector.hpp"
#include "data/string.hpp"
#include "ui/detail/datadriven.hpp"
#include "ui/utils.hpp"
#include "utils/parent.hpp"
#include "utils/rand.hpp"

constexpr auto MIN_SEGMENT_SIZE{12};

namespace {

struct Window : pcui::detail::IDataDriven, wxWindow {
    Window(
        wxWindow *,
        config::blades::WS281X&,
        data::Selector&
    );

    void preDestroyCripple() override;
    bool Layout() override;

    void regenerateSplitData();
    void recalcSizes();

    wxSize DoGetBestClientSize() const override;

    void paintEvent(wxPaintEvent&);
    void mouseMoved(wxMouseEvent&);
    void mouseLeave(wxMouseEvent&);
    void mouseClick(wxMouseEvent&);

    /**
     * Sure let's regenerate this every program run...
     */
    static std::vector<wxColour> sIndexColors;
    static const wxColour& color(uint32 idx);

    config::blades::WS281X& blade_;
    data::Selector& subSel_;

    struct SplitData {
        uint32 start_;
        uint32 length_;
        uint32 splitIdx_;
        uint32 segments_;
    };

    std::vector<SplitData> splitData_;
    int32 selectedSplit_{-1};
    int32 hoveredSplit_{-1};

    struct SplitSize {
        float64 start_;
        float64 length_;
        uint32 splitIdx_;
        uint32 segments_{0};
        float64 segmentSize_;

        bool overlapStart_{false};
        bool overlapEnd_{false};
    };

    std::vector<SplitSize> sizes_;

    struct VecReceiver : data::Vector::Receiver {
        void onInsert(size) override;
        void preRemove(size) override;
        void onRemove(size) override;
    } vecReceiver_;

    struct LengthReceiver : data::Integer::Receiver {
        void onSet() override;
    } lengthReceiver_;

    struct SelReceiver : data::Choice::Receiver {
        void onChoice() override;
    } selChoiceReceiver_;

    struct ExclReceiver : data::Exclusive::Receiver {
        void onSelection(size) override;
        Window *window_;
    };

    struct IntReceiver : data::Integer::Receiver {
        void onSet() override;
        Window *window_;
    };

    struct StrReceiver : data::String::Receiver {
        void onChange() override;
        Window *window_;
    };

    struct SplitReceivers {
        ExclReceiver type_;
        IntReceiver start_;
        IntReceiver end_;
        // start and end should cover length
        IntReceiver segments_;
        StrReceiver list_;
    };
    // Use a map so don't have to keep track of movement up/down, etc.
    // That doesn't happen with splits currently, but still..
    //
    // The map is only accesses from ctor and vec receiver, so no extra
    // locking is required.
    std::map<data::Model *, SplitReceivers> splitReceiversMap_;
};

} // namespace

pcui::DescriptorPtr SplitVisualizer::operator()() {
    return std::make_unique<SplitVisualizer::Desc>(std::move(*this));
}

SplitVisualizer::Desc::Desc(SplitVisualizer&& vis) :
    SplitVisualizer(std::move(vis)) {}

wxSizerItem *SplitVisualizer::Desc::build(
    const pcui::detail::Scaffold& scaffold
) const {
    auto *win{new Window(
        scaffold.childParent_,
        blade_,
        subSel_
    )};

    auto *item{new wxSizerItem(win)};
    pcui::detail::apply(base_, item);

    return item;
}

pcui::detail::Descriptor *SplitVisualizer::Desc::clone() const {
    return new Desc(*this);
}

namespace {

std::vector<wxColour> Window::sIndexColors;

Window::Window(
    wxWindow *parent,
    config::blades::WS281X& ws281x,
    data::Selector& subSel
) : wxWindow(parent, wxID_ANY),
    blade_{ws281x},
    subSel_{subSel} {

    {
        data::Vector::ROContext vecCtxt{blade_.splits_};
        data::Choice::ROContext choiceCtxt{subSel.choice_};

        for (const auto& model : vecCtxt.children()) {
            auto& split{static_cast<config::blades::WS281X::Split&>(*model)};
            auto& rcvrs{splitReceiversMap_[model.get()]};

            rcvrs.type_.window_ = this;
            rcvrs.start_.window_ = this;
            rcvrs.end_.window_ = this;
            rcvrs.segments_.window_ = this;
            rcvrs.list_.window_ = this;

            rcvrs.type_.attach(split.type_);
            rcvrs.start_.attach(split.start_);
            rcvrs.end_.attach(split.end_);
            rcvrs.segments_.attach(split.segments_);
            rcvrs.list_.attach(split.list_);
        }

        selectedSplit_ = choiceCtxt.idx();

        vecReceiver_.attach(blade_.splits_);
        lengthReceiver_.attach(blade_.length_);
        selChoiceReceiver_.attach(subSel.choice_);
    }

    Bind(wxEVT_PAINT, &Window::paintEvent, this);
    Bind(wxEVT_MOTION, &Window::mouseMoved, this);
    Bind(wxEVT_LEAVE_WINDOW, &Window::mouseLeave, this);
    Bind(wxEVT_LEFT_UP, &Window::mouseClick, this);

    SetAutoLayout(true);

    regenerateSplitData();
}

void Window::preDestroyCripple() {
    vecReceiver_.detach();
    lengthReceiver_.detach();
    selChoiceReceiver_.detach();

    for (auto& [model, rcvrs] : splitReceiversMap_) {
        rcvrs.type_.detach();
        rcvrs.start_.detach();
        rcvrs.end_.detach();
        rcvrs.segments_.detach();
        rcvrs.list_.detach();
    }
}

bool Window::Layout() {
    recalcSizes();
    Refresh();
    return true;
}

wxSize Window::DoGetBestClientSize() const {
    static const wxSize defaultSize{50, 200};

    uint32 maxSegments{0};
    for (const auto& data : splitData_) {
        maxSegments = std::max(data.segments_, maxSegments);
    }

    wxSize minSize{
        static_cast<int32>(maxSegments * MIN_SEGMENT_SIZE),
        static_cast<int32>(splitData_.size() * MIN_SEGMENT_SIZE)
    };
    minSize.IncTo(defaultSize);
    return minSize;
}

void Window::regenerateSplitData() {
    data::Vector::ROContext splitVec{blade_.splits_};
    data::Integer::ROContext length{blade_.length_};

    splitData_.clear();

    if (splitVec.children().empty()) {
        SplitData data;
        data.start_ = 0;
        data.length_ = length.val();
        data.splitIdx_ = 0;
        data.segments_ = 0;

        splitData_.push_back(data);
        return;
    }

    for (auto idx{0}; idx < splitVec.children().size(); ++idx) {
        auto& split{static_cast<config::blades::WS281X::Split&>(
            *splitVec.children()[idx]
        )};

        auto type{split.type_.selected()};
        using enum config::blades::WS281X::Split::Type;

        if (type == eStandard or type == eReverse) {
            SplitData data;
            data.start_ = data::Integer::ROContext{split.start_}.val();
            data.length_ = data::Integer::ROContext{split.length_}.val();
            data.segments_ = 0;
            data.splitIdx_ = idx;
            splitData_.push_back(data);
        } else if (type == eZig_Zag or type == eStride) {
            SplitData data;
            data.start_ = data::Integer::ROContext{split.start_}.val();
            data.length_ = data::Integer::ROContext{split.length_}.val();
            data.segments_ = data::Integer::ROContext{split.segments_}.val();
            data.splitIdx_ = idx;
            splitData_.push_back(data);
        } else if (type == eList) {
            SplitData data;
            data.start_ = std::numeric_limits<uint32>::max();

            for (auto val : split.listValues()) {
                if (
                        data.start_ != std::numeric_limits<uint32>::max() and
                        val == data.start_ + data.length_
                   ) {
                    ++data.length_;
                    continue;
                }

                if (data.start_ != std::numeric_limits<uint32>::max()) {
                    splitData_.push_back(data);
                }

                data.start_ = val;
                data.length_ = 1;
                data.segments_ = 0;
                data.splitIdx_ = idx;
            }

            if (data.start_ != std::numeric_limits<uint32>::max()) {
                splitData_.push_back(data);
            }
        }
    }
}

void Window::recalcSizes() {
    data::Integer::ROContext length{blade_.length_};

    sizes_.clear();

    const auto windowSize{GetSize()};
    std::vector<bool> dilationMap(length.val());
    float64 stdPixelSize{static_cast<float64>(windowSize.y) / length.val()};

    uint32 dilatedPixels{0};
    float64 dilatedSize{0};
    for (const auto& data : splitData_) {
        if (data.length_ * stdPixelSize < MIN_SEGMENT_SIZE) {
            float64 requiredPixelSize{
                static_cast<float64>(MIN_SEGMENT_SIZE) / data.length_
            };

            dilatedSize = std::max(requiredPixelSize, dilatedSize);
            for (auto idx{0}; idx < data.length_; ++idx) {
                dilationMap[data.start_ + idx] = true;
            }

            dilatedPixels += data.length_;
        }
    }

    uint32 dynamicPixels{length.val() - dilatedPixels};
    float64 dynamicSize{
        (windowSize.y - (dilatedPixels * dilatedSize)) / dynamicPixels
    };
    
    for (const auto& data : splitData_) {
        SplitSize size;
        float64 pos{0};
        for (auto idx{0}; idx < data.start_ + data.length_; ++idx) {
            if (idx == data.start_) size.start_ = pos;
            pos += dilationMap[idx] ? dilatedSize : dynamicSize;
        }
        size.length_ = pos - size.start_;

        size.splitIdx_ = data.splitIdx_;
        size.segments_ = data.segments_;
        size.segmentSize_ = static_cast<float64>(windowSize.x) / size.segments_;

        for (const auto& checkData : splitData_) {
            if (&data == &checkData) continue;
            auto end{data.start_ + data.length_};
            auto checkEnd{checkData.start_ + checkData.length_};
            if (checkData.start_ < data.start_ and checkEnd > data.start_) {
                size.overlapStart_ = true;
            }
            if (checkData.start_ < end and checkEnd > end) {
                size.overlapEnd_ = true;
            }
        }
        sizes_.push_back(size);
    }
}

void Window::paintEvent(wxPaintEvent&) {
    wxPaintDC paintDC{this};

    constexpr auto CORNER_RADIUS{10};
    auto windowSize{GetSize()};

#   ifdef __WXMSW__
    // GDIPlus doesn't support the layers apparently (and/or doesn't support
    // the composition modes I need for the masking effect) and Direct2D also
    // doesn't seem to have it implemented.
    //
    // There's other things I could do to render this but I just don't care
    // enough, so Windows doesn't get rounded corners.
    //
    // Sucks to use a bad operating system, should've used a better one...
    paintDC.SetPen(*wxTRANSPARENT_PEN);
    if (sizes_.empty()) {
        paintDC.SetBrush(wxBrush(
            wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)
        ));
        paintDC.DrawRoundedRectangle(
            0, 0, windowSize.x, windowSize.y, CORNER_RADIUS
        );
        return;
    }

    paintDC.SetBrush(wxBrush(
        wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)
    ));
    paintDC.DrawRectangle(0, 0, windowSize.x, windowSize.y);

    for (const auto& size : sizes_) {
        auto splitColor{color(size.splitIdx)};
        bool dark{wxSystemSettings::GetAppearance().AreAppsDark()};
        if (size.splitIdx == hoveredSplit_) {
            splitColor = splitColor.ChangeLightness(dark ? 70 : 140);
        }

        paintDC.SetPen(*wxTRANSPARENT_PEN);
        paintDC.SetBrush(wxBrush(splitColor));
        paintDC.DrawRectangle(
            0, static_cast<int32>(size.start),
            windowSize.x, static_cast<int32>(size.length)
        );

        wxPen borderPen{};
        borderPen.SetColour(splitColor.ChangeLightness(dark ? 120 : 85));
        borderPen.SetStyle(wxPENSTYLE_SOLID);
        paintDC.SetPen(borderPen);
        for (auto idx{1}; idx < size.segments; ++idx) {
            auto segmentX{size.segmentSize * idx};
            paintDC.DrawLine(
                static_cast<int32>(segmentX),
                static_cast<int32>(size.start),
                static_cast<int32>(segmentX),
                static_cast<int32>(size.start + size.length)
            );
        }

        borderPen.SetColour(
            wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)
        );

        if (size.start != 0) {
            borderPen.SetStyle(size.overlapStart
                ? wxPENSTYLE_DOT
                : wxPENSTYLE_SOLID
            );
            paintDC.SetPen(borderPen);
            paintDC.DrawLine(
                0, static_cast<int32>(size.start),
                windowSize.x, static_cast<int32>(size.start)
            );
        }

        borderPen.SetStyle(size.overlapEnd
            ? wxPENSTYLE_DOT
            : wxPENSTYLE_SOLID
        );
        paintDC.SetPen(borderPen);
        paintDC.DrawLine(
            0, static_cast<int32>(size.start + size.length),
            windowSize.x, static_cast<int32>(size.start + size.length)
        );

        auto font{wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT)};
        if (size.splitIdx == selectedSplit_) {
            font.MakeBold();
            font.SetFractionalPointSize(font.GetFractionalPointSize() * 1.2);
        }

        paintDC.SetFont(font);
        wxString numText{std::to_string(size.splitIdx)};
        wxCoord textWidth{};
        wxCoord textHeight{};
        paintDC.GetTextExtent(numText, &textWidth, &textHeight);
        paintDC.DrawText(
            numText,
            static_cast<int32>((windowSize.x - textWidth) / 2.0),
            static_cast<int32>(size.start + ((size.length - textHeight) / 2))
        );
    }
    // paintDC.SetPen(*wxTRANSPARENT_PEN);
    // paintDC.SetBrush(*wxWHITE_BRUSH);
    // paintDC.SetLogicalFunction(wxAND);
    // paintDC.DrawRoundedRectangle(0, 0, windowSize.x, windowSize.y, CORNER_RADIUS);
#   else
    
    std::unique_ptr<wxGraphicsContext> paintGC{
        wxGraphicsContext::Create(paintDC)
    };

    paintGC->SetPen(*wxTRANSPARENT_PEN);
    if (sizes_.empty()) {
        paintGC->SetBrush(wxBrush(
            wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)
        ));
        paintGC->DrawRoundedRectangle(
            0, 0, windowSize.x, windowSize.y, CORNER_RADIUS
        );
        return;
    }

    // On GTK the background also gets removed (weird see-through render
    // ensues) because of rendering reasons. I don't understand anything about
    // the composition modes. The theory formulas don't seem to match up with
    // any of the backend behaviors, and I don't care to rationalize it.
#   ifdef __WXGTK__
    paintGC->SetBrush(wxBrush(
        wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)
    ));
    paintGC->DrawRectangle(0, 0, windowSize.x, windowSize.y);
    paintGC->BeginLayer(1);
#   endif

    paintGC->SetBrush(wxBrush(
        wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)
    ));
    paintGC->DrawRectangle(0, 0, windowSize.x, windowSize.y);

    paintGC->SetCompositionMode(wxCOMPOSITION_OVER);
    paintGC->BeginLayer(1);
    for (const auto& size : sizes_) {
        auto splitColor{color(size.splitIdx_)};
        bool dark{wxSystemSettings::GetAppearance().AreAppsDark()};
        if (size.splitIdx_ == hoveredSplit_) {
            splitColor = splitColor.ChangeLightness(dark ? 70 : 140);
        }

        paintGC->SetPen(*wxTRANSPARENT_PEN);
        paintGC->SetBrush(wxBrush(splitColor));
        paintGC->DrawRectangle(
            0, size.start_, windowSize.x, size.length_
        );

        wxPen borderPen{};
        borderPen.SetColour(
            splitColor.ChangeLightness(dark ? 110 : 90)
        );
        borderPen.SetStyle(wxPENSTYLE_SOLID);
        paintGC->SetPen(borderPen);

        for (auto idx{1}; idx < size.segments_; ++idx) {
            auto segmentX{size.segmentSize_ * idx};
            paintGC->StrokeLine(
                segmentX,
                size.start_,
                segmentX,
                size.start_ + size.length_
            );
        }

        borderPen.SetColour(
            wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)
        );
        if (size.start_ != 0) {
            borderPen.SetStyle(size.overlapStart_
                ? wxPENSTYLE_DOT
                : wxPENSTYLE_SOLID
            );
            paintGC->SetPen(borderPen);
            paintGC->StrokeLine(
                0, size.start_, windowSize.x, size.start_
            );
        }
        borderPen.SetStyle(size.overlapEnd_
            ? wxPENSTYLE_DOT
            : wxPENSTYLE_SOLID
        );
        paintGC->SetPen(borderPen);
        paintGC->StrokeLine(
            0,
            size.start_ + size.length_,
            windowSize.x,
            size.start_ + size.length_
        );

        auto font{wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT)};
        if (size.splitIdx_ == selectedSplit_) {
            font.MakeBold();
            font.SetFractionalPointSize(
                font.GetFractionalPointSize() * 1.2
            );
        }
        paintGC->SetFont(
            font,
            wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT)
        );
        wxString numText{std::to_string(size.splitIdx_)};
        float64 textWidth{};
        float64 textHeight{};
        paintGC->GetTextExtent(
            numText, &textWidth, &textHeight
        );
        paintGC->DrawText(
            numText,
            (windowSize.x - textWidth) / 2.0,
            size.start_ + ((size.length_ - textHeight) / 2)
        );
    }
    paintGC->EndLayer();

    // All that for a rounded border
#   ifdef __WXGTK__
    paintGC->SetPen(wxNullPen);
    paintGC->SetCompositionMode(wxCOMPOSITION_DEST_ATOP);
    paintGC->BeginLayer(1);
    paintGC->SetBrush(*wxBLACK_BRUSH);
    paintGC->DrawRectangle(0, 0, windowSize.x, windowSize.y);
    paintGC->SetBrush(*wxWHITE_BRUSH);
    paintGC->DrawRoundedRectangle(
        0, 0, windowSize.x, windowSize.y, CORNER_RADIUS
    );
    paintGC->EndLayer();
    paintGC->SetCompositionMode(wxCOMPOSITION_ATOP);
    paintGC->EndLayer();
#   else
    paintGC->SetBrush(*wxWHITE_BRUSH);
    paintGC->SetCompositionMode(wxCOMPOSITION_DEST_IN);
    paintGC->BeginLayer(1);
    paintGC->DrawRoundedRectangle(
        0, 0, windowSize.x, windowSize.y, CORNER_RADIUS
    );
    paintGC->EndLayer();
#   endif
#   endif
}

void Window::mouseMoved(wxMouseEvent& evt) {
    auto pos{evt.GetPosition()};

    hoveredSplit_ = -1;
    for (auto iter{sizes_.rbegin()}; iter != sizes_.rend(); ++iter) {
        const auto& size{*iter};
        if (pos.y > size.start_ and pos.y < size.start_ + size.length_) {
            hoveredSplit_ = static_cast<int32>(size.splitIdx_);
            break;
        }
    }
    Refresh();
}

void Window::mouseLeave(wxMouseEvent&) {
    hoveredSplit_ = -1;
    Refresh();
}

void Window::mouseClick(wxMouseEvent&) {
    if (hoveredSplit_ == -1) return;

    data::Vector::ROContext splits{blade_.splits_};
    if (hoveredSplit_ >= splits.children().size()) return;

    data::Choice::Context subSel{subSel_.choice_};
    subSel.choose(hoveredSplit_);
}

const wxColour& Window::color(uint32 idx) {
    while (idx >= sIndexColors.size()) {
        const auto hue{utils::rand::get(0.0, 1.0)};
        const auto saturation{utils::rand::get(0.4, 0.7)};

        const auto rgbValue{wxImage::HSVtoRGB({
            hue, saturation, 0.7
        })};

        sIndexColors.emplace_back(
            rgbValue.red,
            rgbValue.green,
            rgbValue.blue,
            0x7F
        );
    }

    return sIndexColors[idx];
}

void Window::VecReceiver::onInsert(size pos) {
    auto *win{&utils::parent<&Window::vecReceiver_>(*this)};
    auto ctxt{context<data::Vector>()};

    const auto& model{ctxt.children()[pos]};
    auto& split{static_cast<config::blades::WS281X::Split&>(*model)};
    auto& rcvrs{win->splitReceiversMap_[model.get()]};

    rcvrs.type_.window_ = win;
    rcvrs.start_.window_ = win;
    rcvrs.end_.window_ = win;
    rcvrs.segments_.window_ = win;
    rcvrs.list_.window_ = win;

    rcvrs.type_.attach(split.type_);
    rcvrs.start_.attach(split.start_);
    rcvrs.end_.attach(split.end_);
    rcvrs.segments_.attach(split.segments_);
    rcvrs.list_.attach(split.list_);

    pcui::safeCall([win] {
        win->regenerateSplitData();
        win->Layout();
    });
}

void Window::VecReceiver::preRemove(size pos) {
    auto *win{&utils::parent<&Window::vecReceiver_>(*this)};
    auto ctxt{context<data::Vector>()};

    const auto& model{ctxt.children()[pos]};
    auto& rcvrs{win->splitReceiversMap_[model.get()]};

    rcvrs.type_.detach();
    rcvrs.start_.detach();
    rcvrs.end_.detach();
    rcvrs.segments_.detach();
    rcvrs.list_.detach();

    win->splitReceiversMap_.erase(model.get());
}

void Window::VecReceiver::onRemove(size) {
    auto *win{&utils::parent<&Window::vecReceiver_>(*this)};
    pcui::safeCall([win] {
        win->regenerateSplitData();
        win->Layout();
    });
}

void Window::LengthReceiver::onSet() {
    auto *win{&utils::parent<&Window::lengthReceiver_>(*this)};
    pcui::safeCall([win] {
        win->regenerateSplitData();
        win->Layout();
    });
}

void Window::SelReceiver::onChoice() {
    auto& win{utils::parent<&Window::selChoiceReceiver_>(*this)};
    pcui::safeCall([&win, idx=context<data::Choice>().idx()] {
        win.selectedSplit_ = idx;
        win.Refresh();
    });
}

void Window::ExclReceiver::onSelection(size) {
    pcui::safeCall([win=window_] {
        win->regenerateSplitData();
        win->Layout();
    });
}

void Window::IntReceiver::onSet() {
    pcui::safeCall([win=window_] {
        win->regenerateSplitData();
        win->Layout();
    });
}

void Window::StrReceiver::onChange() {
    pcui::safeCall([win=window_] {
        win->regenerateSplitData();
        win->Layout();
    });
}

} // namespace

