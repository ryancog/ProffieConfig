// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/dialogs/bladearraydlg.h"

#include "core/defines.h"
#include "editor/editorwindow.h"
#include "core/utilities/misc.h"
#include "onboard/onboard.h"

#include <wx/tooltip.h>
#include <wx/button.h>

#ifdef __WINDOWS__
#undef wxMessageDialog
#include <wx/msgdlg.h>
#define wxMessageDialog wxGenericMessageDialog
#else
#include <wx/msgdlg.h>
#endif

BladeArrayDlg::BladeArrayDlg(EditorWindow* _parent) : wxDialog(_parent, wxID_ANY, "Blade Awareness - " + wxString{_parent->getOpenConfig()}, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), parent(_parent) {
  sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* enableSizer = new wxBoxSizer(wxHORIZONTAL);
  enableDetect = new wxCheckBox(this, ID_BladeDetectEnable, "Enable Blade Detect");
  enableID = new wxCheckBox(this, ID_BladeIDEnable, "Enable Blade ID");
  enableSizer->Add(enableDetect, FIRSTITEMFLAGS);
  enableSizer->Add(enableID, FIRSTITEMFLAGS);

  wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(createBladeDetect(this), wxSizerFlags(2).Border(wxALL, 10).Expand());
  topSizer->Add(createIDSetup(this), wxSizerFlags(3).Border(wxALL, 10).Expand());
  topSizer->Add(createBladeArrays(this), wxSizerFlags(4).Border(wxALL, 10).Expand());

  wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
  bottomSizer->Add(createIDPowerSettings(this), wxSizerFlags(1).Border(wxALL, 10).Expand());
  bottomSizer->Add(createContinuousScanSettings(this), wxSizerFlags(1).Border(wxALL, 10).Expand());

  sizer->Add(enableSizer);
  sizer->Add(topSizer, wxSizerFlags(1).Expand());
  sizer->Add(bottomSizer, wxSizerFlags(0).Expand());

  SetSizerAndFit(sizer);
  bindEvents();
  createToolTips();
  update();
  FULLUPDATEWINDOW(this);
}

void BladeArrayDlg::bindEvents() {
  Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& event) {
    if (event.CanVeto()) {
      parent->bladesPage->update();
      Hide();
      event.Veto();
    } else event.Skip();
  });
  
  auto clearBladeArray = [](BladeArrayDlg* page) {
    page->arrayList->SetSelection(-1);
    page->lastArraySelection = -1;
    page->parent->bladesPage->bladeArray->entry()->SetSelection(0);
    page->parent->bladesPage->lastBladeArraySelection = -1;
    page->parent->presetsPage->bladeArray->entry()->SetSelection(0);
  };

  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
      update(); // Store last one before we do *damage*
      if (enableDetect->GetValue()) {
        bladeArrays.insert(bladeArrays.begin() + 1, BladeArray{"no_blade", 0, {}, { BladesPage::BladeConfig{} }});
        clearBladeArray(this);
      } else {
          if (OnboardFrame::instance == nullptr && wxMessageDialog(parent, "Are you sure you want to disable Blade Detect?\n\n\"no_blade\" array will be deleted!", "Disable Blade Detect", wxYES_NO | wxNO_DEFAULT | wxCENTRE | wxICON_WARNING).ShowModal() == wxID_NO) {
          enableDetect->SetValue(true);
          update();
          return;
        }
        bladeArrays.erase(bladeArrays.begin() + 1);
        clearBladeArray(this);
      }
      update();

    }, ID_BladeDetectEnable);
  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
      if (enableID->GetValue()) {

      } else {
          if (OnboardFrame::instance == nullptr && wxMessageDialog(parent, "Are you sure you want to disable Blade ID?\n\nAll custom blade arrays will be deleted!", "Disable Blade ID", wxYES_NO | wxNO_DEFAULT | wxCENTRE | wxICON_WARNING).ShowModal() == wxID_NO) {
          enableID->SetValue(true);
          update();
          return;
        }
        if (enableDetect->GetValue()) bladeArrays.erase(bladeArrays.begin() + 2, bladeArrays.end());
        else bladeArrays.erase(bladeArrays.begin() + 1, bladeArrays.end());

        clearBladeArray(this);
      }
      update();
    }, ID_BladeIDEnable);
  Bind(wxEVT_TEXT, [&](wxCommandEvent&) {
      stripAndSaveName();
      update();
    }, ID_NameEntry);
  Bind(wxEVT_CHOICE, [&](wxCommandEvent&) {
      update();
      FULLUPDATEWINDOW(this);
    }, ID_BladeIDMode);
  Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
      update();
    }, ID_BladeArray);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
      bladeArrays.push_back(BladeArray{"newarray", 40000, {}, { BladesPage::BladeConfig{} }});
      update();
      arrayList->SetSelection(bladeArrays.size() - 1);
      arrayList->SendSelectionChangedEvent(wxEVT_LISTBOX);
      FULLUPDATEWINDOW(this);
    }, ID_AddArray);
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
      bladeArrays.erase(bladeArrays.begin() + arrayList->GetSelection());
      update();
    }, ID_RemoveArray);
  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
      update();
    }, ID_BladeIDPower);
  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
      update();
    }, ID_ContinuousScan);
}
void BladeArrayDlg::createToolTips() {
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

wxStaticBoxSizer* BladeArrayDlg::createBladeArrays(wxWindow* parent) {
  wxStaticBoxSizer* bladeIDSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Blade Arrays");

  bladeIDSizer->Add(createBladeArraysLeft(bladeIDSizer->GetStaticBox()), wxSizerFlags(1).Expand());
  bladeIDSizer->Add(createBladeArraysRight(bladeIDSizer), wxSizerFlags(0).Expand());

  return bladeIDSizer;
}
wxBoxSizer* BladeArrayDlg::createBladeArraysLeft(wxWindow* parent) {
  wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

  arrayList = new wxListBox(parent, ID_BladeArray, wxDefaultPosition, wxSize(100, -1), wxArrayString{}, wxNO_BORDER);

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  addID = new wxButton(parent, ID_AddArray, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeID = new wxButton(parent, ID_RemoveArray, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  buttonSizer->Add(addID, MENUITEMFLAGS);
  buttonSizer->Add(removeID, MENUITEMFLAGS);

  leftSizer->Add(arrayList, wxSizerFlags(1).Border(wxLEFT | wxTOP | wxRIGHT, 10).Expand());
  leftSizer->Add(buttonSizer, wxSizerFlags(0).Border(wxALL, 5).Center());

  return leftSizer;
}
wxBoxSizer* BladeArrayDlg::createBladeArraysRight(wxStaticBoxSizer* parent) {
  wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

  arrayName = new pcTextCtrl(parent->GetStaticBox(), ID_NameEntry, "Blade Array Name");
  resistanceID = new pcSpinCtrl(parent->GetStaticBox(), wxID_ANY, "ID Value", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 2000, 100000, 0);
  resistanceID->entry()->SetIncrement(100);

  rightSizer->Add(arrayName, MENUITEMFLAGS.Expand());
  rightSizer->Add(resistanceID, MENUITEMFLAGS.Expand());

  return rightSizer;
}

wxStaticBoxSizer* BladeArrayDlg::createIDSetup(wxWindow* parent) {
  wxStaticBoxSizer* setupSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade ID Setup");
  mode = new pcChoice(setupSizer->GetStaticBox(), ID_BladeIDMode, "Blade ID Mode", wxDefaultPosition, wxDefaultSize, Misc::createEntries({ BLADE_ID_MODE_SNAPSHOT, BLADE_ID_MODE_EXTERNAL, BLADE_ID_MODE_BRIDGED }), 0);
  IDPin = new pcTextCtrl(setupSizer->GetStaticBox(), wxID_ANY, "Blade ID Pin");

  pullupResistance = new pcSpinCtrl(setupSizer->GetStaticBox(), wxID_ANY, "Pullup Resistance", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 20000, 50000, 30000);
  pullupResistance->entry()->SetIncrement(100);
  pullupPin = new pcTextCtrl(setupSizer->GetStaticBox(), wxID_ANY, "Pullup Pin");

  setupSizer->Add(mode, BOXITEMFLAGS);
  setupSizer->Add(IDPin, BOXITEMFLAGS);
  setupSizer->Add(pullupResistance, BOXITEMFLAGS);
  setupSizer->Add(pullupPin, BOXITEMFLAGS);

  return setupSizer;
}
wxStaticBoxSizer* BladeArrayDlg::createIDPowerSettings(wxWindow* parent) {
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
wxStaticBoxSizer* BladeArrayDlg::createContinuousScanSettings(wxWindow* parent) {
  wxStaticBoxSizer* continuousScansSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Continuous Scanning");
  continuousScans = new wxCheckBox(continuousScansSizer->GetStaticBox(), ID_ContinuousScan, "Enable Continuous Scanning", wxDefaultPosition, wxDefaultSize, 0);
  numIDTimes = new pcSpinCtrl(continuousScansSizer->GetStaticBox(), wxID_ANY, "Number of Reads to Average", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 50, 10);
  scanIDMillis = new pcSpinCtrl(continuousScansSizer->GetStaticBox(), wxID_ANY, "Scan Interval (ms)", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS,  10, 50000, 1000);
  continuousScansSizer->Add(continuousScans, MENUITEMFLAGS);
  continuousScansSizer->Add(numIDTimes, MENUITEMFLAGS);
  continuousScansSizer->Add(scanIDMillis, MENUITEMFLAGS);

  return continuousScansSizer;
}


wxStaticBoxSizer* BladeArrayDlg::createBladeDetect(wxWindow* parent) {
  wxStaticBoxSizer* bladeDetectSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade Detect");

  detectPin = new pcTextCtrl(bladeDetectSizer->GetStaticBox(), wxID_ANY, "Blade Detect Pin");
  bladeDetectSizer->Add(detectPin, wxSizerFlags(0).Border(wxALL, 5).Expand());

  return bladeDetectSizer;
}

void BladeArrayDlg::update() {
  if (lastArraySelection >= 0 && lastArraySelection < static_cast<int32_t>(bladeArrays.size())) {
    bladeArrays.at(lastArraySelection).name = arrayName->entry()->GetValue();
    bladeArrays.at(lastArraySelection).value = resistanceID->entry()->GetValue();
  }

  lastArraySelection = arrayList->GetSelection();
  arrayList->Clear();

  for (int32_t array = 0; array < static_cast<int32_t>(bladeArrays.size()); array++) {
    if (arrayList->FindString(bladeArrays.at(array).name) == wxNOT_FOUND) arrayList->Append(bladeArrays.at(array).name);
    else bladeArrays.erase(bladeArrays.begin() + array);
  }
  if (lastArraySelection < 0 || lastArraySelection >= static_cast<int32_t>(arrayList->GetCount())) lastArraySelection = 0;
  arrayList->SetSelection(lastArraySelection);

  detectPin->entry()->Enable(enableDetect->GetValue());

  arrayList->Enable(enableID->GetValue());
  mode->entry()->Enable(enableID->GetValue());
  IDPin->entry()->Enable(enableID->GetValue());
  pullupPin->entry()->Enable(enableID->GetValue());
  pullupResistance->entry()->Enable(enableID->GetValue());
  enablePowerForID->Enable(enableID->GetValue());
  powerPin1->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin2->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin3->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin4->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin5->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  powerPin6->Enable(enableID->GetValue() && enablePowerForID->GetValue());
  continuousScans->Enable(enableID->GetValue());
  numIDTimes->entry()->Enable(enableID->GetValue() && continuousScans->GetValue());
  scanIDMillis->entry()->Enable(enableID->GetValue() && continuousScans->GetValue());
  addID->Enable(enableID->GetValue());
  removeID->Enable(enableID->GetValue() && lastArraySelection && !(arrayList->GetStringSelection() == "blade_in" || arrayList->GetStringSelection() == "no_blade"));

  bool customBladeArraySelected = bladeArrays[lastArraySelection].name != "no_blade" && bladeArrays[lastArraySelection].name != "blade_in";
  arrayName->entry()->Enable(customBladeArraySelected);
  resistanceID->entry()->Enable(customBladeArraySelected);
  if (customBladeArraySelected) resistanceID->entry()->SetRange(2000, 100000);
  else resistanceID->entry()->SetRange(0, 0);
  arrayName->entry()->ChangeValue(bladeArrays.at(lastArraySelection).name);
  resistanceID->entry()->SetValue(bladeArrays.at(lastArraySelection).value);

  if (mode->entry()->GetStringSelection() == BLADE_ID_MODE_SNAPSHOT) {
    pullupResistance->Show(false);
    pullupPin->Show(false);
  } else if (mode->entry()->GetStringSelection() == BLADE_ID_MODE_BRIDGED) {
    pullupResistance->Show(false);
    pullupPin->Show(true);
  } else if (mode->entry()->GetStringSelection() == BLADE_ID_MODE_EXTERNAL) {
    pullupPin->Show(false);
    pullupResistance->Show(true);
  }

  if (parent->bladesPage && parent->presetsPage) {
    parent->bladesPage->update();
    parent->presetsPage->update();
  }
#ifdef __WINDOWS__
  Refresh();
#endif
}

void BladeArrayDlg::stripAndSaveName() {
  if (lastArraySelection > 0 && lastArraySelection < static_cast<int32_t>(bladeArrays.size())) {
    wxString name = arrayName->entry()->GetValue();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c){ return std::tolower(c); }); // to lowercase
    int32_t insertPoint = arrayName->entry()->GetInsertionPoint();
    arrayName->entry()->ChangeValue(name);
    arrayName->entry()->SetInsertionPoint(insertPoint);

    if (name == "blade_in") { bladeArrays.erase(bladeArrays.begin() + lastArraySelection); lastArraySelection = 0; }
    if (name == "no_blade") {
      bladeArrays.erase(bladeArrays.begin() + lastArraySelection);
    }
  }
}
