#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/base.h
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

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "utils/types.h"

namespace PCUI {

struct ControlData {
    ControlData& operator=(const ControlData&) = delete;
    ControlData& operator=(ControlData&&) = delete;

    /**
     * A callback to alert program-side code that the contained
     * value has been updated, either by program or by the user in UI
     */
    function<void(void)> onUpdate;

    /**
     * Enable/disable the UI control.
     */
    void enable(bool en = true) {
        mEnabled = en;
        pNew = true; 
    }
    void disable() { enable(false); }

    /**
     * Show/hide the UI control.
     */
    void show(bool show = true) {
        mShown = show;
        pNew = true;
    }
    void hide() { show(false); }

    /**
     * If data has been updated since the UI has last seen it
     */
    bool isNew() const { return pNew; }
    /**
     * Mark data new
     */
    void refresh() { pNew = true; }

    [[nodiscard]] bool isEnabled() const { return mEnabled; }
    [[nodiscard]] bool isShown() const { return mShown; }

protected:
    ControlData() = default;
    ControlData(const ControlData&) = delete;
    ControlData(ControlData&&) = default;

    /**
     * Set true whenever the user modifies the data.
     *
     * Windows can use UpdateWindowUI() to force updates for dirty data.
     */
    bool pNew{false};

private:
    /**
     * UI Control states
     */
    bool mEnabled{true};
    bool mShown{true};
};

template<
    class DERIVED,
    typename CONTROL_DATA,
    class CONTROL,
    class CONTROL_EVENT
>
class ControlBase : public wxPanel {
public:
    static_assert(std::is_base_of_v<wxControl, CONTROL>, "PCUI Control core must be wxControl descendant");
    static_assert(std::is_base_of_v<ControlData, CONTROL_DATA>, "PCUI Control data must be ControlData descendant");

    operator wxWindow *() { return this; }

    void setToolTip(wxToolTip *tip);

    // Hide these for now until I decide we really need them.
    // [[nodiscard]] inline constexpr CONTROL *entry() { return mEntry; }
    // [[nodiscard]] inline constexpr const CONTROL *entry() const { return mEntry; }
    // [[nodiscard]] inline constexpr wxStaticText *text() { return mText; }
    // [[nodiscard]] inline constexpr const wxStaticText *text() const { return mText; }

protected:
    ControlBase(wxWindow *parent, CONTROL_DATA& data) : 
        wxPanel(parent, wxID_ANY) { bind(data); }

    void init(
        CONTROL *control,
        const wxEventTypeTag<CONTROL_EVENT>& eventTag,
        wxString label,
        wxOrientation orient
    ) {
        auto *sizer{new wxBoxSizer(orient)};
        constexpr auto PADDING{5};

        pControl = control;

        if (not label.empty()) {
            auto sizerFlags{
                wxSizerFlags(0).Border(wxLEFT | wxRIGHT, PADDING)
            };
            sizer->Add(
                new wxStaticText(this, wxID_ANY, label),
                orient == wxHORIZONTAL ? sizerFlags.Center() : sizerFlags
            );
        }

        sizer->Add(control, wxSizerFlags(1).Expand());
        SetSizerAndFit(sizer);

        Bind(wxEVT_UPDATE_UI, [this](wxUpdateUIEvent&) {
            if (not pData->isNew()) return;

            Enable(pData->isEnabled());
            Show(pData->isShown());
            onUIUpdate();
        });
        Bind(eventTag, [this](CONTROL_EVENT& evt) {
            if (not pData->isEnabled() or pData->isNew()) return;

            onModify(evt);
            if (pData->onUpdate) pData->onUpdate();
        });
    }

    void bind(CONTROL_DATA& newData) {
        pData = newData;
        pData->refresh();
    };

    virtual void onUIUpdate() = 0;
    virtual void onModify(CONTROL_EVENT&) = 0;

    CONTROL *pControl{nullptr};
    // Never nullptr
    CONTROL_DATA *pData;
};

} // namespace PCUI
