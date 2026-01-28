#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

#include "ui/notifier.h"
#include "utils/types.h"

namespace pcui {

enum {
    /**
     * Show/hide
     */
    eID_Visibility = 1000,
    /**
     * Show/hide and fit parent
     */
    eID_Visibility_Fit,
    /**
     * Enable/disable
     */
    eID_Active,
    /**
     * Focus control. Not held in state, cannot be hooked by update handler.
     */
    eID_Focus,
};

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
class ControlBase;

struct UI_EXPORT ControlData : Notifier {
    ControlData(const ControlData&) = delete;
    ControlData(ControlData&&) = delete;
    ControlData& operator=(const ControlData&) = delete;
    ControlData& operator=(ControlData&&) = delete;

    void setUpdateHandler(function<void(uint32 id)>&& handler);

    /**
     * Enable/disable the UI control.
     */
    void enable(bool en = true);
    void disable() { enable(false); }

    /**
     * Show/hide the UI control.
     */
    void show(bool show = true, bool parentFit = false);
    void hide() { show(false); }

    [[nodiscard]] bool isEnabled() const { return mEnabled; }
    [[nodiscard]] bool isShown() const { return mShown; }

    void setFocus();

protected:
    ControlData() = default;

    /**
     * Used by derived friends to update from UI
     */
    void update(uint32 id);

    /**
     * Used by derived to signal to UI
     */
    void notify(uint32 id);

private:
    template<typename DataType> requires std::is_base_of_v<ControlData, DataType>
    friend struct ControlDataProxy;
    /**
     * A callback to alert program-side code that the contained
     * value has been updated, either by program or by the user in UI
     */
    function<void(uint32 id)> mOnUpdate;

    /**
     * UI Control states
     */
    bool mEnabled{true};
    bool mShown{true};
};

template<typename DataType> requires std::is_base_of_v<ControlData, DataType>
struct ControlDataProxy : NotifierProxy {
    void bind(DataType& data) {
        NotifierProxy::bind(&data);
        data.update(eID_Rebound);
    }
    void unbind() { NotifierProxy::bind(nullptr); }
    DataType *data() { return static_cast<DataType *>(NotifierProxy::data()); };

    /**
     * Whether to show the control whenever its proxy (this) is unbound.
     *
     * Normally the control is simply disabled.
     * Takes effect on next unbind, NOT immediately
     */
    void showWhenUnbound(bool show) { mShowWhenUnbound = show; }

private:
    template<typename, typename, typename, typename, typename>
    friend class ControlBase;
    bool mShowWhenUnbound{true};
};

struct StaticBox;

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent = ControlEvent
>
class UI_EXPORT ControlBase : public wxPanel, protected NotifyReceiver {
public:
    static_assert(std::is_base_of_v<wxWindow, Control> or std::is_same_v<pcui::StaticBox, Control>, "pcui Control core must be wxWindow descendant");
    static_assert(std::is_base_of_v<ControlData, ControlData>, "pcui Control data must be ControlData descendant");

    void SetToolTip(const wxString&);
    void SetMinSize(const wxSize&) final;
    /**
     * @param obeyControl may be set false to ignore the best size
     */
    void SetMinSize(const wxSize&, bool considerBest);

    /**
     * Should not be used directly. Use the data or proxy methods for show/hide
     *
     * Implemented for sizer/parent use.
     */
    bool Show(bool show = true) final;

protected:
    ControlBase(wxWindow *parent, ControlData &data);
    ControlBase(wxWindow *parent, ControlDataProxy<ControlData>& proxy);

    void init(
        Control *control,
        const wxEventTypeTag<ControlEvent>& eventTag,
        wxString label,
        wxOrientation orient
    );
    void init(
        Control *control,
        const wxEventTypeTag<ControlEvent>& eventTag,
        const wxEventTypeTag<SecondaryEvent>& secondaryTag,
        wxString label,
        wxOrientation orient
    );

    virtual void onUIUpdate(uint32 id) = 0;

    /**
     * Controls may override to set themselves into a sane state while unbound and disabled
     */
    virtual void onUnbound() {};

    /**
     * Must use the data update function to signal
     * Data is already locked.
     */
    virtual void onModify(ControlEvent&) = 0;

    virtual void onModifySecondary(SecondaryEvent&) {}

    /**
     * @param parentFit fit top level parent to best size
     */
    void refreshSizeAndLayout(bool parentFit = false);

    Control *pControl{nullptr};
    ControlData *data() { return static_cast<ControlData *>(NotifyReceiver::data()); }

private:
    void handleNotification(uint32 id) final;
    void handleUnbound() final;
    void controlEventHandler(ControlEvent& evt);
    void secondaryEventHandler(SecondaryEvent& evt);

    wxSize mMinSize;
    bool mConsiderBest{true};
    bool mHidden{false};
};

} // namespace pcui
