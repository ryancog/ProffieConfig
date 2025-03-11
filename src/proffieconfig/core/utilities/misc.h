#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../core/config/configuration.h"

#include <initializer_list>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>

template <typename STRING>
constexpr void trimWhiteSpace(STRING& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char ch) {
        return not std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return not std::isspace(ch);
    }).base(), str.end());
};

class Misc {
public:
    class MessageBoxEvent;

    static wxEventTypeTag<wxCommandEvent> EVT_MSGBOX;

    static const wxArrayString createEntries(const std::vector<wxString>& list);
    static const wxArrayString createEntries(const std::initializer_list<wxString>& list);
    static const wxArrayString createEntries(const Configuration::VMap& map);

    template<typename T, size_t size>
    static const wxArrayString createEntries(const std::array<T, size>& list) {
        wxArrayString entries;
        for (const wxString& entry : list) {
            entries.Add(entry);
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
        caption = _caption;
        message = _message;
        style = _style;
    }

    wxString caption;
    wxString message;
    long style;
};
