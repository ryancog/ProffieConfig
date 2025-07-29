#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023-2025 Ryan Ogurek

#include <wx/frame.h>
#include <wx/sizer.h>

#include "config/config.h"
#include "ui/frame.h"

// Forward declarations to get around circular dependencies
class GeneralPage;
class PropsPage;
class BladesPage;
class PresetsPage;

class EditorWindow : public PCUI::Frame {
public:
    EditorWindow(wxWindow *, Config::Config&);
    bool Destroy() final;

    // Handles errors
    bool save();

    [[nodiscard]] Config::Config& getOpenConfig() const;

    GeneralPage *generalPage{nullptr};
    PropsPage *propsPage{nullptr};
    BladesPage *bladesPage{nullptr};
    PresetsPage *presetsPage{nullptr};

    enum {
        ID_General = 2,
        ID_Props,
        ID_Presets,
        ID_BladeArrays,

        ID_ExportConfig,
        ID_VerifyConfig,
        ID_AddInjection,

        ID_StyleEditor,
    };
private:
    void createMenuBar();
    void createPages(wxSizer *);
    void bindEvents();

    void Fit() override;

    Config::Config& mConfig;
};
