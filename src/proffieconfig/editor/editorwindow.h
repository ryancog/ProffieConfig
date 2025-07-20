#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include <wx/frame.h>
#include <wx/sizer.h>

#include "config/config.h"
#include "ui/controls/choice.h"
#include "ui/frame.h"

// Forward declarations to get around circular dependencies
class GeneralPage;
class PropsPage;
class BladesPage;
class PresetsPage;

class EditorWindow : public PCUI::Frame {
public:
    EditorWindow(wxWindow *, std::shared_ptr<Config::Config>);

    bool isSaved();

    [[nodiscard]] std::shared_ptr<Config::Config> getOpenConfig() const;

    GeneralPage *generalPage{nullptr};
    PropsPage *propsPage{nullptr};
    BladesPage *bladesPage{nullptr};
    PresetsPage *presetsPage{nullptr};

    enum {
        ID_WindowSelect,
        ID_DUMMY, // on Win32, for some reason ID #1 is triggerred by hitting enter in pcTextCtrl? This is a workaround.

        ID_ExportConfig,
        ID_VerifyConfig,
        ID_AddInjection,

        ID_StyleEditor,
    };
private:
    void createMenuBar();
    void createPages(wxSizer *);
    void bindEvents();

    std::shared_ptr<Config::Config> mConfig;
};
