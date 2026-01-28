#include "base.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

#include <utility>

#include "ui/controls/checklist.h"
#include "ui/controls/choice.h"
#include "ui/controls/combobox.h"
#include "ui/controls/filepicker.h"
#include "ui/controls/numeric.h"
#include "ui/controls/radios.h"
#include "ui/controls/text.h"
#include "ui/controls/toggle.h"
#include "ui/controls/version.h"
#include "utils/demangle.h"

void pcui::ControlData::setUpdateHandler(
    function<void(uint32 id)>&& handler
) {
    mOnUpdate = std::move(handler);
}

void pcui::ControlData::enable(bool en) {
    std::scoped_lock scopeLock{getLock()};
    if (mEnabled == en) return;
    mEnabled = en;
    notify(eID_Active);
}

void pcui::ControlData::show(bool show, bool fit) {
    std::scoped_lock scopeLock{getLock()};
    if (mShown == show) return;
    mShown = show;
    if (fit) notify(eID_Visibility_Fit);
    else notify(eID_Visibility);
}

void pcui::ControlData::setFocus() {
    Notifier::notify(eID_Focus);
}

void pcui::ControlData::update(uint32 id) {
    if (mOnUpdate) mOnUpdate(id);
}

void pcui::ControlData::notify(uint32 id) {
    if (mOnUpdate) mOnUpdate(id);
    Notifier::notify(id);
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::ControlBase(
    wxWindow *parent,
    ControlData &data
) : wxPanel(
        parent,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxNO_BORDER,
        Utils::demangle(typeid(Derived).name())
    ),
    NotifyReceiver(this, data) {}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::ControlBase(
    wxWindow *parent,
    ControlDataProxy<ControlData>& proxy
) : wxPanel(
        parent,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxNO_BORDER,
        Utils::demangle(typeid(Derived).name())
    ),
    NotifyReceiver{this, proxy} {}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::init(
    Control *control,
    const wxEventTypeTag<ControlEvent>& eventTag,
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

    pControl->Bind(
        eventTag,
        [this](ControlEvent& evt) { controlEventHandler(evt); }
    );
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::init(
    Control *control,
    const wxEventTypeTag<ControlEvent>& eventTag,
    const wxEventTypeTag<SecondaryEvent>& secondaryTag,
    wxString label,
    wxOrientation orient
) {
    init(control, eventTag, std::move(label), orient);
    pControl->Bind(
        secondaryTag,
        [this](SecondaryEvent& evt) { secondaryEventHandler(evt); }
    );
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::SetToolTip(
    const wxString& tip
) {
    for (auto *child : GetChildren()) {
        child->SetToolTip(tip);
    }

    wxPanel::SetToolTip(tip);
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::SetMinSize(
    const wxSize& minSize, bool considerBest
) {
    mConsiderBest = considerBest;
    SetMinSize(minSize);
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::SetMinSize(
    const wxSize& minSize
) {
    mMinSize = minSize;
    refreshSizeAndLayout();
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
bool pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::Show(bool show) {
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
        if (static_cast<ControlDataProxy<ControlData> *>(proxy())->mShowWhenUnbound) {
            auto ret{wxPanel::Show()};
            refreshSizeAndLayout();
            return ret;
        }
        return wxPanel::Show(false);
    }

    assert(0);
    __builtin_unreachable();
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
> void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::refreshSizeAndLayout(
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
    // Fit();

    auto *parent{wxGetTopLevelParent(this)};
    if (parent) {
        parent->Layout();
        if (parentFit) parent->Fit();
    }
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::handleNotification(
    uint32 id
) {
    onUIUpdate(id);

    bool rebound{id == eID_Rebound};
    if (
            rebound or
            id == eID_Visibility or
            id == eID_Visibility_Fit
       ) {
        bool show{data()->isShown() and not mHidden};
        wxPanel::Show(show);
        bool fit{id == eID_Visibility_Fit};
        refreshSizeAndLayout(fit);
    }

    if (rebound or id == eID_Active) {
        Enable(data()->isEnabled());
    }

    if (id == eID_Focus) {
        pControl->SetFocus();
    }
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::handleUnbound() {
    onUnbound();

    Disable();
    if (proxy()) {
        auto *const ctrlProxy{
            static_cast<ControlDataProxy<ControlData> *>(proxy())
        };
        auto show{ctrlProxy->mShowWhenUnbound};
        wxPanel::Show(show);
        refreshSizeAndLayout();
    }
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::controlEventHandler(
    ControlEvent& evt
) {
    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};

    if (not data()->isEnabled() or data()->eventsInFlight()) {
        return;
    }

    onModify(evt);
    evt.Skip();
}

template<
    typename Derived,
    typename ControlData,
    typename Control,
    typename ControlEvent,
    typename SecondaryEvent
>
void pcui::ControlBase<
    Derived,
    ControlData,
    Control,
    ControlEvent,
    SecondaryEvent
>::secondaryEventHandler(
    SecondaryEvent& evt
) {
    if (not data()) return;
    std::scoped_lock scopeLock{data()->getLock()};

    if (not data()->isEnabled() or data()->eventsInFlight()) {
        return;
    }

    onModifySecondary(evt);
    evt.Skip();
}

template class pcui::ControlBase<
    pcui::CheckList,
	pcui::CheckListData,
	wxCheckListBox,
	wxCommandEvent
>;
template class pcui::ControlBase<
    pcui::Choice,
	pcui::ChoiceData,
	wxChoice,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::List,
	pcui::ChoiceData,
	wxListBox,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::ComboBox,
	pcui::ComboBoxData,
	wxComboBox,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::FilePicker,
	pcui::FilePickerData,
	wxFilePickerCtrl,
	wxFileDirPickerEvent
>;
template class pcui::ControlBase<
	pcui::Numeric,
	pcui::NumericData,
	wxSpinCtrl,
	wxSpinEvent,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::Decimal,
    pcui::DecimalData,
    wxSpinCtrlDouble,
	wxSpinDoubleEvent,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::Radios,
	pcui::RadiosData,
	pcui::StaticBox,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::Text,
	pcui::TextData,
	wxTextCtrl,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::CheckBox,
	pcui::ToggleData,
	wxCheckBox,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::Toggle,
	pcui::ToggleData,
	wxToggleButton,
	wxCommandEvent
>;
template class pcui::ControlBase<
	pcui::Version,
	pcui::VersionData,
	wxPanel,
	wxEvent
>;

