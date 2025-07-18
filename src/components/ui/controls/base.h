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

    void setUpdateHandler(function<void(void)>&& handler) {
        mOnUpdate = std::move(handler);
    }

    /**
     * Enable/disable the UI control.
     */
    void enable(bool en = true) {
        if (mEnabled == en) return;
        mEnabled = en;
        refresh();
    }
    void disable() { enable(false); }

    /**
     * Show/hide the UI control.
     */
    void show(bool show = true) {
        if (mShown == show) return;
        mShown = show;
        refresh();
    }
    void hide() { show(false); }

    /**
     * If data has been updated since the UI has last seen it
     */
    bool isNew() const { return mNew; }
    /**
     * Mark data new
     */
    void refresh(bool logic = true, bool ui = true) { 
        if (logic and mOnUpdate) [[likely]] mOnUpdate();
        if (ui) [[likely]] mNew = true; 
    }


    [[nodiscard]] bool isEnabled() const { return mEnabled; }
    [[nodiscard]] bool isShown() const { return mShown; }

protected:
    ControlData() = default;
    ControlData(const ControlData&) = delete;
    ControlData(ControlData&&) = default;

    /**
     * Only for UI elements to mark they've handled the new data
     */
    void refreshed() { mNew = false; }

private:
    /**
     * A callback to alert program-side code that the contained
     * value has been updated, either by program or by the user in UI
     */
    function<void(void)> mOnUpdate;

    /**
     * UI Control states
     */
    bool mEnabled{true};
    bool mShown{true};
    /**
     * Set true whenever the user modifies the data.
     *
     * Windows can use UpdateWindowUI() to force updates for dirty data.
     */
    bool mNew{false};
};

template<typename DATA_TYPE> requires std::is_base_of_v<ControlData, DATA_TYPE>
struct ControlDataProxy {
    DATA_TYPE *data;
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
    ControlBase(wxWindow *parent, ControlDataProxy<CONTROL_DATA>& proxy) : 
        wxPanel(parent, wxID_ANY), mDataProxy{proxy} { bind(mDataProxy->data); }

    ControlBase(wxWindow *parent, CONTROL_DATA &data) : 
        wxPanel(parent, wxID_ANY) { bind(&data); }

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
            if (mDataProxy and mDataProxy->data != pData) {
                bind(mDataProxy->data);
            }
            
            if (not pData) {
                Disable();
                Hide();
                return;
            }

            if (not pData->isNew()) return;

            Enable(pData->isEnabled());
            Show(pData->isShown());
            onUIUpdate();
        });
        Bind(eventTag, [this](CONTROL_EVENT& evt) { controlEventHandler(evt); });
    }

    void init(
        CONTROL *control,
        const wxEventTypeTag<CONTROL_EVENT>& eventTag,
        const wxEventTypeTag<CONTROL_EVENT>& secondaryTag,
        wxString label,
        wxOrientation orient
    ) {
        init(control, eventTag, label, orient);
        Bind(secondaryTag, [this](CONTROL_EVENT& evt) { controlEventHandler(evt); });
    }

    virtual void onUIUpdate() = 0;
    virtual void onModify(CONTROL_EVENT&) = 0;

    CONTROL *pControl{nullptr};
    CONTROL_DATA *pData{nullptr};

private:
    void controlEventHandler(CONTROL_EVENT& evt) {
        if (not pData or not pData->isEnabled() or pData->isNew()) return;

        onModify(evt);
        pData->refresh(true, false);
    }

    void bind(CONTROL_DATA *newData) {
        pData = newData;
        if (pData) pData->refresh(false);
    };

    ControlDataProxy<CONTROL_DATA> *mDataProxy{nullptr};
};

} // namespace PCUI
