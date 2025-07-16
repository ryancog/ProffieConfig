#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/ui/controls.h
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


#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/control.h>
#include <wx/filepicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>

#include "utils/types.h"

#include "private/export.h"

namespace PCUI {

struct ControlData {
    ControlData& operator=(const ControlData&) = delete;
    ControlData& operator=(ControlData&&) = delete;

    /**
     * A callback to alert program-side code that the contained
     * value has been updated, either by program or by the user in UI
     */
    std::function<void(void)> onUpdate;

    /**
     * Enable/disable the UI control.
     */
    void enable(bool en = true) {
        mEnabled = en;
        pDirty = true; 
    }
    void disable() { enable(false); }

    /**
     * Show/hide the UI control.
     */
    void show(bool show = true) {
        mShown = show;
        pDirty = true;
    }
    void hide() { show(false); }

    bool isDirty() const { return pDirty; }

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
    bool pDirty{false};

    /**
     * Control can run this whenever it receives an event.
     *
     * @param valueUpdate function to update the control's data. 
     * Only run if needed.
     */
    void genericModifyHandler(function<void(void)>&& valueUpdate);

    /**
     * @return Whether or not the control needs to process its custom data.
     */
    bool genericUIUpdateHandler(wxWindow *);

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
        wxPanel(parent, wxID_ANY), pData{data} {}

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

        Enable(pData.isEnabled());
        Show(pData.isShown());
        onUIUpdate();

        Bind(wxEVT_UPDATE_UI, [this](wxUpdateUIEvent&) {
            if (not pData.isDirty()) return;

            Enable(pData.isEnabled());
            Show(pData.isShown());
            onUIUpdate();
        });
        Bind(eventTag, [this](CONTROL_EVENT& evt) {
            if (not pData.isEnabled() or pData.isDirty()) return;

            onModify(evt);
            if (pData.onUpdate) pData.onUpdate();
        });
    }

    virtual void onUIUpdate() = 0;
    virtual void onModify(CONTROL_EVENT&) = 0;

    CONTROL *pControl{nullptr};
    CONTROL_DATA& pData;
};

struct ToggleData : ControlData {
    operator bool() const { return mValue; }
    void operator=(bool val) {
        mValue = val;
        if (onUpdate) onUpdate();
        pDirty = true;
    }

private:
    friend class Toggle;
    friend class CheckBox;
    bool mValue;
};

class UI_EXPORT Toggle : public ControlBase<
                         Toggle,
                         ToggleData,
                         wxToggleButton,
                         wxCommandEvent> {
public:
    Toggle(
        wxWindow *parent,
        ToggleData& data,
        wxString onText = "True",
        wxString offText = "False",
        int64 style = 0,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;

    wxString mOnText;
    wxString mOffText;
};

class UI_EXPORT CheckBox : public ControlBase<
                           CheckBox,
                           ToggleData,
                           wxCheckBox,
                           wxCommandEvent> {
public:
    CheckBox(
        wxWindow *parent,
        ToggleData& data,
        int64 style = 0,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

struct ChoiceData : ControlData {
    operator int32() const { return mValue; }
    operator string() const { 
        if (mValue == -1) return {};
        return mChoices[mValue];
    }

    void operator=(int32 val) {
        mValue = val;
        pDirty = true;
    }

    const vector<string>& choices() const { return mChoices; }
    void setChoices(vector<string>&& choices) { 
        mChoices = std::move(choices); 
        pDirty = true;
    }

private:
    friend class Choice;
    vector<string> mChoices;
    int32 mValue{-1};
};

class UI_EXPORT Choice : public ControlBase<
                         Choice,
                         ChoiceData,
                         wxChoice,
                         wxCommandEvent> {
public:
    Choice(
        wxWindow *parent,
        ChoiceData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

struct ComboBoxData : ControlData {
    operator string() const { return mValue; }
    void operator=(string&& val) {
        mValue = std::move(val);
        pDirty = true;
    }

    const vector<string>& defaults() const { return mDefaults; }
    void setDefaults(vector<string>&& defaults) {
        mDefaults = std::move(defaults);
        pDirty = true;
    }

private:
    friend class ComboBox;
    vector<string> mDefaults;
    string mValue;
};

class UI_EXPORT ComboBox : public ControlBase<
                           ComboBox,
                           ComboBoxData,
                           wxComboBox,
                           wxCommandEvent> {
public:
    ComboBox(
        wxWindow *parent,
        ComboBoxData& data,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
};

struct NumericData : ControlData {
    operator int32() const { return mValue; }
    void operator=(int32 val) {
        mValue = val;
        pDirty = true;
    }

private:
    friend class Numeric;
    int32 mValue{0};
};

class UI_EXPORT Numeric : public ControlBase<
                          Numeric,
                          NumericData,
                          wxSpinCtrl,
                          wxSpinEvent> {
public:
    Numeric(
        wxWindow *parent,
        NumericData& data,
        int32 min       = 0,
        int32 max       = 100,
        int32 increment = 1,
        int64 style = wxSP_ARROW_KEYS,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
    );

private:
    void onUIUpdate() final;
    void onModify(wxSpinEvent&) final;
};

struct DecimalData : ControlData {
    operator float64() const { return mValue; }
    void operator=(float64 val) {
        mValue = val;
        pDirty = true;
    }

private:
    friend class Decimal;
    float64 mValue;
};

class UI_EXPORT Decimal : public ControlBase<
                          Decimal,
                          DecimalData,
                          wxSpinCtrlDouble,
                          wxSpinDoubleEvent> {
public:
    Decimal(
        wxWindow *parent,
        DecimalData& data,
        float64 min       = 0,
        float64 max       = 100,
        float64 increment = 1,
        int64 style = wxSP_ARROW_KEYS,
        const wxString& label = {},
        const wxOrientation& orient = wxVERTICAL
        );

private:
    void onUIUpdate() final;
    void onModify(wxSpinDoubleEvent&) final;
};

struct TextData : ControlData {
    operator string() { return mValue; }
    void operator=(string&& val) {
        mValue = std::move(val);
        pDirty = true;
    }

private:
    friend class Text;
    string mValue;
};

class UI_EXPORT Text : public ControlBase<
                       Text,
                       TextData,
                       wxTextCtrl,
                       wxCommandEvent> {
public:
    Text(
        wxWindow *parent,
        TextData& data,
        int64 style = 0,
        const wxString &label = {},
        wxOrientation orient = wxVERTICAL
        );

    // TODO: Set up use of validators to forbid certain entry.
    //
    // Prevent any chars in the given wxString from being entered.
    // void setInvalidChars(const wxString&);

    // [[nodiscard]] wxString getInvalidChars() const;

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
    // void pruneText();

    // wxString mInvalidChars;
};

struct FilePickerData : ControlData {
    operator filepath() { return mValue; }
    void operator=(filepath&& val) {
        mValue = std::move(val);
        pDirty = true;
    }

private:
    friend class FilePicker;
    filepath mValue;
};

class UI_EXPORT FilePicker : public ControlBase<
                             FilePicker,
                             FilePickerData,
                             wxFilePickerCtrl,
                             wxFileDirPickerEvent> {
public:
    FilePicker(
        wxWindow *parent,
        FilePickerData& data,
        int64 style,
        const wxString& prompt = {},
        const wxString& wildcard = {},
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );
    
private:
    void onUIUpdate() final;
    void onModify(wxFileDirPickerEvent&) final;
};

} // namespace PCUI

