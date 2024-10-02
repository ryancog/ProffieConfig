// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"

#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

// Forward declaration to get around circular dependency
class PropFile;

class PropsPage : public wxStaticBoxSizer {
public:
  PropsPage(wxWindow*);

  void update();
  void updateSizeAndLayout();
  void updateDisables(PropFile*);
  void updateProps();
  void updateSelectedProp(const wxString& = "");
  PropFile* getSelectedProp();
  const std::vector<PropFile*>& getLoadedProps();
  wxScrolledWindow* propsWindow{nullptr};

  pcChoice* propSelection{nullptr};
  wxButton* buttonInfo{nullptr};
  wxButton* propInfo{nullptr};

  enum {
    ID_PropSelect,
    ID_Option,
    ID_Buttons,
    ID_PropInfo,
  };
private:
  EditorWindow* parent{nullptr};

  std::vector<PropFile*> props;

  void loadProps();
  void bindEvents();
};
