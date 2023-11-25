#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>

#pragma once

class HardwarePage : public wxStaticBoxSizer
{
public:
  HardwarePage(wxWindow*);
  static HardwarePage* instance;

  void update();

  wxCheckBox* bladeDetect{nullptr};
  wxStaticText* bladeDetectPinLabel{nullptr};
  wxComboBox* bladeDetectPin{nullptr};

  wxCheckBox* OLED{nullptr};

private:
  wxStaticBoxSizer* createBladeDetect(wxStaticBoxSizer*);
  wxStaticBoxSizer* createOLED(wxStaticBoxSizer*);

  HardwarePage();
};
