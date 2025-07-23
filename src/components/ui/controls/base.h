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

#include "ui/notifier.h"
#include "utils/types.h"

namespace PCUI {

template<
    class DERIVED,
    typename CONTROL_DATA,
    class CONTROL,
    class CONTROL_EVENT
>
class ControlBase;

struct UI_EXPORT ControlData : NotifierData {
    ControlData(const ControlData&) = delete;
    ControlData(ControlData&&) = delete;
    ControlData& operator=(const ControlData&) = delete;
    ControlData& operator=(ControlData&&) = delete;

    enum EventID {
        // Show/hide
        ID_VISIBILITY = 1000,
        // Enable/disable
        ID_ACTIVE
    };

    void setUpdateHandler(function<void(uint32 id)>&& handler);

    /**
     * Enable/disable the UI control.
     */
    void enable(bool en = true);
    void disable() { enable(false); }

    /**
     * Show/hide the UI control.
     */
    void show(bool show = true);
    void hide() { show(false); }

    [[nodiscard]] bool isEnabled() const { return mEnabled; }
    [[nodiscard]] bool isShown() const { return mShown; }

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

template<typename DATA_TYPE> requires std::is_base_of_v<ControlData, DATA_TYPE>
struct ControlDataProxy : NotifierDataProxy {
    void bind(DATA_TYPE& data) { NotifierDataProxy::bind(&data); }
    void unbind() { NotifierDataProxy::bind(nullptr); }
    DATA_TYPE *data() { return static_cast<DATA_TYPE *>(NotifierDataProxy::data()); };
};

template<
    class DERIVED,
    typename CONTROL_DATA,
    class CONTROL,
    class CONTROL_EVENT
>
class ControlBase : public wxPanel, protected Notifier {
public:
    static_assert(std::is_base_of_v<wxControl, CONTROL>, "PCUI Control core must be wxControl descendant");
    static_assert(std::is_base_of_v<ControlData, CONTROL_DATA>, "PCUI Control data must be ControlData descendant");

    void setToolTip(wxToolTip *tip);

protected:
    ControlBase(wxWindow *parent, CONTROL_DATA &data) : 
        wxPanel(parent, wxID_ANY), Notifier(this, data) {}
    ControlBase(wxWindow *parent, ControlDataProxy<CONTROL_DATA>& proxy) : 
        wxPanel(parent, wxID_ANY), Notifier{this, proxy} {}

    void init(
        CONTROL *control,
        const wxEventTypeTag<CONTROL_EVENT>& eventTag,
        wxString label,
        wxOrientation orient
    );
    void init(
        CONTROL *control,
        const wxEventTypeTag<CONTROL_EVENT>& eventTag,
        const wxEventTypeTag<CONTROL_EVENT>& secondaryTag,
        wxString label,
        wxOrientation orient
    );

    virtual void onUIUpdate(uint32 id) = 0;
    /**
     * Must use the data update function to signal
     * Data is already locked.
     */
    virtual void onModify(CONTROL_EVENT&) = 0;

    CONTROL *pControl{nullptr};
    CONTROL_DATA *data() { return static_cast<CONTROL_DATA *>(Notifier::data()); }

private:
    void handleNotification(uint32 id) final;
    void handleUnbound() final;
    void controlEventHandler(CONTROL_EVENT& evt);
};

} // namespace PCUI
