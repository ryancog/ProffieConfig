#include "base.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/base.cpp
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

#include "ui/controls/checklist.h"
#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/filepicker.h"
#include "ui/controls/numeric.h"
#include "ui/controls/radios.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"
#include "wx/window.h"
#include <mutex>

void PCUI::ControlData::setUpdateHandler(function<void(uint32 id)>&& handler) {
    mOnUpdate = std::move(handler);
}

void PCUI::ControlData::enable(bool en) {
    std::scoped_lock scopeLock{getLock()};
    if (mEnabled == en) return;
    mEnabled = en;
    notify(ID_ACTIVE);
}

void PCUI::ControlData::show(bool show) {
    std::scoped_lock scopeLock{getLock()};
    if (mShown == show) return;
    mShown = show;
    notify(ID_VISIBILITY);
}

void PCUI::ControlData::update(uint32 id) {
    if (mOnUpdate) mOnUpdate(id);
}

void PCUI::ControlData::notify(uint32 id) {
    if (mOnUpdate) mOnUpdate(id);
    NotifierData::notify(id);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT>::init(
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

    if (Notifier::data()) {
        Notifier::data()->notify(ID_REBOUND);
    } else {
        handleUnbound();
    }

    Bind(eventTag, [this](CONTROL_EVENT& evt) { controlEventHandler(evt); });
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT>::init(
    CONTROL *control,
    const wxEventTypeTag<CONTROL_EVENT>& eventTag,
    const wxEventTypeTag<CONTROL_EVENT>& secondaryTag,
    wxString label,
    wxOrientation orient
) {
    init(control, eventTag, label, orient);
    Bind(secondaryTag, [this](CONTROL_EVENT& evt) { controlEventHandler(evt); });
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT>::handleNotification(uint32 id) {
    bool rebound{id == ID_REBOUND};
    if (rebound or id == ControlData::ID_VISIBILITY) Show(data()->isShown());
    if (rebound or id == ControlData::ID_ACTIVE) Enable(data()->isEnabled());
    onUIUpdate(id);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT>::handleUnbound() {
    Disable();
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT>::controlEventHandler(CONTROL_EVENT& evt) {
    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};
    if (not data()->isEnabled() or data()->eventsInFlight()) return;

    onModify(evt);
}

template class PCUI::ControlBase<PCUI::CheckList, PCUI::CheckListData, wxCheckListBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Choice, PCUI::ChoiceData, wxChoice, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::List, PCUI::ChoiceData, wxListBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::ComboBox, PCUI::ComboBoxData, wxComboBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::FilePicker, PCUI::FilePickerData, wxFilePickerCtrl, wxFileDirPickerEvent>;
template class PCUI::ControlBase<PCUI::Numeric, PCUI::NumericData, wxSpinCtrl, wxSpinEvent>;
template class PCUI::ControlBase<PCUI::Decimal, PCUI::DecimalData, wxSpinCtrlDouble, wxSpinDoubleEvent>;
template class PCUI::ControlBase<PCUI::Radios, PCUI::RadiosData, wxRadioBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Text, PCUI::TextData, wxTextCtrl, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::CheckBox, PCUI::ToggleData, wxCheckBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Toggle, PCUI::ToggleData, wxToggleButton, wxCommandEvent>;

