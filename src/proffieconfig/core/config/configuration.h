#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../../editor/editorwindow.h"
#include "utils/types.h"

namespace Configuration {
    bool outputConfig(EditorWindow *editorWindow);

    /**
     * @param fullOutput output injection files
     */
    bool outputConfig(const filepath&, EditorWindow *editorWindow, bool fullOutput = false);
    bool exportConfig(EditorWindow *editorWindow);
    bool readConfig(const filepath&, EditorWindow *editorWindow);
    bool importConfig(EditorWindow *editorWindow);

    using MapPair = std::pair<const string, const string>;
    using VMap = vector<MapPair>;
    const MapPair& findInVMap(const VMap&, const string& search);

    inline const VMap ORIENTATION = {
        { "FETs Towards Blade", "ORIENTATION_FETS_TOWARDS_BLADE" },
        { "USB Towards Blade", "ORIENTATION_USB_TOWARDS_BLADE" },
        { "USB CCW From Blade", "ORIENTATION_USB_CCW_FROM_BLADE" },
        { "USB CW From Blade", "ORIENTATION_USB_CW_FROM_BLADE" },
        { "Top Towards Blade", "ORIENTATION_TOP_TOWARDS_BLADE" },
        { "Bottom Towards Blade", "ORIENTATION_BOTTOM_TOWARDS_BLADE" },
    };
    inline const VMap PROFFIEBOARD = {
        { "Proffieboard V3", "#include \"proffieboard_v3_config.h\"" },
        { "Proffieboard V2", "#include \"proffieboard_v2_config.h\"" },
        { "Proffieboard V1", "#include \"proffieboard_v1_config.h\"" },
    };
} // namespace Configuration
