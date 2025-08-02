#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/controls/radios.h
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

#include <limits>

#include <wx/radiobox.h>

#include "base.h"
#include "../private/export.h"
#include "wx/arrstr.h"

namespace PCUI {

struct UI_EXPORT RadiosData : ControlData {
    RadiosData(uint32 numSelections);

    operator uint32() const { return mSelected; }
    /**
     * Efficient assign/update
     */
    void operator=(uint32 idx);
    /**
     * Unconditional assign/update
     */
    void setValue(uint32 idx);

    uint32 numSelections() const { return mEnabled.size(); }

    /**
     * Choices which are enabled will be shown regardless of their shown value
     */
    [[nodiscard]] const vector<bool>& enabledChoices() const { return mEnabled; }
    [[nodiscard]] const vector<bool>& shownChoices() const { return mShown; }

    void showChoice(uint32 idx, bool show = true);
    void enableChoice(uint32 idx, bool enable = true);

    enum {
        ID_SELECTION,
        ID_CHOICE_STATE,
    };

private:
    friend class Radios;
    friend class RadiosDataProxy;
    vector<bool> mEnabled;
    vector<bool> mShown;
    uint32 mSelected;
};

struct RadiosDataProxy : ControlDataProxy<RadiosData> {
    RadiosDataProxy(uint32 numSelections) : numSelections{numSelections} {}

    void bind(RadiosData& data) { 
        assert(numSelections == data.mEnabled.size());
        ControlDataProxy::bind(data);
    }

    const uint32 numSelections;
};

class UI_EXPORT Radios : public ControlBase<
                         Radios,
                         RadiosData,
                         wxRadioBox,
                         wxCommandEvent> {
public:
    Radios(
        wxWindow *parent,
        RadiosData& data,
        const wxArrayString& labels,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );
    Radios(
        wxWindow *parent,
        RadiosDataProxy& proxy,
        const wxArrayString& labels,
        const wxString& label = {},
        wxOrientation orient = wxVERTICAL
    );

private:
    void create(const wxArrayString&, const wxString&, wxOrientation);
    void onUIUpdate(uint32) final;
    void onModify(wxCommandEvent&) final;
};

} // namespace PCUI

