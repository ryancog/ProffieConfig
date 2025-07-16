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

class Misc {
public:
    class MessageBoxEvent;

    static const wxEventTypeTag<wxCommandEvent> EVT_MSGBOX;

    static vector<string> createEntries(const std::vector<wxString>& vec);
    static vector<string> createEntries(const std::initializer_list<wxString>& list);
    static vector<string> createEntries(const Configuration::VMap& map);

    template<typename T, size_t SIZE>
    static vector<string> createEntries(const array<T, SIZE>& list) {
        vector<string> entries;
        for (const auto& entry : list) {
            entries.push_back(string{entry});
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
