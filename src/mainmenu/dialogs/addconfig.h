#pragma once

#include <wx/dialog.h>

class AddConfig : public wxDialog {
public:
  AddConfig();



private:
  void update();
  void createUI();
  void bindEvents();

};
