#include "misc.h"
#include "defines.h"
#include "wx/arrstr.h"
#include <initializer_list>
#include <memory>
#include <string>

Misc::numEntry Misc::createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrl *numEntry = new wxSpinCtrl(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal);
    numEntryBox->Add(text, MENUITEMFLAGS.Center());
    numEntryBox->Add(numEntry);

    Misc::numEntry returnVal;
    returnVal.box = numEntryBox;
    returnVal.num = numEntry;

    return returnVal;
}
Misc::numEntryDouble Misc::createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrlDouble *numEntry = new wxSpinCtrlDouble(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal, 0.1);
    numEntryBox->Add(text, MENUITEMFLAGS.Center());
    numEntryBox->Add(numEntry);

    Misc::numEntryDouble returnVal;
    returnVal.box = numEntryBox;
    returnVal.num = numEntry;

    return returnVal;
}

const wxArrayString Misc::createEntries(std::vector<std::string> list) {
    wxArrayString entries;
    for (const std::string& entry : list) {
      entries.Add(entry);
    }

    return entries;
}
const wxArrayString Misc::createEntries(std::initializer_list<std::string> list) {
    return Misc::createEntries(std::vector<std::string>(list));
}
