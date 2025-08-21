#include "base.h"

#include <utility>
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
#include "ui/controls/version.h"

void PCUI::ControlData::setUpdateHandler(function<void(uint32 id)>&& handler) {
    mOnUpdate = std::move(handler);
}

void PCUI::ControlData::enable(bool en) {
    std::scoped_lock scopeLock{getLock()};
    if (mEnabled == en) return;
    mEnabled = en;
    notify(ID_ACTIVE);
}

void PCUI::ControlData::show(bool show, bool fit) {
    std::scoped_lock scopeLock{getLock()};
    if (mShown == show) return;
    mShown = show;
    if (fit) notify(ID_VISIBILITY_FIT);
    else notify(ID_VISIBILITY);
}

void PCUI::ControlData::setFocus() {
    Notifier::notify(ID_FOCUS);
}

void PCUI::ControlData::update(uint32 id) {
    if (mOnUpdate) mOnUpdate(id);
}

void PCUI::ControlData::notify(uint32 id) {
    if (mOnUpdate) mOnUpdate(id);
    Notifier::notify(id);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::ControlBase(
    wxWindow *parent,
    CONTROL_DATA &data
) : wxPanel(parent, wxID_ANY), NotifyReceiver(this, data) {}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::ControlBase(
    wxWindow *parent,
    ControlDataProxy<CONTROL_DATA>& proxy
) : wxPanel(parent, wxID_ANY), NotifyReceiver{this, proxy} {}

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
        sizer->Add(
            new wxStaticText(this, wxID_ANY, label),
            orient == wxHORIZONTAL ?
                wxSizerFlags().Border(wxRIGHT, PADDING).Center() :
                wxSizerFlags().Border(wxLEFT | wxRIGHT, PADDING)
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

    pControl->Bind(eventTag, [this](CONTROL_EVENT& evt) { controlEventHandler(evt); });
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::init(
    CONTROL *control,
    const wxEventTypeTag<CONTROL_EVENT>& eventTag,
    const wxEventTypeTag<SECONDARY_EVENT>& secondaryTag,
    wxString label,
    wxOrientation orient
) {
    init(control, eventTag, std::move(label), orient);
    pControl->Bind(secondaryTag, [this](SECONDARY_EVENT& evt) { secondaryEventHandler(evt); });
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
    const wxSize& minSize, bool considerBest
) {
    mConsiderBest = considerBest;
    SetMinSize(minSize);
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
        if (data()->isShown()) {
            auto ret{wxPanel::Show()};
            refreshSizeAndLayout();
            return ret;
        }
        return wxPanel::Show(false);
    } 

    if (proxy()) {
        if (static_cast<ControlDataProxy<CONTROL_DATA> *>(proxy())->mShowWhenUnbound) {
            auto ret{wxPanel::Show()};
            refreshSizeAndLayout();
            return ret;
        }
        return wxPanel::Show(false);
    }

    assert(0);
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::refreshSizeAndLayout(
    bool parentFit
) {
    wxPanel::SetMinSize({-1, -1});
    pControl->SetMinSize({-1, -1});

    wxSize newSize{GetBestSize()};
    if (not mConsiderBest) newSize.DecToIfSpecified(mMinSize);
    else newSize.IncTo(mMinSize);
    newSize.DecToIfSpecified(GetMaxSize());
    wxPanel::SetMinSize(newSize);

    Layout();
    Fit();
    
    auto *parent{wxGetTopLevelParent(this)};
    if (parent) {
        parent->Layout();
        if (parentFit) parent->Fit();
    }
}


template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::handleNotification(
    uint32 id
) {
    onUIUpdate(id);

    bool rebound{id == ID_REBOUND};
    if (rebound or id == ControlData::ID_VISIBILITY or id == ControlData::ID_VISIBILITY_FIT) {
        bool show{data()->isShown() and not mHidden};
        wxPanel::Show(show);
        refreshSizeAndLayout(id == ControlData::ID_VISIBILITY_FIT);
    }
    if (rebound or id == ControlData::ID_ACTIVE) Enable(data()->isEnabled());
    if (id == ControlData::ID_FOCUS) pControl->SetFocus();
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::handleUnbound() {
    onUnbound();

    Disable();
    if (proxy()) {
        auto show{static_cast<ControlDataProxy<CONTROL_DATA> *>(proxy())->mShowWhenUnbound};
        wxPanel::Show(show);
        refreshSizeAndLayout();
    }
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::controlEventHandler(
    CONTROL_EVENT& evt
) {
    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};
    if (not data()->isEnabled() or data()->eventsInFlight()) return;

    onModify(evt);
    evt.Skip();
}

template<class DERIVED, typename CONTROL_DATA, class CONTROL, class CONTROL_EVENT, class SECONDARY_EVENT>
void PCUI::ControlBase<DERIVED, CONTROL_DATA, CONTROL, CONTROL_EVENT, SECONDARY_EVENT>::secondaryEventHandler(
    SECONDARY_EVENT& evt
) {
    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};
    if (not data()->isEnabled() or data()->eventsInFlight()) return;

    onModifySecondary(evt);
    evt.Skip();
}

template class PCUI::ControlBase<PCUI::CheckList, PCUI::CheckListData, wxCheckListBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Choice, PCUI::ChoiceData, wxChoice, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::List, PCUI::ChoiceData, wxListBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::ComboBox, PCUI::ComboBoxData, wxComboBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::FilePicker, PCUI::FilePickerData, wxFilePickerCtrl, wxFileDirPickerEvent>;
template class PCUI::ControlBase<PCUI::Numeric, PCUI::NumericData, wxSpinCtrl, wxSpinEvent, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Decimal, PCUI::DecimalData, wxSpinCtrlDouble, wxSpinDoubleEvent, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Radios, PCUI::RadiosData, PCUI::StaticBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Text, PCUI::TextData, wxTextCtrl, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::CheckBox, PCUI::ToggleData, wxCheckBox, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Toggle, PCUI::ToggleData, wxToggleButton, wxCommandEvent>;
template class PCUI::ControlBase<PCUI::Version, PCUI::VersionData, wxPanel, wxEvent>;

