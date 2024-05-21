#include "styledocs.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * styles/documentation/styledocs.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>

#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "appcore/interfaces.h"
#include "log/logger.h"
#include "ui/frame.h"
#include "pconf/pconf.h"
#include "styles/bladestyle.h"

using namespace BladeStyles;

struct DocInfo {
    std::string humanName;
    std::string osName{};
    uint32_t type{0};
    std::string description{};
    
    struct ArgInfo {
        std::string name;
        uint32_t type{0};
        std::string description{};
    };
    
    std::vector<ArgInfo> args{};
};

static PCUI::Frame* frame{nullptr};
static wxPanel* docPanel{nullptr};
static std::unordered_map<void*, std::string>* treeMap{nullptr};
static std::unordered_map<std::string, DocInfo>* docMap{nullptr};

static void createUI();
static void generateTree(wxTreeCtrl*);
static void generateDocs();

void Documentation::open(const std::string& styleName) {
    if (!frame) {
        frame = new PCUI::Frame(nullptr, AppCore::STYLEDOCS, "ProffieConfig StyleDocs");
        frame->setReference(&frame);

        generateDocs();
        createUI();

        frame->Show();
    }

    frame->Raise();
    if (styleName.empty()) goHome(); 
    else showPage(styleName);
}

void generateDocs() {
    if (docMap) return;

    docMap = new std::unordered_map<std::string, DocInfo>;
    
    constexpr const char* docFiles[]{
        "functions.pconf",
        "transitions.pconf",
    };

    static const auto parseType{[](const std::string& typeStr) -> uint32_t {
        uint32_t type{0};
        // if (typeStr.find("Number")      != std::string::npos) type |= INT;
        if (typeStr.find("Transition")  != std::string::npos) type |= TRANSITION;
        if (typeStr.find("Color")       != std::string::npos) type |= COLOR;
        if (typeStr.find("Effect")      != std::string::npos) type |= EFFECT;
        return type;
    }};

    std::ifstream docFile; 
    for (const auto& file : docFiles) {
        docFile.open(wxGetCwd().ToStdString() + RESOURCEPATH "styledocs/" + file);
        if (!docFile.is_open()) {
            Logger::warn(std::string("Failed to open StyleDoc file: ") + file);
            continue;
        }
        Logger::info(std::string("Reading StyleDoc: ") + file);

        while (docFile.peek() != EOF) {
            auto sect{PConf::readSection(docFile)};
            if (!sect) {
                Logger::info("Stray entry in StyleDoc file.");
                continue;
            }
            if (sect->name != "ELEMENT") {
                Logger::info("Stray non-element section in StyleDoc file.");
                continue;
            }
            if (!sect->label) {
                Logger::warn("Element missing label in StyleDoc file");
                continue;
            }

            DocInfo info{
                .humanName = sect->label.value()
            };
            for (auto entry : sect->entries) {
                if (!entry->value) {
                    Logger::warn("Stray entry without value in element: " + info.humanName);
                    continue;
                }

                if (entry->name == "TYPE") info.type = parseType(entry->value.value());
                else if (entry->name == "OSNAME") info.osName = entry->value.value();
                else if (entry->name == "DESCRIPTION") info.description = entry->value.value();
                else if (entry->name == "ARGUMENTS") {
                    if (entry->getType() != PConf::DataType::SECTION) {
                        Logger::warn("Argument section read as entry in element: " + info.humanName);
                        continue;
                    }

                    for (const auto arg : static_cast<PConf::Section*>(entry)->entries) {
                        if (!arg->label) {
                            Logger::warn("Argument with missing name in element: " + info.humanName);
                            continue;
                        }
                        if (arg->getType() != PConf::DataType::SECTION) {
                            Logger::warn("Argument \"" + arg->label.value() + "\" section interpreted as entry in element: " + info.humanName);
                            continue;
                        }

                        DocInfo::ArgInfo argInfo{
                            .name = arg->label.value(),
                        };
                        for (const auto argEntry : static_cast<PConf::Section*>(arg)->entries) {
                            if (!arg->value) {
                                Logger::warn("Missing value for entry \"" + argEntry->name + "\" in argument \"" + arg->name + "\" for element: " + info.humanName);
                                continue;
                            }
                            if (arg->name == "TYPE") argInfo.type = parseType(arg->value.value());
                            else if (arg->name == "DESC") argInfo.description = arg->value.value();
                        }
                    }
                }
            }

            docMap->emplace(info.osName, info);
        }
    }
}

void generateTree(wxTreeCtrl* treeCtrl) {
    auto root{treeCtrl->AddRoot("ProffieConfig StyleDocs")};
    auto styleNode{treeCtrl->AppendItem(root, "Styles")};
    auto layerNode{treeCtrl->AppendItem(root, "Layers")};
    auto funcNode{treeCtrl->AppendItem(root, "Functions")};
    auto transNode{treeCtrl->AppendItem(root, "Transitions")};
    auto effectNode{treeCtrl->AppendItem(root, "Effects")};
    auto builtinNode{treeCtrl->AppendItem(root, "Built-In")};

    for (const auto& [ elemName, doc ] : *docMap) {
        wxTreeItemId category;
        switch (doc.type & BladeStyles::FLAGMASK) {
            case COLOR:
                category = styleNode;
                break;
            case LAYER:
                category = layerNode;
                break;
            // case INT:
                // category = funcNode;
                // break;
            case TRANSITION:
                category = transNode;
                break;
            case EFFECT:
                category = effectNode;
                break;
            case BUILTIN:
                category = builtinNode;
                break;
        }

        auto docNode{treeCtrl->AppendItem(category, doc.humanName)};
        treeMap->emplace(docNode.GetID(), elemName);
    }
}

void createUI() {
    auto splitter{new wxSplitterWindow(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE)};

    auto treeView{new wxTreeCtrl(splitter, wxID_ANY)};
    generateTree(treeView);

    docPanel = new wxPanel(splitter, wxID_ANY);
    docPanel->SetSizer(new wxBoxSizer(wxVERTICAL));

    splitter->SplitVertically(treeView, docPanel);
    splitter->SetMinimumPaneSize(100);
}

void Documentation::showPage(const std::string& styleName) {
    auto docSizer{docPanel->GetSizer()};
    docSizer->Clear(true);

    auto docInfo{docMap->find(styleName)};
    if (docInfo == docMap->end()) {
        docSizer->AddSpacer(20);
        docSizer->Add(new wxStaticText(docPanel, wxID_ANY, "Sorry, this documentation doesn't exist yet!"), wxSizerFlags(0).Center());
        return;
    }

    // Page generation logic here
}

void Documentation::goHome() {
    auto docSizer{docPanel->GetSizer()};
    docSizer->Clear(true);

    docSizer->AddSpacer(20);
    docSizer->Add(new wxStaticText(docPanel, wxID_ANY, "Welcome to the ProffieConfig StyleDocs!"), wxSizerFlags(0).Center());
}
