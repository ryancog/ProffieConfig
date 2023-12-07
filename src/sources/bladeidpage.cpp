#include "bladeidpage.h"

#include "defines.h"
#include "mainwindow.h"
#include "misc.h"

#include <wx/tooltip.h>
#include <wx/button.h>

BladeIDPage* BladeIDPage::instance;
BladeIDPage::BladeIDPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "") {
  wxBoxSizer* enableSizer = new wxBoxSizer(wxHORIZONTAL);
  enableDetect = new wxCheckBox(GetStaticBox(), ID_BladeDetectEnable, "Enable Blade Detect");
  enableID = new wxCheckBox(GetStaticBox(), ID_BladeIDEnable, "Enable Blade ID");
  enableSizer->Add(enableDetect, FIRSTITEMFLAGS);
  enableSizer->Add(enableID, FIRSTITEMFLAGS);

  wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(createBladeDetect(GetStaticBox()), wxSizerFlags(2).Border(wxALL, 10).Expand());
  topSizer->Add(createIDSetup(GetStaticBox()), wxSizerFlags(3).Border(wxALL, 10).Expand());
  topSizer->Add(createBladeArrays(GetStaticBox()), wxSizerFlags(4).Border(wxALL, 10).Expand());

  wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
  bottomSizer->Add(createIDPowerSettings(GetStaticBox()), wxSizerFlags(1).Border(wxALL, 10).Expand());
  bottomSizer->Add(createContinuousScanSettings(GetStaticBox()), wxSizerFlags(1).Border(wxALL, 10).Expand());

  Add(enableSizer);
  Add(topSizer, wxSizerFlags(1).Expand());
  Add(bottomSizer, wxSizerFlags(0).Expand());

  bindEvents();
  createToolTips();
  update();
}

void BladeIDPage::bindEvents() {
  auto clearBladeArray = []() {
    BladeIDPage::instance->arrayList->SetSelection(-1);
    BladeIDPage::instance->lastArraySelection = -1;
    BladesPage::instance->bladeArray->SetSelection(0);
    BladesPage::instance->lastBladeArraySelection = -1;
    PresetsPage::instance->bladeArray->SetSelection(0);
  };

  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        update(); // Store last one before we do *damage*
        if (enableDetect->GetValue()) {
          bladeArrays.insert(bladeArrays.begin() + 1, BladeArray{"no_blade", 0, {}, { BladesPage::BladeConfig{} }});
          clearBladeArray();
        } else {
          if (wxMessageBox("Are you sure you want to disable Blade Detect?\n\n\"no_blade\" array will be deleted!", "Disable Blade Detect", wxYES_NO | wxNO_DEFAULT | wxCENTRE | wxICON_WARNING, MainWindow::instance) == wxNO) {
            enableDetect->SetValue(true);
            update();
            return;
          }
          bladeArrays.erase(bladeArrays.begin() + 1);
          clearBladeArray();
        }
        update();
      }, ID_BladeDetectEnable);
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        if (enableID->GetValue()) {

        } else {
          if (wxMessageBox("Are you sure you want to disable Blade ID?\n\nAll custom blade arrays will be deleted!", "Disable Blade ID", wxYES_NO | wxNO_DEFAULT | wxCENTRE | wxICON_WARNING, MainWindow::instance) == wxNO) {
            enableID->SetValue(true);
            update();
            return;
          }
          if (enableDetect->GetValue()) bladeArrays.erase(bladeArrays.begin() + 2, bladeArrays.end());
          else bladeArrays.erase(bladeArrays.begin() + 1, bladeArrays.end());

          clearBladeArray();
        }
        update();
      }, ID_BladeIDEnable);
  GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
        stripAndSaveName();
        update();
      }, ID_NameEntry);
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        update();
        FULLUPDATEWINDOW;
      }, ID_BladeIDMode);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        update();
      }, ID_BladeArray);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        bladeArrays.push_back(BladeArray{"newarray", 40000, {}, { BladesPage::BladeConfig{} }});
        update();
        arrayList->SetSelection(bladeArrays.size() - 1);
        arrayList->SendSelectionChangedEvent(wxEVT_LISTBOX);
      }, ID_AddArray);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {

        bladeArrays.erase(bladeArrays.begin() + arrayList->GetSelection());
        update();
      }, ID_RemoveArray);
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        update();
      }, ID_BladeIDPower);
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        update();
      }, ID_ContinuousScan);
}
void BladeIDPage::createToolTips() {
  TIP(enableDetect, "Detect when a blade is inserted into the saber or not.");
  TIP(enableID, "Detect when a specific blade is inserted based on a resistor placed in the blade to give it an identifier.");

  TIP(detectPin, "The pin which will be bridged to BATT- when blade is inserted.\nCannot be the same as ID Pin.");
  TIP(IDPin, "The pin used to detect blade resistance values.\nCannot be the same as Detect Pin.");
  TIP(mode, "The mode to be used for Blade ID.\nSee the POD page \"Blade ID\" for more info.");
  TIP(pullupResistance, "The value of the pullup resistor placed on the Blade ID line.");
  TIP(pullupPin, "The pin number or name of the pin which ID Pin is bridged to for pullup.\n This pin cannot be used for anything else.");

  TIP(enablePowerForID, "Enable power during Blade ID.\nThis is required for WS281X blades.");
  TIP(continuousScans, "Continuously monitor the Blade ID to detect changes.");
  TIP(scanIDMillis, "Scan the Blade ID and update accordingly every input number of millis.");
  TIP(numIDTimes, "Number of times to read the Blade ID to average out the result and compensate for inaccurate readings.");

  TIP(addID, "Add a blade array which will be enabled when a blade with the specified ID is inserted.");
  TIP(removeID, "Remove the selected blade array.");

  TIP(arrayName, "The name of the blade array.\nEach name must be unique, but it is for reference only (and thus specific names will not make a difference).");
  TIP(resistanceID, "The ID of the blade associated with the currently-selected blade array.\nThis value can be measured by typing \"id\" into the Serial Monitor.");
}

wxStaticBoxSizer* BladeIDPage::createBladeArrays(wxWindow* parent) {
  wxStaticBoxSizer* bladeIDSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Blade Arrays");

  bladeIDSizer->Add(createBladeArraysLeft(bladeIDSizer->GetStaticBox()), wxSizerFlags(1).Expand());
  bladeIDSizer->Add(createBladeArraysRight(bladeIDSizer), wxSizerFlags(0).Expand());

  return bladeIDSizer;
}
wxBoxSizer* BladeIDPage::createBladeArraysLeft(wxWindow* parent) {
  wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

  arrayList = new wxListBox(parent, ID_BladeArray, wxDefaultPosition, wxSize(100, 0), {}, 0);

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  addID = new wxButton(parent, ID_AddArray, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeID = new wxButton(parent, ID_RemoveArray, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  buttonSizer->Add(addID, MENUITEMFLAGS);
  buttonSizer->Add(removeID, MENUITEMFLAGS);

  leftSizer->Add(arrayList, wxSizerFlags(1).Border(wxLEFT | wxTOP | wxRIGHT, 10).Expand());
  leftSizer->Add(buttonSizer, wxSizerFlags(0).Border(wxALL, 5).Center());

  return leftSizer;
}
wxBoxSizer* BladeIDPage::createBladeArraysRight(wxStaticBoxSizer* parent) {
  wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

  arrayName = Misc::createTextEntry(parent->GetStaticBox(), "Blade Array Name", ID_NameEntry, "", 0);
  resistanceID = Misc::createNumEntry(parent->GetStaticBox(), "ID Value", wxID_ANY, 2000, 100000, 0);
  resistanceID->num->SetIncrement(100);

  rightSizer->Add(arrayName->box, MENUITEMFLAGS.Expand());
  rightSizer->Add(resistanceID->box, MENUITEMFLAGS.Expand());

  return rightSizer;
}

wxStaticBoxSizer* BladeIDPage::createIDSetup(wxWindow* parent) {
  wxStaticBoxSizer* setupSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade ID Setup");
  mode = new wxComboBox(setupSizer->GetStaticBox(), ID_BladeIDMode, BLADE_ID_MODE_SNAPSHOT, wxDefaultPosition, wxDefaultSize, Misc::createEntries({ BLADE_ID_MODE_SNAPSHOT, BLADE_ID_MODE_EXTERNAL, BLADE_ID_MODE_BRIDGED }), wxCB_READONLY);
  IDPin = Misc::createTextEntry(setupSizer->GetStaticBox(), "Blade ID Pin", wxID_ANY, "", 0);

  pullupResistance = Misc::createNumEntry(setupSizer->GetStaticBox(), "Pullup Resistance", wxID_ANY, 20000, 50000, 30000);
  pullupResistance->num->SetIncrement(100);
  pullupPin = Misc::createTextEntry(setupSizer->GetStaticBox(), "Pullup Pin", wxID_ANY, "", 0);

  setupSizer->Add(mode, BOXITEMFLAGS);
  setupSizer->Add(IDPin->box, BOXITEMFLAGS);
  setupSizer->Add(pullupResistance->box, BOXITEMFLAGS);
  setupSizer->Add(pullupPin->box, BOXITEMFLAGS);

  return setupSizer;
}
wxStaticBoxSizer* BladeIDPage::createIDPowerSettings(wxWindow* parent) {
  wxStaticBoxSizer* powerForIDSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Power for Blade ID");
  enablePowerForID = new wxCheckBox(powerForIDSizer->GetStaticBox(), ID_BladeIDPower, "Enable Power on ID", wxDefaultPosition, wxDefaultSize, 0);

  wxBoxSizer* powerPinSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* powerPinLeftSizer = new wxBoxSizer(wxVERTICAL);
  powerPin1 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 1", wxDefaultPosition, wxDefaultSize, 0);
  powerPin2 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 2", wxDefaultPosition, wxDefaultSize, 0);
  powerPin3 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 3", wxDefaultPosition, wxDefaultSize, 0);
  powerPinLeftSizer->Add(powerPin1, MENUITEMFLAGS);
  powerPinLeftSizer->Add(powerPin2, MENUITEMFLAGS);
  powerPinLeftSizer->Add(powerPin3, MENUITEMFLAGS);
  wxBoxSizer* powerPinRightSizer = new wxBoxSizer(wxVERTICAL);
  powerPin4 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 4", wxDefaultPosition, wxDefaultSize, 0);
  powerPin5 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 5", wxDefaultPosition, wxDefaultSize, 0);
  powerPin6 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 6", wxDefaultPosition, wxDefaultSize, 0);
  powerPinRightSizer->Add(powerPin4, MENUITEMFLAGS);
  powerPinRightSizer->Add(powerPin5, MENUITEMFLAGS);
  powerPinRightSizer->Add(powerPin6, MENUITEMFLAGS);

  powerPinSizer->Add(powerPinLeftSizer);
  powerPinSizer->Add(powerPinRightSizer);

  powerForIDSizer->Add(enablePowerForID, MENUITEMFLAGS);
  powerForIDSizer->Add(powerPinSizer);

  return powerForIDSizer;
}
wxStaticBoxSizer* BladeIDPage::createContinuousScanSettings(wxWindow* parent) {
  wxStaticBoxSizer* continuousScansSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Continuous Scanning");
  continuousScans = new wxCheckBox(continuousScansSizer->GetStaticBox(), ID_ContinuousScan, "Enable Continuous Scanning", wxDefaultPosition, wxDefaultSize, 0);
  numIDTimes = Misc::createNumEntry(continuousScansSizer->GetStaticBox(), "Number of Reads to Average", wxID_ANY, 1, 50, 10);
  scanIDMillis = Misc::createNumEntry(continuousScansSizer->GetStaticBox(), "Scan Interval (ms)", wxID_ANY, 10, 50000, 1000);
  continuousScansSizer->Add(continuousScans, MENUITEMFLAGS);
  continuousScansSizer->Add(numIDTimes->box, MENUITEMFLAGS);
  continuousScansSizer->Add(scanIDMillis->box, MENUITEMFLAGS);

  return continuousScansSizer;
}


wxStaticBoxSizer* BladeIDPage::createBladeDetect(wxWindow* parent) {
  wxStaticBoxSizer* bladeDetectSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade Detect");

  detectPin = Misc::createTextEntry(bladeDetectSizer->GetStaticBox(), "Blade Detect Pin", wxID_ANY, "", 0);
  bladeDetectSizer->Add(detectPin->box, wxSizerFlags(0).Border(wxALL, 5).Expand());

  return bladeDetectSizer;
}

void BladeIDPage::update() {
  if (lastArraySelection >= 0 && lastArraySelection < static_cast<int32_t>(bladeArrays.size())) {
    bladeArrays.at(lastArraySelection).name = arrayName->entry->GetValue();
    bladeArrays.at(lastArraySelection).value = resistanceID->num->GetValue();
  }

  lastArraySelection = arrayList->GetSelection();
  arrayList->Clear();

  for (int32_t array = 0; array < static_cast<int32_t>(bladeArrays.size()); array++) {
    if (arrayList->FindString(bladeArrays.at(array).name) == wxNOT_FOUND) arrayList->Append(bladeArrays.at(array).name);
    else bladeArrays.erase(bladeArrays.begin() + array);
  }
  if (lastArraySelection < 0 || lastArraySelection >= static_cast<int32_t>(arrayList->GetCount())) lastArraySelection = 0;
  arrayList->SetSelection(lastArraySelection);

  detectPin->Enable(enableDetect->GetValue());

  arrayList->Enable(enableID->GetValue());
  mode->Enable(enableID->GetValue());
  IDPin->Enable(enableID->GetValue());
  pullupPin->Enable(enableID->GetValue());
  pullupResistance->Enable(enableID->GetValue());
  enablePowerForID->Enable(enableID->GetValue());
  powerPin1->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin2->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin3->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin4->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin5->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin6->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  continuousScans->Enable(enableID->GetValue());
  numIDTimes->Enable(enableID->GetValue() && continuousScans->GetValue());
  scanIDMillis->Enable(enableID->GetValue() && continuousScans->GetValue());
  addID->Enable(enableID->GetValue());
  removeID->Enable(enableID->GetValue() && lastArraySelection && !(arrayList->GetStringSelection() == "blade_in" || arrayList->GetStringSelection() == "no_blade"));

  bool enable = bladeArrays[lastArraySelection].name != "no_blade" && bladeArrays[lastArraySelection].name != "blade_in";
  arrayName->Enable(enable);
  resistanceID->Enable(enable);
  if (enable) resistanceID->num->SetRange(2000, 100000);
  else resistanceID->num->SetRange(0, 0);
  arrayName->entry->ChangeValue(bladeArrays.at(lastArraySelection).name);
  resistanceID->num->SetValue(bladeArrays.at(lastArraySelection).value);

  if (mode->GetValue() == BLADE_ID_MODE_SNAPSHOT) {
    pullupResistance->box->Show(false);
    pullupPin->box->Show(false);
  } else if (mode->GetValue() == BLADE_ID_MODE_BRIDGED) {
    pullupResistance->box->Show(false);
    pullupPin->box->Show(true);
  } else if (mode->GetValue() == BLADE_ID_MODE_EXTERNAL) {
    pullupPin->box->Show(false);
    pullupResistance->box->Show(true);
  }

  UPDATEWINDOW;
}

void BladeIDPage::stripAndSaveName() {
  if (lastArraySelection > 0 && lastArraySelection < static_cast<int32_t>(bladeArrays.size())) {
    wxString name = arrayName->entry->GetValue();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c){ return std::tolower(c); }); // to lowercase
    int32_t insertPoint = arrayName->entry->GetInsertionPoint();
    arrayName->entry->ChangeValue(name);
    arrayName->entry->SetInsertionPoint(insertPoint);

    if (name == "blade_in") { bladeArrays.erase(bladeArrays.begin() + lastArraySelection); lastArraySelection = 0; }
    if (name == "no_blade") {
      bladeArrays.erase(bladeArrays.begin() + lastArraySelection);
    }
  }
}
