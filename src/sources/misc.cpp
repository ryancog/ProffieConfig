#include "misc.h"
#include "defines.h"

#include <wx/msgdlg.h>
#include <wx/arrstr.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <fstream>

void Misc::importFile(wxWindow*) {
  /*
  wxFileDialog fileDlg(win, "Select Config File", "../", "", "C++ Header File (*.h)|*.h", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (fileDlg.ShowModal() == wxID_CANCEL) return;

  std::ifstream file(fileDlg.GetPath().ToStdString());
*/
  std::ifstream file("resources/ProffieOS/config/ProffieConfig_autogen.h");
  if (!file.is_open()) {
    wxMessageBox("Could not open existing file", "File Error");
    return;
  }

  std::string line;
  while (!file.eof()) {
    file >> line;
    switch (fileItemType(line)) {
      case ItemType::TOP_DEFINE:
        Misc::readTopDefine(file);
        break;
      case ItemType::CONST_VAL:
        break;
      case ItemType::INCLUDE:
        break;
      case ItemType::PRESET_SECTION:
        break;
      case ItemType::BLADE_SECTION:
        break;
      default: break;
    }
  }
}

Misc::ItemType Misc::fileItemType(const std::string&) {

}

void Misc::readTopDefine(std::ifstream& file) {
  std::string entryType;
  file >> entryType;


}

void Misc::exportFile(wxWindow*) {

}

Misc::numEntry* Misc::createNumEntry(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, int32_t minVal, int32_t maxVal, int32_t defaultVal) {
    wxBoxSizer *numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrl *numEntry = new wxSpinCtrl(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal);
    numEntryBox->Add(text, MENUITEMFLAGS.Center());
    numEntryBox->Add(numEntry, MENUITEMFLAGS.Border(wxRIGHT, 5));

    Misc::numEntry* returnVal = new Misc::numEntry();
    returnVal->box = numEntryBox;
    returnVal->num = numEntry;

    return returnVal;
}
Misc::numEntryDouble* Misc::createNumEntryDouble(wxStaticBoxSizer *parent, wxString displayText, int32_t ID, double minVal, double maxVal, double defaultVal) {
    wxBoxSizer* numEntryBox = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* text = new wxStaticText(parent->GetStaticBox(), wxID_ANY, displayText);
    wxSpinCtrlDouble *numEntry = new wxSpinCtrlDouble(parent->GetStaticBox(), ID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, minVal, maxVal, defaultVal, 0.1);
    numEntryBox->Add(text, MENUITEMFLAGS.Center());
    numEntryBox->Add(numEntry, MENUITEMFLAGS.Border(wxRIGHT, 5));

    Misc::numEntryDouble* returnVal = new Misc::numEntryDouble;
    returnVal->box = numEntryBox;
    returnVal->num = numEntry;

    return returnVal;
}

const wxArrayString Misc::createEntries(std::vector<std::string> list) {
    wxArrayString entries;
    for (const std::string& entry : list)
      entries.Add(entry);

    return entries;
}
const wxArrayString Misc::createEntries(std::initializer_list<std::string> list) {
    return Misc::createEntries(std::vector<std::string>(list));
}
