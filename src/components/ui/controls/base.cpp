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

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::ControlBase(
    wxWindow *parent,
    CONTROL_DATA &data
) : wxPanel(parent, wxID_ANY), Notifier(this, data) {}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::ControlBase(
    wxWindow *parent,
    ControlDataProxy<CONTROL_DATA>& proxy
) : wxPanel(parent, wxID_ANY), Notifier{this, proxy} {}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::init(
    CONTROL *control,
    const wxEventTypeTag<CONTROL_EVENT>& eventTag,
    wxString label,
    wxOrientation orient
) {
    auto *sizer{new wxBoxSizer(orient)};

    pControl = control;

    if (not label.empty()) {
        constexpr auto PADDING{5};
        auto sizerFlags{
            wxSizerFlags().Border(wxLEFT | wxRIGHT, PADDING)
        };
        sizer->Add(
            new wxStaticText(this, wxID_ANY, label),
            orient == wxHORIZONTAL ? sizerFlags.Center() : sizerFlags
        );
    }

    wxSizerFlags controlFlags;
    if (orient == wxHORIZONTAL) {
        controlFlags.Proportion(1).Center();
    } else {
        controlFlags.Proportion(1).Expand();
    }
    sizer->Add(control, controlFlags);

    SetSizer(sizer);

    initializeNotifier();

    Bind(eventTag, [this](CONTROL_EVENT& evt) { controlEventHandler(evt); });
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::init(
    CONTROL *control,
    const wxEventTypeTag<CONTROL_EVENT>& eventTag,
    const wxEventTypeTag<SECONDARY_EVENT>& secondaryTag,
    wxString label,
    wxOrientation orient
) {
    init(control, eventTag, label, orient);
    Bind(secondaryTag, [this](SECONDARY_EVENT& evt) { secondaryEventHandler(evt); });
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::SetToolTip(
    const wxString& tip
) {
    for (auto *child : GetChildren()) {
        child->SetToolTip(tip);
    }

    wxPanel::SetToolTip(tip);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::SetMinSize(
    const wxSize& minSize
) {
    mMinSize = minSize;
    refreshSizeAndLayout();
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
bool PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::Show(bool show) {
    if (mHidden == not show) return false;

    mHidden = not show;

    if (mHidden) {
        return wxPanel::Show(false);
    } 

    if (data()) {
        return wxPanel::Show(data()->isShown());
    } else if (proxy()) {
        return wxPanel::Show(static_cast<ControlDataProxy<CONTROL_DATA> *>(proxy())->mShowWhenUnbound);
    }
    assert(0);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::refreshSizeAndLayout() {
    wxPanel::SetMinSize({-1, -1});
    auto newSize{GetBestSize()};
    newSize.IncTo(mMinSize);
    wxPanel::SetMinSize(newSize);

    pControl->SetMinSize({-1, -1});
    pControl->SetSize(pControl->GetBestSize());

    Layout();
    Fit();
    
    auto *parent{wxGetTopLevelParent(this)};
    if (parent) {
        parent->Layout();
    }
}


template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::handleNotification(
    uint32 id
) {
    bool rebound{id == ID_REBOUND};
    if (rebound or id == ControlData::ID_VISIBILITY) {
        wxPanel::Show(data()->isShown() and not mHidden);
        refreshSizeAndLayout();
    }
    if (rebound or id == ControlData::ID_ACTIVE) Enable(data()->isEnabled());
    onUIUpdate(id);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::handleUnbound() {
    Disable();
    if (proxy()) {
        wxPanel::Show(static_cast<ControlDataProxy<CONTROL_DATA> *>(proxy())->mShowWhenUnbound);
    }
    onUnbound();
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::controlEventHandler(
    CONTROL_EVENT& evt
) {

    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};
    if (not data()->isEnabled() or data()->eventsInFlight()) return;

    onModify(evt);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::secondaryEventHandler(
    SECONDARY_EVENT& evt
) {
    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};
    if (not data()->isEnabled() or data()->eventsInFlight()) return;

    onModifySecondary(evt);
}

template class PCUI::ControlBase<PCUI::CheckList, PCUI::CheckListData, wxCheckListBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Choice, PCUI::ChoiceData, wxChoice, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::List, PCUI::ChoiceData, wxListBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::ComboBox, PCUI::ComboBoxData, wxComboBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::FilePicker, PCUI::FilePickerData, wxFilePickerCtrl, wxFileDirPickerEvent>;
template class PCUI::ControlBase<PCUI::Numeric, PCUI::NumericData, wxSpinCtrl, wxSpinEvent, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Decimal, PCUI::DecimalData, wxSpinCtrlDouble, wxSpinDoubleEvent, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Radios, PCUI::RadiosData, wxRadioBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Text, PCUI::TextData, wxTextCtrl, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::CheckBox, PCUI::ToggleData, wxCheckBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Toggle, PCUI::ToggleData, wxToggleButton, wxCommandEvent>;

