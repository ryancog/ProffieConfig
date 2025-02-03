// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#pragma once

#include "ui/pcchoice.h"

#include <wx/frame.h>
#include <wx/sizer.h>

// Forward declarations to get around circular dependencies
class GeneralPage;
class PropsPage;
class BladesPage;
class PresetsPage;
class BladeArrayDlg;
class Settings;

class EditorWindow : public wxFrame {
public:
    EditorWindow(const std::string&, wxWindow*);
    ~EditorWindow();

    bool isSaved();
    std::string_view getOpenConfig() const;

    GeneralPage* generalPage{nullptr};
    PropsPage* propsPage{nullptr};
    BladesPage* bladesPage{nullptr};
    PresetsPage* presetsPage{nullptr};
    Settings* settings{nullptr};

    wxBoxSizer* sizer{nullptr};

    pcChoice* windowSelect{nullptr};

    enum {
        ID_WindowSelect,
        ID_DUMMY, // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl? This is a workaround.

        ID_SaveConfig,
        ID_ExportConfig,
        ID_VerifyConfig,

        ID_StyleEditor,
    };
private:
    void bindEvents();
    void createToolTips();
    void createMenuBar();
    void createPages();

    const std::string openConfig{};
};
