#include "misc.h"

Misc::numEntry Misc::createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrl *numEntry = new wxSpinCtrl(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal);
    numEntryBox->Add(text, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 10));
    numEntryBox->Add(numEntry);

    Misc::numEntry returnVal;
    returnVal.box = numEntryBox;
    returnVal.num = numEntry;

    return returnVal;
}
Misc::numEntry Misc::createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrlDouble *numEntry = new wxSpinCtrlDouble(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal, 0.1);
    numEntryBox->Add(text, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 10));
    numEntryBox->Add(numEntry);

    Misc::numEntry returnVal;
    returnVal.box = numEntryBox;
    returnVal.doubleNum = numEntry;

    return returnVal;
}
