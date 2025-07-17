#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * components/ui/controls/controls.h
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

#include "../private/export.h"
#include "controldata.h"

namespace PCUI {

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

class UI_EXPORT Button : public ControlBase<
                         Button,
                         ButtonData,
                         wxButton,
                         wxCommandEvent> {
public:
    Button(
        wxWindow *parent,
        ButtonData& data,
        int64 style = 0,
        const wxString& label = {}
    );

private:
    void onUIUpdate() final;
    void onModify(wxCommandEvent&) final;
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

