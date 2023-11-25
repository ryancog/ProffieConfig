#include "hardwarepage.h"

#include "defines.h"
#include "misc.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>

HardwarePage* HardwarePage::instance;
HardwarePage::HardwarePage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "")
{
  Add(createBladeDetect(this), MENUITEMFLAGS);
  Add(createOLED(this), MENUITEMFLAGS);
}

wxStaticBoxSizer* HardwarePage::createBladeDetect(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer *bladeDetectSizer = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "Blade Detect");

  bladeDetect = new wxCheckBox(bladeDetectSizer->GetStaticBox(), wxID_ANY, "Blade Detect");
  bladeDetectPinLabel = new wxStaticText(bladeDetectSizer->GetStaticBox(), wxID_ANY, "Pin");
  bladeDetectPin = new wxComboBox(bladeDetectSizer->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"bladePin", "blade2Pin", "blade3Pin", "blade4Pin"}));
  bladeDetectSizer->Add(bladeDetect, FIRSTITEMFLAGS);
  bladeDetectSizer->Add(bladeDetectPinLabel, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10));
  bladeDetectSizer->Add(bladeDetectPin, wxSizerFlags(0).Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

  return bladeDetectSizer;
}
wxStaticBoxSizer* HardwarePage::createOLED(wxStaticBoxSizer* parent) {
  wxStaticBoxSizer* OLEDSizer = new wxStaticBoxSizer(wxVERTICAL, parent->GetStaticBox(), "OLED");

  OLED = new wxCheckBox(OLEDSizer->GetStaticBox(), wxID_ANY, "Has OLED");
  OLEDSizer->Add(OLED, FIRSTITEMFLAGS);

  return OLEDSizer;
}

void HardwarePage::update() {
  bladeDetect->Show();
  bladeDetectPin->Show();
  bladeDetectPinLabel->Show();
}
