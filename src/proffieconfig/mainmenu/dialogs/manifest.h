#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/dialog.h>

#include "../mainmenu.h"
#include "ui/controls/controldata.h"

class ManifestDialog : public wxDialog {
public:
    ManifestDialog(MainMenu *);

private:
    PCUI::TextData mEntryData;
};
