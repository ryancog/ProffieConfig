#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <wx/frame.h>
#include <wx/sizer.h>

#include "ui/controls.h"
#include "ui/frame.h"

// Forward declarations to get around circular dependencies
class GeneralPage;
class PropsPage;
class BladesPage;
class PresetsPage;
class BladeArrayDlg;
class Settings;

class EditorWindow : public PCUI::Frame {
public:
    EditorWindow(const string&, wxWindow*);
    ~EditorWindow() override;

    bool isSaved();

    [[nodiscard]] string getOpenConfig() const;
    void renameConfig(const string&);


    GeneralPage* generalPage{nullptr};
    PropsPage* propsPage{nullptr};
    BladesPage* bladesPage{nullptr};
    PresetsPage* presetsPage{nullptr};
    Settings* settings{nullptr};

    wxBoxSizer* sizer{nullptr};

    PCUI::Choice* windowSelect{nullptr};

    enum {
        ID_WindowSelect,
        ID_DUMMY, // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl? This is a workaround.

        ID_ExportConfig,
        ID_VerifyConfig,
        ID_AddInjection,

        ID_StyleEditor,
    };
private:
    void bindEvents();
    void createToolTips();
    void createMenuBar();
    void createPages();

    string mOpenConfig;
};
