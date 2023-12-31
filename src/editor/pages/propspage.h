// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "core/utilities/misc.h"

#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

// Forward declaration to get around circular dependency
class PropFile;

class PropsPage : public wxScrolledWindow {
public:
  PropsPage(wxWindow*);

  void update();
  void updateProps();
  void updateSelectedProp(const wxString& = "");
  PropFile* getSelectedProp();
  const std::vector<PropFile*>& getLoadedProps();
  wxStaticBoxSizer* sizer{nullptr};

  Misc::comboBoxEntry propSelection{};
  wxButton* buttonInfo{nullptr};

  enum {
    ID_PropSelect,
    ID_Option,
    ID_Buttons
  };
private:
  EditorWindow* parent{nullptr};

  std::vector<PropFile*> props;

  void loadProps();
  void bindEvents();
};
