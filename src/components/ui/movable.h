#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/movable.h
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
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/timer.h>

#include <utils/types.h>

namespace pcui {

wxDECLARE_EVENT(MV_START, wxMouseEvent);
wxDECLARE_EVENT(MV_END, wxMouseEvent);

class Movable;

class MoveArea : public wxPanel {
public:
    MoveArea(
            wxWindow* parent,
            wxWindowID winID,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int64_t style = wxTAB_TRAVERSAL
            );

    Movable* getDragWindow() const { return mDragWindow; }

    /**
     * @brief AdoptionRoutine functions take in a movable
     * as well as it's screen position, test to see if they
     * want to adopt the movable, and return true if so, false otherwise.
    */
    using AdoptionRoutine = std::function<bool (Movable *, wxPoint)>;

    void addAdoptRoutine(wxWindow* window, AdoptionRoutine routine);
    void removeAdoptRoutine(wxWindow* window);

protected:
    friend class Movable;
    friend class MovePanel;

    void addChildToStack(Movable*);
    void removeChildFromStack(Movable*);

private:
    wxPanel* mDragArea{nullptr};
    Movable* mDragWindow{nullptr};
    std::unordered_map<wxWindow*, AdoptionRoutine> mAdoptionRoutines;
    std::vector<Movable*> mChildStack;

   wxPoint calcDelta(const wxPoint* mousePos);
   struct CalcData {
       wxPoint lastPos;
       bool reset{false};
   } mCalcData;
};

class MovePanel : public wxPanel {
public:
    MovePanel() = default;
    MovePanel(
            wxWindow* parent,
            MoveArea* moveArea,
            wxWindowID winID,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int64_t style = wxTAB_TRAVERSAL,
            const wxString& objName = "MovePanel");
    ~MovePanel() override;

    void create(
            wxWindow* parent, 
            MoveArea* moveArea, 
            wxWindowID winID, 
            const wxPoint& pos,
            const wxSize& size,
            int64_t style = wxTAB_TRAVERSAL,
            const wxString& objName = "MovePanel");

    [[nodiscard]] virtual bool hitTest(wxPoint) const;

protected:
    friend class MoveArea;
    friend class Movable;

    MoveArea* pMoveArea;
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
            wxWindowID winID,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int64_t style = wxTAB_TRAVERSAL,
            int64_t scrollStyle = wxScrolledWindowStyle,
            const wxString& objName = "Scrollable Move Panel");
    ~ScrolledMovePanel() override;

    wxScrolledCanvas* getCanvas();
    bool hitTest(wxPoint) const override;
    void scroll(const wxSize& deltaAmt);
    
protected:
    void dragTick(wxTimerEvent&);
    void shrinkToFit(int8_t ignoreDirs = 0);
    void checkSuppressRoutines();
    void restoreRoutine(Movable*);
    void suppressRoutine(Movable*, MoveArea::AdoptionRoutine = nullptr);

    enum : int8_t {
        UP          = 0b00000001,
        DOWN        = 0b00000010,
        LEFT        = 0b00000100,
        RIGHT       = 0b00001000,
        REPOS_VERT  = 0b00010000,
        REPOS_HORIZ = 0b00100000,
    };
    enum : int8_t {
        TIMER_DRAG
    };

    int32_t pJogDistance{20}; // NOLINT(readability-magic-numbers)

    bool pDragging{false};
    wxPoint pLastDragPos;
    std::unordered_map<Movable*, MoveArea::AdoptionRoutine> pSuppressedRoutines;

    wxScrolledCanvas* pCanvas{nullptr};
    wxTimer* pTimer{nullptr};
};


class Movable : public virtual wxWindow {
public:
    Movable(MoveArea* moveParent);
    ~Movable() override;

    [[nodiscard]] virtual bool hitTest(wxPoint) const;
    virtual void doOnGrab();
    void doGrab(wxMouseEvent& evt);
    MoveArea* getMoveArea();

protected:
    friend class MovePanel;
    friend class MoveArea;
    friend class ScrolledMovePanel;

    MoveArea::AdoptionRoutine pRoutine{nullptr};
    MoveArea* const pMoveArea{nullptr};
};

} // namespace pcui

