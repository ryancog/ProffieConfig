#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <initializer_list>
#include <utility>

#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>

#include "../../core/config/configuration.h"

template <typename STRING>
constexpr void trimWhiteSpace(STRING& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char chr) {
        return not std::isspace(chr);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char chr) {
        return not std::isspace(chr);
    }).base(), str.end());
};

class Misc {
public:
    class MessageBoxEvent;

    static const wxEventTypeTag<wxCommandEvent> EVT_MSGBOX;

    static wxArrayString createEntries(const std::vector<wxString>& vec);
    static wxArrayString createEntries(const std::initializer_list<wxString>& list);
    static wxArrayString createEntries(const Configuration::VMap& map);

    template<typename T, size_t SIZE>
    static wxArrayString createEntries(const array<T, SIZE>& list) {
        wxArrayString entries;
        for (const auto& entry : list) {
            entries.Add(wxString{entry});
        }
        return entries;
    }

private:
    Misc();
};

class Misc::MessageBoxEvent : public wxCommandEvent {
public:
    MessageBoxEvent(int32_t id, wxString _message, wxString _caption, long _style = wxOK | wxCENTER){
        this->SetEventType(EVT_MSGBOX);
        this->SetId(id);
        caption = std::move(_caption);
        message = std::move(_message);
        style = _style;
    }

    wxString caption;
    wxString message;
    long style;
};
