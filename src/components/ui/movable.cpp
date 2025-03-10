#include "movable.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/movable.cpp
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
    wxWindowID winID,
    const wxPoint& pos,
    const wxSize& size,
    int64_t style) :
    wxPanel(parent, winID, pos, size, style, "MoveArea") {
    Bind(wxEVT_LEFT_UP, [&](wxMouseEvent& evt) {
        if (HasCapture()) ReleaseMouse();
        if (mDragWindow) {
            auto newEvt{wxMouseEvent(evt)};
            newEvt.SetEventType(MV_END);
            wxPostEvent(this, newEvt);
            auto evtPos{ClientToScreen(evt.GetPosition())};

            for (auto childIt{mChildStack.crbegin()}; childIt != mChildStack.crend(); childIt++) {
                if (*childIt == mDragWindow) continue;
                if (!(*childIt)->pRoutine) continue;
                if (!(*childIt)->pRoutine(mDragWindow, evtPos)) continue;

                addChildToStack(mDragWindow);
                mDragWindow = nullptr;
                return;
            }
            for (const auto& [ window, routine ] : mAdoptionRoutines) {
                if (window == mDragWindow) continue;
                if (routine(mDragWindow, evtPos)) {
                    addChildToStack(mDragWindow);
                    mDragWindow = nullptr;
                    return;
                }
            }
            mDragWindow->Destroy();
            mDragWindow = nullptr;
        }
    });
    Bind(MOVECLICK_FALLTHROUGH, [&](wxMouseEvent& evt) {
        bool testing{false};
        for (auto childIt{mChildStack.crbegin()}; childIt != mChildStack.crend(); childIt++) {
            if (not testing) {
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
        if (not mDragWindow) return;

        auto pos{event.GetPosition()};
        mDragWindow->Move(mDragWindow->GetPosition() - calcDelta(&pos));
    });
}

void MoveArea::addAdoptRoutine(wxWindow* window, AdoptionRoutine routine) { mAdoptionRoutines.emplace(window, routine); };

void MoveArea::removeAdoptRoutine(wxWindow* window) { 
    auto routine{mAdoptionRoutines.find(window)};
    if (routine == mAdoptionRoutines.end()) return;
    mAdoptionRoutines.erase(routine);
};

void MoveArea::addChildToStack(Movable* child) {
    mChildStack.push_back(child);
    for (auto *const grandChild : child->GetChildren()) {
        auto *grandMoveChild{dynamic_cast<Movable*>(grandChild)};
        if (grandMoveChild) addChildToStack(grandMoveChild);
    }
}

void MoveArea::removeChildFromStack(Movable* child) {
    for (auto *const grandChild : child->GetChildren()) {
        auto *grandMoveChild{dynamic_cast<Movable*>(grandChild)};
        if (grandMoveChild) removeChildFromStack(grandMoveChild);
    }
    for (auto childIt{mChildStack.cbegin()}; childIt != mChildStack.cend(); childIt++) {
        if (*childIt == child) {
            mChildStack.erase(childIt); 
            return;
        }
    }
}

wxPoint MoveArea::calcDelta(const wxPoint* mousePos) {
    if (not mousePos) {
        mCalcData.reset = true;
        return {};
    }
    if (mCalcData.reset) {
        mCalcData.reset = false;
        mCalcData.lastPos = *mousePos;
    }

    auto ret{mCalcData.lastPos - *mousePos};
    mCalcData.lastPos = *mousePos;
    return ret;
}

MovePanel::MovePanel(
        wxWindow* parent,
        MoveArea* moveArea,
        wxWindowID winID,
        const wxPoint& pos,
        const wxSize& size,
        int64_t style,
        const std::string& objName) {
    create(parent, moveArea, winID, pos, size, style, objName);
}

void MovePanel::create(
        wxWindow* parent,
        MoveArea* moveArea,
        wxWindowID winID,
        const wxPoint& pos,
        const wxSize& size,
        int64_t style,
        const std::string& objName) {
    Create(parent, winID, pos, size, style, objName);
    this->pMoveArea = moveArea;
    moveArea->addAdoptRoutine(this, [this](Movable* toAdopt, wxPoint mousePos) -> bool {
        if (not hitTest(ScreenToClient(mousePos))) return false;

        auto winPos{toAdopt->GetScreenPosition()};
        toAdopt->Reparent(this);
        toAdopt->SetPosition(ScreenToClient(winPos));
        return true;
    });
}

MovePanel::~MovePanel() {
    pMoveArea->removeAdoptRoutine(this);
}

bool MovePanel::hitTest(wxPoint pos) const {
    if (pos.x < 0 or pos.y < 0) return false;

    auto size{GetSize()};
    return pos.x <= size.x and pos.y <= size.y;
}

ScrolledMovePanel::ScrolledMovePanel(
        wxWindow* parent,
        MoveArea* moveArea,
        wxWindowID winID,
        const wxPoint& pos,
        const wxSize& size,
        int64_t style,
        int64_t scrollStyle,
        const std::string& objName) {
    pCanvas = new wxScrolledCanvas(parent, winID, pos, size, scrollStyle, "Scrolled Canvas (Move Panel)");
    pCanvas->SetScrollRate(1, 1);
    auto *sizer{new wxBoxSizer(wxVERTICAL)};
    create(pCanvas, moveArea, winID, wxDefaultPosition, wxDefaultSize, style, objName);
    sizer->Add(this, wxSizerFlags(1).Expand());
    pCanvas->SetSizerAndFit(sizer);

    moveArea->Bind(PCUI::MV_START, [this](wxMouseEvent& evt) {
        evt.Skip();
        if (not pTimer) pTimer = new wxTimer(this, TIMER_DRAG);
        restoreRoutine(this->pMoveArea->getDragWindow());
        constexpr auto UPDATE_INTERVAL{20};
        pTimer->Start(UPDATE_INTERVAL);
    });
    moveArea->Bind(PCUI::MV_END, [this](wxMouseEvent& evt) {
        evt.Skip();
        shrinkToFit();
    });
    pCanvas->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, [this](wxScrollWinEvent& evt) {
        evt.Skip();
        shrinkToFit();
    });
    pCanvas->Bind(wxEVT_MOUSEWHEEL, [this](wxMouseEvent& evt) {
        if (evt.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) scroll({evt.GetWheelRotation() < 0 ? pJogDistance : -pJogDistance, 0});
        else scroll({0, evt.GetWheelRotation() < 0 ? pJogDistance : -pJogDistance});
        shrinkToFit();
    });
    Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& evt) {
        if (not HasCapture()) CaptureMouse();
        pLastDragPos = ClientToScreen(evt.GetPosition());
        pDragging = true;
    });
    Bind(wxEVT_LEFT_UP, [this](wxMouseEvent&) {
        if (HasCapture()) ReleaseMouse();
        shrinkToFit();
        pDragging = false;
    });
    Bind(wxEVT_MOTION, [this](wxMouseEvent& evt) {
        if (not pDragging) return;

        auto evtPos{ClientToScreen(evt.GetPosition())};
        wxSize scrollDelta{pLastDragPos.x - evtPos.x, pLastDragPos.y - evtPos.y};
        scroll(scrollDelta);
        pLastDragPos = evtPos;
    });
    Bind(wxEVT_TIMER, &ScrolledMovePanel::dragTick, this, TIMER_DRAG);
}

ScrolledMovePanel::~ScrolledMovePanel() {
    if (pCanvas and not pCanvas->GetParent()) pCanvas->Destroy();
}

wxScrolledCanvas* ScrolledMovePanel::getCanvas() { return pCanvas; }
bool ScrolledMovePanel::hitTest(wxPoint pos) const {
    if (pos.x < 0 or pos.y < 0) return false;

    auto size{GetSize()};
    return pos.x <= size.x and pos.y <= size.y;
}

void ScrolledMovePanel::checkSuppressRoutines() {
    for (auto *const child : GetChildren()) {
        auto *moveChild{dynamic_cast<Movable*>(child)};
        if (not moveChild) continue;

        auto viewStart{pCanvas->GetScreenPosition()};
        auto viewEnd{viewStart + pCanvas->GetSize()};
        auto childStart{moveChild->GetScreenPosition()};
        auto childEnd{childStart + moveChild->GetSize()};
        if (childEnd.x < viewStart.x or 
            childEnd.y < viewStart.y or 
            childStart.x > viewEnd.x or 
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
        if (clipSuppress) suppressRoutine(moveChild, [this, clippedRegion, moveChild](Movable* toAdopt, wxPoint pos) -> bool {
            if (not clippedRegion.Contains(pos)) return false;

            auto suppressedRoutineIt{pSuppressedRoutines.find(moveChild)};
            if (suppressedRoutineIt == pSuppressedRoutines.end()) return false;
            if (not suppressedRoutineIt->second) return false;

            return suppressedRoutineIt->second(toAdopt, pos);
        });
        else restoreRoutine(moveChild);
    }
}

void ScrolledMovePanel::suppressRoutine(Movable* movable, MoveArea::AdoptionRoutine routine) {
    auto routineIt{pSuppressedRoutines.find(movable)};
    if (routineIt == pSuppressedRoutines.end()) pSuppressedRoutines.emplace(movable, movable->pRoutine);
    movable->pRoutine = std::move(routine);
}

void ScrolledMovePanel::restoreRoutine(Movable* movable) {
    if (not movable) return;
    auto routineIt{pSuppressedRoutines.find(movable)};
    if (routineIt == pSuppressedRoutines.end()) return;

    movable->pRoutine = routineIt->second;
    pSuppressedRoutines.erase(routineIt);
}


void ScrolledMovePanel::dragTick(wxTimerEvent&) {
    auto *dragWindow{pMoveArea->getDragWindow()};
    if (not dragWindow) {
        pTimer->Stop();
        return;
    }

    auto mousePos{wxGetMousePosition()};
    auto physCanvasPos{pCanvas->GetScreenPosition()};
    auto physCanvasSize{pCanvas->GetSize()};
    static constexpr auto HOT_WIDTH{50};

    auto leftHotEdge{wxRegion(physCanvasPos.x, physCanvasPos.y, HOT_WIDTH, physCanvasSize.y)};
    auto rightHotEdge{wxRegion(physCanvasPos.x + physCanvasSize.x - HOT_WIDTH, physCanvasPos.y, HOT_WIDTH, physCanvasSize.y)};
    auto topHotEdge{wxRegion(physCanvasPos.x, physCanvasPos.y, physCanvasSize.x, HOT_WIDTH)};
    auto bottomHotEdge{wxRegion(physCanvasPos.x, physCanvasPos.y + physCanvasSize.y - HOT_WIDTH, physCanvasSize.x, HOT_WIDTH)};

    int8_t dirs{0};

    if (leftHotEdge.Contains(mousePos)) dirs |= LEFT;
    else if (rightHotEdge.Contains(mousePos)) dirs |= RIGHT;
    if (topHotEdge.Contains(mousePos)) dirs |= UP;
    else if (bottomHotEdge.Contains(mousePos)) dirs |= DOWN;

    if (not dirs) return;
    wxSize scrollDelta{
        (dirs & LEFT ? -pJogDistance : dirs & RIGHT ? pJogDistance : 0), 
        (dirs & UP ? -pJogDistance : dirs & DOWN ? pJogDistance : 0)
    };
    scroll(scrollDelta);
}

void ScrolledMovePanel::scroll(const wxSize& deltaAmt) {
    auto vScrollRange{pCanvas->GetScrollRange(wxVERTICAL)};
    auto vScrollPos{pCanvas->GetScrollPos(wxVERTICAL)};
    auto vScrollPageSize{pCanvas->GetScrollPageSize(wxVERTICAL)};
    auto hScrollRange{pCanvas->GetScrollRange(wxHORIZONTAL)};
    auto hScrollPos{pCanvas->GetScrollPos(wxHORIZONTAL)};
    auto hScrollPageSize{pCanvas->GetScrollPageSize(wxHORIZONTAL)};

    wxSize grow{0, 0};
    if ((hScrollPos == 0 and deltaAmt.x < 0) or 
            ((hScrollPos + hScrollPageSize == hScrollRange or hScrollPageSize == 0) and deltaAmt.x > 0)) {
        grow.x += abs(deltaAmt.x);
    }
    if ((vScrollPos == 0 and deltaAmt.y < 0) or 
            ((vScrollPos + vScrollPageSize == vScrollRange or vScrollPageSize == 0) and deltaAmt.y > 0)) { 
        grow.y += abs(deltaAmt.y);
    }

    auto virtCanvasSize{GetSize() + grow};
    SetSize(virtCanvasSize);
    SetMinSize(virtCanvasSize);
    pCanvas->FitInside();
    hScrollPos += deltaAmt.x;
    vScrollPos += deltaAmt.y;
    pCanvas->Scroll(hScrollPos, vScrollPos);

    for (auto *const child : GetChildren()) {
        auto childPos{child->GetPosition()};
        if (grow.y and deltaAmt.y < 0) childPos.y -= deltaAmt.y;
        if (grow.x and deltaAmt.x < 0) childPos.x -= deltaAmt.x;

        child->SetPosition(childPos);
    }

    checkSuppressRoutines();
}

void ScrolledMovePanel::shrinkToFit(int8_t ignoreDirs) {
    // For some reason, GetViewStart doesn't work correctly, so we have to do this:
    wxPoint usedAreaStart{pCanvas->GetScrollPos(wxHORIZONTAL), pCanvas->GetScrollPos(wxVERTICAL)};
    auto usedAreaEnd{usedAreaStart + pCanvas->GetSize()};
    
    for (auto *child : GetChildren()) {
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
    pCanvas->FitInside();

    for (auto *child : GetChildren()) {
        child->SetPosition(child->GetPosition() - usedAreaStart);
    }
}


Movable::Movable(MoveArea* moveParent) :
    pMoveArea(moveParent) {
    if (not pMoveArea) return;

    Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& evt) { doGrab(evt); });
    pMoveArea->addChildToStack(this);
};

Movable::~Movable() {
    if (not pMoveArea) return;

    pMoveArea->removeAdoptRoutine(this);
    pMoveArea->removeChildFromStack(this);
}


bool Movable::hitTest(wxPoint point) const { 
    if (point.x < 0 or point.y < 0) return false;

    auto thisSize{GetSize()};
    return point.x <= thisSize.x and point.y <= thisSize.y;
};

void Movable::doOnGrab() {}

void Movable::doGrab(wxMouseEvent& evt) {
    if (not hitTest(evt.GetPosition())) {
        wxMouseEvent newEvt(MOVECLICK_FALLTHROUGH);
        newEvt.SetPosition(ClientToScreen(evt.GetPosition()));
        newEvt.SetEventObject(this);
        wxPostEvent(pMoveArea, newEvt);
        return;
    }

    doOnGrab();

    auto startPos = this->GetScreenPosition();
    pMoveArea->removeChildFromStack(this);
    pMoveArea->mDragWindow = this;
    pMoveArea->calcDelta(nullptr);
    pMoveArea->CaptureMouse();

    this->Reparent(pMoveArea);
    this->Move(pMoveArea->ScreenToClient(startPos)); 
    auto mvEvt{wxMouseEvent(evt)};
    mvEvt.SetEventType(MV_START);
    wxPostEvent(pMoveArea, mvEvt); 
}

MoveArea* Movable::getMoveArea() { return pMoveArea; }


