#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/movable.h
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

#include <vector>

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/gdicmn.h>
#include <wx/timer.h>

namespace PCUI {

wxDECLARE_EVENT(MV_START, wxMouseEvent);
wxDECLARE_EVENT(MV_END, wxMouseEvent);

class Movable;

class MoveArea : public wxPanel {
public:
    MoveArea(wxWindow* parent,
              int32_t id,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              int32_t style = wxTAB_TRAVERSAL);

    Movable* getDragWindow() const { return dragWindow; }

    typedef std::function<bool(Movable*, wxPoint)> AdoptionRoutine;

    void addAdoptRoutine(wxWindow* moveWin, AdoptionRoutine routine);
    void removeAdoptRoutine(wxWindow* moveWin);

protected:
    friend class Movable;
    friend class MovePanel;

    void addChildToStack(Movable*);
    void removeChildFromStack(Movable*);

private:
    wxPanel* dragArea{nullptr};
    Movable* dragWindow{nullptr};
    std::unordered_map<wxWindow*, AdoptionRoutine> adoptionRoutines;
    std::vector<Movable*> childStack;

   wxPoint calcDelta(const wxPoint* mousePos);
};

class MovePanel : public wxPanel {
public:
    MovePanel();
    MovePanel(
            wxWindow* parent,
            MoveArea* moveArea,
            int32_t id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int32_t style = wxTAB_TRAVERSAL,
            const wxString& objName = "MovePanel");
    ~MovePanel();

    void create(
            wxWindow* parent, 
            MoveArea* moveArea, 
            int32_t id, 
            const wxPoint& pos,
            const wxSize& size,
            int32_t style = wxTAB_TRAVERSAL,
            const wxString& objName = "MovePanel");

    virtual bool hitTest(wxPoint) const;

protected:
    friend class MoveArea;
    friend class Movable;

    MoveArea* moveArea;
};

/**
 * Scrolled Move Panel
 *
 * Must add the underlying canvas, not this object, to a sizer
 * via getCanvas();
 */
class ScrolledMovePanel: public MovePanel {
public:
    ScrolledMovePanel(
            wxWindow* parent,
            MoveArea* moveArea,
            int32_t id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int32_t style = wxTAB_TRAVERSAL,
            int32_t scrollStyle = wxScrolledWindowStyle,
            const wxString& objName = "Scrollable Move Panel");
    ~ScrolledMovePanel();

    wxScrolledCanvas* getCanvas();
    virtual bool hitTest(wxPoint) const override;
    void scroll(const wxSize& deltaAmt);
    
protected:
    void dragTick(wxTimerEvent&);
    void shrinkToFit(int8_t ignoreDirs = 0);
    void checkSuppressRoutines();
    void restoreRoutine(Movable*);
    void suppressRoutine(Movable*, MoveArea::AdoptionRoutine = nullptr);

    enum {
        UP          = 0b00000001,
        DOWN        = 0b00000010,
        LEFT        = 0b00000100,
        RIGHT       = 0b00001000,
        REPOS_VERT  = 0b00010000,
        REPOS_HORIZ = 0b00100000,
    };
    enum {
        TIMER_DRAG
    };

    int32_t jogDistance{20};

    bool dragging{false};
    wxPoint lastDragPos;
    std::unordered_map<Movable*, MoveArea::AdoptionRoutine> suppressedRoutines;

    wxScrolledCanvas* canvas{nullptr};
    wxTimer* timer{nullptr};
};


class Movable : public virtual wxWindow {
public:
    Movable(MoveArea* moveParent);
    ~Movable();

    virtual bool hitTest(wxPoint) const;
    virtual void doOnGrab();
    void doGrab(wxMouseEvent& evt);
    MoveArea* getMoveArea();

protected:
    friend class MovePanel;
    friend class MoveArea;
    friend class ScrolledMovePanel;

    MoveArea::AdoptionRoutine routine{nullptr};
    MoveArea* const moveArea{nullptr};
};

}

