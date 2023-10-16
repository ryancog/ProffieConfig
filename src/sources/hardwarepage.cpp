#include "hardwarepage.h"
#include "defines.h"
#include "misc.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>

int ugly;

HardwarePage::HardwarePage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  Add(bladeDetect(this), MENUITEMFLAGS);
  Add(OLED(this), MENUITEMFLAGS);
}

wxStaticBoxSizer* HardwarePage::bladeDetect(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *bladeDetect = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Blade Detect");

  settings.bladeDetect = new wxCheckBox(bladeDetect->GetStaticBox(), wxID_ANY, "Blade Detect");
  settings.bladeDetectPinLabel = new wxStaticText(bladeDetect->GetStaticBox(), wxID_ANY, "Pin");
  settings.bladeDetectPin = new wxComboBox(bladeDetect->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"}));
  bladeDetect->Add(settings.bladeDetect, FIRSTITEMFLAGS);
  bladeDetect->Add(settings.bladeDetectPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeDetect->Add(settings.bladeDetectPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  return bladeDetect;
}
wxStaticBoxSizer* HardwarePage::OLED(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* OLED = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "OLED");

  settings.OLED = new wxCheckBox(OLED->GetStaticBox(), wxID_ANY, "Has OLED");
  OLED->Add(settings.OLED, FIRSTITEMFLAGS);

  return OLED;
}

void HardwarePage::update() {
  settings.bladeDetect->Show();
  settings.bladeDetectPin->Show();
  settings.bladeDetectPinLabel->Show();
}

decltype(HardwarePage::settings) HardwarePage::settings;
