#include "movable.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/movable.cpp
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

#include <iostream>

#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/timer.h>

using namespace PCUI;

wxDEFINE_EVENT(MOVECLICK_FALLTHROUGH, wxMouseEvent);
wxDEFINE_EVENT(PCUI::MV_START, wxMouseEvent);
wxDEFINE_EVENT(PCUI::MV_END, wxMouseEvent);

MoveArea::MoveArea(
    wxWindow* parent,
    int32_t id,
    const wxPoint& pos,
    const wxSize& size,
    int32_t style) :
    wxPanel(parent, id, pos, size, style, "MoveArea") {
    Bind(wxEVT_LEFT_UP, [&](wxMouseEvent& evt) {
        if (HasCapture()) ReleaseMouse();
        if (dragWindow) {
            auto newEvt{wxMouseEvent(evt)};
            newEvt.SetEventType(MV_END);
            wxPostEvent(this, newEvt);
            auto evtPos{ClientToScreen(evt.GetPosition())};

            for (auto childIt{childStack.rbegin()}; childIt != childStack.rend(); childIt++) {
                if (*childIt == dragWindow) continue;
                if (!(*childIt)->routine) continue;
                if (!(*childIt)->routine(dragWindow, evtPos)) continue;

                addChildToStack(dragWindow);
                dragWindow = nullptr;
                return;
            }
            for (const auto& [ window, routine ] : adoptionRoutines) {
                if (window == dragWindow) continue;
                if (routine(dragWindow, ClientToScreen(evt.GetPosition()))) {
                    addChildToStack(dragWindow);
                    dragWindow = nullptr;
                    return;
                }
            }
            dragWindow->Destroy();
            dragWindow = nullptr;
        }
    });
    Bind(MOVECLICK_FALLTHROUGH, [&](wxMouseEvent& evt) {
        bool testing{false};
        for (auto childIt{childStack.rbegin()}; childIt != childStack.rend(); childIt++) {
            if (!testing) {
                testing = (evt.GetEventObject() == *childIt);
                continue;
            }

            auto childRelPosition{(*childIt)->ScreenToClient(evt.GetPosition())};
            if ((*childIt)->hitTest(childRelPosition)) {
                evt.SetPosition(childRelPosition);
                (*childIt)->doGrab(evt);
                return;
            }
        }
    });
    Bind(wxEVT_MOTION, [&](wxMouseEvent& event) {
        auto pos{event.GetPosition()};
        if (dragWindow) dragWindow->Move(dragWindow->GetPosition() - calcDelta(&pos));
    });
}

void MoveArea::addAdoptRoutine(wxWindow* window, AdoptionRoutine routine) { adoptionRoutines.emplace(window, routine); };

void MoveArea::removeAdoptRoutine(wxWindow* window) { 
    auto routine{adoptionRoutines.find(window)};
    if (routine == adoptionRoutines.end()) return;
    adoptionRoutines.erase(routine);
};

void MoveArea::addChildToStack(Movable* child) {
    childStack.push_back(child);
    for (const auto cChild : child->GetChildren()) {
        auto moveChild{dynamic_cast<Movable*>(cChild)};
        if (moveChild) addChildToStack(moveChild);
    }
}

void MoveArea::removeChildFromStack(Movable* child) {
    for (auto childIt{childStack.cbegin()}; childIt != childStack.cend(); childIt++) {
        if (*childIt == child) {
            childStack.erase(childIt); 
            return;
        }
    }
}

wxPoint MoveArea::calcDelta(const wxPoint* mousePos) {
    static wxPoint lastPos;
    static bool reset{false};

    if (!mousePos) {
        reset = true;
        return {};
    }
    if (reset) {
        reset = false;
        lastPos = *mousePos;
    }

    auto ret{lastPos - *mousePos};
    lastPos = *mousePos;
    return ret;
}

MovePanel::MovePanel(
        wxWindow* parent,
        MoveArea* moveArea,
        int32_t id,
        const wxPoint& pos,
        const wxSize& size,
        int32_t style,
        const wxString& objName) {
    create(parent, moveArea, id, pos, size, style, objName);
}

MovePanel::MovePanel() {}

void MovePanel::create(
        wxWindow* parent,
        MoveArea* moveArea,
        int32_t id,
        const wxPoint& pos,
        const wxSize& size,
        int32_t style,
        const wxString& objName) {
    Create(parent, id, pos, size, style, objName);
    this->moveArea = moveArea;
    moveArea->addAdoptRoutine(this, [this](Movable* toAdopt, wxPoint mousePos) -> bool {
        if (!hitTest(mousePos)) return false;

        auto winPos{toAdopt->GetScreenPosition()};
        toAdopt->Reparent(this);
        toAdopt->SetPosition(ScreenToClient(winPos));
        return true;
    });
}

MovePanel::~MovePanel() {
    moveArea->removeAdoptRoutine(this);
}

bool MovePanel::hitTest(wxPoint pos) const {
    auto thisPos{GetScreenPosition()};
    auto thisEdge{thisPos + GetSize()};
    if (pos.x < thisPos.x || pos.y < thisPos.y) return false;
    if (pos.x > thisEdge.x || pos.y > thisEdge.y) return false;

    return true;
}

ScrolledMovePanel::ScrolledMovePanel(
        wxWindow* parent,
        MoveArea* moveArea,
        int32_t id,
        const wxPoint& pos,
        const wxSize& size,
        int32_t style,
        int32_t scrollStyle,
        const wxString& objName) {
    canvas = new wxScrolledCanvas(parent, id, pos, size, scrollStyle, "Scrolled Canvas (Move Panel)");
    canvas->SetScrollRate(1, 1);
    auto sizer{new wxBoxSizer(wxVERTICAL)};
    create(canvas, moveArea, id, wxDefaultPosition, wxDefaultSize, style, objName);
    sizer->Add(this, wxSizerFlags(1).Expand());
    canvas->SetSizerAndFit(sizer);

    moveArea->Bind(PCUI::MV_START, [this](wxMouseEvent& evt) {
        evt.Skip();
        if (!timer) timer = new wxTimer(this, TIMER_DRAG);
        restoreRoutine(this->moveArea->getDragWindow());
        timer->Start(20);
    });
    moveArea->Bind(PCUI::MV_END, [this](wxMouseEvent& evt) {
        evt.Skip();
        shrinkToFit();
    });
    canvas->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, [this](wxScrollWinEvent& evt) {
        evt.Skip();
        shrinkToFit();
    });
    canvas->Bind(wxEVT_MOUSEWHEEL, [this](wxMouseEvent& evt) {
        if (evt.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) scroll({evt.GetWheelRotation() < 0 ? jogDistance : -jogDistance, 0});
        else scroll({0, evt.GetWheelRotation() < 0 ? jogDistance : -jogDistance});
        shrinkToFit();
    });
    Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& evt) {
        if (!HasCapture()) CaptureMouse();
        lastDragPos = ClientToScreen(evt.GetPosition());
        dragging = true;
    });
    Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) {
        if (HasCapture()) ReleaseMouse();
        shrinkToFit();
        dragging = false;
    });
    Bind(wxEVT_MOTION, [this](wxMouseEvent& evt) {
        if (!dragging) return;

        auto evtPos{ClientToScreen(evt.GetPosition())};
        wxSize scrollDelta{lastDragPos.x - evtPos.x, lastDragPos.y - evtPos.y};
        scroll(scrollDelta);
        lastDragPos = evtPos;
    });
    Bind(wxEVT_TIMER, &ScrolledMovePanel::dragTick, this, TIMER_DRAG);
}

ScrolledMovePanel::~ScrolledMovePanel() {
    if (canvas && !canvas->GetParent()) canvas->Destroy();
}

wxScrolledCanvas* ScrolledMovePanel::getCanvas() { return canvas; }
bool ScrolledMovePanel::hitTest(wxPoint pos) const {
    auto thisPos{canvas->GetScreenPosition()};
    auto thisEdge{thisPos + canvas->GetSize()};
    if (pos.x < thisPos.x || pos.y < thisPos.y) return false;
    if (pos.x > thisEdge.x || pos.y > thisEdge.y) return false;

    return true;
}

void ScrolledMovePanel::checkSuppressRoutines() {
    for (const auto child : GetChildren()) {
        auto moveChild{dynamic_cast<Movable*>(child)};
        if (!moveChild) continue;

        auto viewStart{canvas->GetScreenPosition()};
        auto viewEnd{viewStart + canvas->GetSize()};
        auto childStart{moveChild->GetScreenPosition()};
        auto childEnd{childStart + moveChild->GetSize()};
        if (childEnd.x < viewStart.x || 
            childEnd.y < viewStart.y || 
            childStart.x > viewEnd.x || 
            childStart.y > viewEnd.y) {
            suppressRoutine(moveChild);
            continue;
        }

        bool clipSuppress{false};
        if (childStart.x < viewStart.x) { clipSuppress = true; childStart.x = viewStart.x; }
        if (childStart.y < viewStart.y) { clipSuppress = true; childStart.y = viewStart.y; }
        if (childEnd.x > viewEnd.x) { clipSuppress = true; childEnd.x = viewEnd.x; }
        if (childEnd.y > viewEnd.y) { clipSuppress = true; childEnd.y = viewEnd.y; }

        wxRegion clippedRegion{childStart, childEnd};
        if (clipSuppress) suppressRoutine(moveChild, [=](Movable* toAdopt, wxPoint pos) -> bool {
            if (!clippedRegion.Contains(pos)) return false;

            auto suppressedRoutineIt{suppressedRoutines.find(moveChild)};
            if (suppressedRoutineIt == suppressedRoutines.end()) return false;
            if (!suppressedRoutineIt->second) return false;

            return suppressedRoutineIt->second(toAdopt, pos);
        });
        else restoreRoutine(moveChild);
    }
}

void ScrolledMovePanel::suppressRoutine(Movable* movable, MoveArea::AdoptionRoutine routine) {
    auto routineIt{suppressedRoutines.find(movable)};
    if (routineIt == suppressedRoutines.end()) suppressedRoutines.emplace(movable, movable->routine);
    movable->routine = routine;
}

void ScrolledMovePanel::restoreRoutine(Movable* movable) {
    if (!movable) return;
    auto routineIt{suppressedRoutines.find(movable)};
    if (routineIt == suppressedRoutines.end()) return;

    movable->routine = routineIt->second;
    suppressedRoutines.erase(routineIt);
}


void ScrolledMovePanel::dragTick(wxTimerEvent&) {
    auto dragWindow{moveArea->getDragWindow()};
    if (!dragWindow) {
        timer->Stop();
        return;
    }

    auto mousePos{wxGetMousePosition()};
    auto physCanvasPos{canvas->GetScreenPosition()};
    auto physCanvasSize{canvas->GetSize()};
    static constexpr auto hotWidth{50};

    auto leftHotEdge{wxRegion(physCanvasPos.x, physCanvasPos.y, hotWidth, physCanvasSize.y)};
    auto rightHotEdge{wxRegion(physCanvasPos.x + physCanvasSize.x - hotWidth, physCanvasPos.y, hotWidth, physCanvasSize.y)};
    auto topHotEdge{wxRegion(physCanvasPos.x, physCanvasPos.y, physCanvasSize.x, hotWidth)};
    auto bottomHotEdge{wxRegion(physCanvasPos.x, physCanvasPos.y + physCanvasSize.y - hotWidth, physCanvasSize.x, hotWidth)};

    int8_t dirs{0};

    if (leftHotEdge.Contains(mousePos)) dirs |= LEFT;
    else if (rightHotEdge.Contains(mousePos)) dirs |= RIGHT;
    if (topHotEdge.Contains(mousePos)) dirs |= UP;
    else if (bottomHotEdge.Contains(mousePos)) dirs |= DOWN;

    if (!dirs) return;
    wxSize scrollDelta{
        (dirs & LEFT ? -jogDistance : dirs & RIGHT ? jogDistance : 0), 
        (dirs & UP ? -jogDistance : dirs & DOWN ? jogDistance : 0)
    };
    scroll(scrollDelta);
}

void ScrolledMovePanel::scroll(const wxSize& deltaAmt) {
    auto vScrollRange{canvas->GetScrollRange(wxVERTICAL)};
    auto vScrollPos{canvas->GetScrollPos(wxVERTICAL)};
    auto vScrollPageSize{canvas->GetScrollPageSize(wxVERTICAL)};
    auto hScrollRange{canvas->GetScrollRange(wxHORIZONTAL)};
    auto hScrollPos{canvas->GetScrollPos(wxHORIZONTAL)};
    auto hScrollPageSize{canvas->GetScrollPageSize(wxHORIZONTAL)};

    wxSize grow{0, 0};
    if ((hScrollPos == 0 && deltaAmt.x < 0) || 
            ((hScrollPos + hScrollPageSize == hScrollRange || hScrollPageSize == 0) && deltaAmt.x > 0)) {
        grow.x += abs(deltaAmt.x);
    }
    if ((vScrollPos == 0 && deltaAmt.y < 0) || 
            ((vScrollPos + vScrollPageSize == vScrollRange || vScrollPageSize == 0) && deltaAmt.y > 0)) { 
        grow.y += abs(deltaAmt.y);
    }

    auto virtCanvasSize{GetSize() + grow};
    SetSize(virtCanvasSize);
    SetMinSize(virtCanvasSize);
    canvas->FitInside();
    hScrollPos += deltaAmt.x;
    vScrollPos += deltaAmt.y;
    canvas->Scroll(hScrollPos, vScrollPos);

    for (const auto child : GetChildren()) {
        auto childPos{child->GetPosition()};
        if (grow.y && deltaAmt.y < 0) childPos.y -= deltaAmt.y;
        if (grow.x && deltaAmt.x < 0) childPos.x -= deltaAmt.x;

        child->SetPosition(childPos);
    }

    checkSuppressRoutines();
}

void ScrolledMovePanel::shrinkToFit(int8_t ignoreDirs) {
    // For some reason, GetViewStart doesn't work correctly, so we have to do this:
    wxPoint usedAreaStart{canvas->GetScrollPos(wxHORIZONTAL), canvas->GetScrollPos(wxVERTICAL)};
    auto usedAreaEnd{usedAreaStart + canvas->GetSize()};
    
    for (auto child : GetChildren()) {
        auto childStart{child->GetPosition()};
        auto childSize{child->GetSize()};
        auto childEnd{childStart + childSize};
        if (childStart.x < usedAreaStart.x) usedAreaStart.x = childStart.x;
        if (childStart.y < usedAreaStart.y) usedAreaStart.y = childStart.y;
        if (childEnd.x > usedAreaEnd.x) usedAreaEnd.x = childEnd.x;
        if (childEnd.y > usedAreaEnd.y) usedAreaEnd.y = childEnd.y;
    }

    if (ignoreDirs & UP) usedAreaStart.y = 0;
    if (ignoreDirs & DOWN) usedAreaEnd.y = GetSize().y;
    if (ignoreDirs & LEFT) usedAreaStart.y = 0;
    if (ignoreDirs & RIGHT) usedAreaEnd.x = GetSize().x;
    wxSize usedSize{usedAreaEnd.x - usedAreaStart.x, usedAreaEnd.y - usedAreaStart.y};
    SetSize(usedSize);
    SetMinSize(usedSize);
    canvas->FitInside();

    for (auto child : GetChildren()) {
        child->SetPosition(child->GetPosition() - usedAreaStart);
    }
}


Movable::Movable(MoveArea* moveParent) :
    moveArea(moveParent) {
    if (!moveArea) return;

    Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& evt) { doGrab(evt); });
    moveArea->addChildToStack(this);
};

Movable::~Movable() {
    if (!moveArea) return;

    moveArea->removeAdoptRoutine(this);
    moveArea->removeChildFromStack(this);
}

bool Movable::hitTest(wxPoint point) const { 
    if (point.x < 0 || point.y < 0) return false;
    auto thisSize{GetSize()};
    if (point.x > thisSize.x || point.y > thisSize.y) return false;

    return true;
};

void Movable::doOnGrab() {}

void Movable::doGrab(wxMouseEvent& evt) {
    if (!hitTest(evt.GetPosition())) {
        wxMouseEvent newEvt(MOVECLICK_FALLTHROUGH);
        newEvt.SetPosition(ClientToScreen(evt.GetPosition()));
        newEvt.SetEventObject(this);
        wxPostEvent(moveArea, newEvt);
        return;
    }

    doOnGrab();

    auto startPos = this->GetScreenPosition();
    moveArea->removeChildFromStack(this);
    for (const auto child : GetChildren()) {
        auto moveChild{dynamic_cast<Movable*>(child)};
        if (moveChild) moveArea->removeChildFromStack(moveChild);
    }
    moveArea->dragWindow = this;
    moveArea->calcDelta(nullptr);
    moveArea->CaptureMouse();

    this->Reparent(moveArea);
    this->Move(moveArea->ScreenToClient(startPos)); 
    auto mvEvt{wxMouseEvent(evt)};
    mvEvt.SetEventType(MV_START);
    wxPostEvent(moveArea, mvEvt); 
}

MoveArea* Movable::getMoveArea() { return moveArea; }


