#include "styleblock.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * styleeditor/blocks/styleblock.cpp
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

#include <iostream>
#include <optional>

#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/colour.h>
#include <wx/button.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>

#include "styleeditor/blocks/bitsctrl.h"
#include "styles/documentation/styledocs.h"
#include "styles/bladestyle.h"
#include "styles/elements/args.h"
#include "styles/elements/colors.h"
#include "styles/elements/effects.h"
#include "styles/elements/lockuptype.h"
#include "styles/parse.h"
#include "ui/bool.h"
#include "ui/numeric.h"
#include "wx/dataobj.h"

using namespace PCUI;

wxDEFINE_EVENT(PCUI::SB_COLLAPSED, wxCommandEvent);
std::unordered_map<const BladeStyles::BladeStyle*, StyleBlock*> StyleBlock::blockMap{};

StyleBlock::StyleBlock(MoveArea* moveParent, wxWindow* parent, BladeStyles::BladeStyle* style) :
    Block(parent, style->getType()),
    Movable(moveParent),
    style(style),
    name(style->humanName) {

    SetName("StyleBlock");
    routine = std::bind(&StyleBlock::tryAdopt, this, std::placeholders::_1, std::placeholders::_2);
    bindEvents();

    blockMap.emplace(style, this);

    bindChildren();
    initHelp();
    update();
}

StyleBlock::~StyleBlock() {
    if (moveArea) moveArea->removeAdoptRoutine(this);
    blockMap.erase(blockMap.find(style));
}

void StyleBlock::bindEvents() {
    Block::Bind(wxEVT_PAINT, [&](wxPaintEvent& evt) { paintEvent(evt); });
    Block::Bind(wxEVT_MIDDLE_DOWN, [&](wxMouseEvent&) { collapse(!collapsed); });
    Block::Bind(wxEVT_RIGHT_DOWN, [&](wxMouseEvent& evt) { showPopMenu(evt); });
    Block::Bind(wxEVT_MENU, [&](wxCommandEvent&) { collapse(!collapsed); }, COLLAPSE);
    Block::Bind(wxEVT_MENU, [&](wxCommandEvent&) { 
        if (!wxClipboard::Get()->Open()) return;
        wxClipboard::Get()->SetData(new wxTextDataObject(getString()));
        wxClipboard::Get()->Close();
    }, COPY);
    Block::Bind(wxEVT_MENU, [&](wxCommandEvent&) { 
        if (!wxClipboard::Get()->Open()) return;
        wxClipboard::Get()->SetData(new wxTextDataObject(getString()));
        wxClipboard::Get()->Close();
        Destroy();
    }, CUT);
    Block::Bind(wxEVT_MENU, [&](wxCommandEvent&) { BladeStyles::Documentation::open(name.ToStdString()); }, HELP);
}

void StyleBlock::bindChildren() {
    for (const auto& param : style->getParams()) {
        if (!(param->getType() & BladeStyles::STYLETYPE)) continue;
        auto paramStyle{static_cast<const BladeStyles::StyleParam*>(param)->getStyle()};
        if (!paramStyle) continue;
        if (blockMap.find(paramStyle) != blockMap.end()) continue;

        new StyleBlock(moveArea, GetParent(), const_cast<BladeStyles::BladeStyle*>(paramStyle));
    }
}

std::string StyleBlock::getString() const { 
    auto style{getStyle()};
    auto ret{BladeStyles::asString(*style)};
    delete style;
    return ret.value_or("");
}

BladeStyles::BladeStyle* StyleBlock::getStyle() const {
    // auto gen{BladeStyles::Generator::get(name.ToStdString())};

    // std::vector<BladeStyles::Value> args;
    // for (const auto& arg : argsInfo) {
    //     if (arg.argBlock) {
    //         auto styleBlock{dynamic_cast<StyleBlock*>(arg.argBlock)};
    //         if (styleBlock) args.push_back(styleBlock->getStyle());
    //         else {
    //             auto staticBlock{dynamic_cast<StaticBlock*>(arg.argBlock)};
    //             if (staticBlock) args.push_back(staticBlock->getValue());
    //             else args.push_back(std::nullopt);
    //         }
    //     } else if (arg.numCtrl && arg.numCtrl->IsEnabled()) {
    //         args.push_back(arg.numCtrl->entry()->GetValue());
    //     } else if (arg.bitsCtrl && arg.bitsCtrl->IsEnabled()) {
    //         // TODO
    //         args.push_back(0);
    //     } else if (arg.type & BladeStyles::REFMASK) {
    //         auto refNum{(arg.type & BladeStyles::REFMASK) >> 16};
    //         args.push_back(args.at(refNum - 1));
    //     } else args.push_back(std::nullopt);
    // }

    // return gen(args);
}

bool StyleBlock::isCollapsed() const { return collapsed; }

void StyleBlock::initHelp() {
    helpButton = new wxButton(this, HELP, "?", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    Bind(wxEVT_BUTTON, [&](wxCommandEvent&){
        BladeStyles::Documentation::open(name.ToStdString());
    });
}

void StyleBlock::doOnGrab() {

}

bool StyleBlock::tryAdopt(wxWindow* window, wxPoint pos) {
        auto block{dynamic_cast<Block*>(window)};
        if (!block) return false;

        // ArgInfo* argToFill{nullptr};
        // for (auto& arg : argsInfo) {
        //     if (arg.numCtrl || arg.bitsCtrl) continue;

        //     auto typesEqual{(arg.type & block->type) & BladeStyles::FLAGMASK};
        //     if (!typesEqual) continue;

        //     auto argScreenLoc{Block::GetScreenPosition() + arg.drawLoc};
        //     auto argReg{wxRegion(argScreenLoc.x, argScreenLoc.y, size.x - arg.drawLoc.x, arg.size.y)};
        //     
        //     if (argReg.Contains(pos)) {
        //         argToFill = &arg;
        //         break;
        //     }
        // }

        // if (!argToFill) return false;

        // bool hasArg{static_cast<bool>(argToFill->argBlock)};
        // if (hasArg) {
        //     auto argStyleBlock{dynamic_cast<StyleBlock*>(argToFill->argBlock)};
        //     if (argStyleBlock) argStyleBlock->tryAdopt(window, pos);    
        // }

        // if (!hasArg) {
        //     argToFill->argBlock = block;
        //     argToFill->argBlock->Reparent(this);
        // }

        update();
        
        return true;
}

bool StyleBlock::hitTest(wxPoint point) const {
    if (point.x < 0 || point.y < 0) return false;
    if (point.x > rectSize.x || point.y > rectSize.y) return false;

    // for (const auto& arg : argsInfo) {
    //     auto argReg{wxRegion(arg.drawLoc.x, arg.drawLoc.y, arg.size.x, arg.size.y)};
    //     if (argReg.Contains(point)) return false;
    // }

    return true;
}

void StyleBlock::update(bool repaint) {
    calc();
    if (repaint) paintNow();
    moveParamBlocks();
    auto sizeToSet{collapsed ? rectSize : size};
    Block::SetSize(sizeToSet);
    Block::SetMinSize(sizeToSet);
}

void StyleBlock::recurseUpdate(bool repaint) {
    update(repaint);

    auto styleBlockParent{dynamic_cast<StyleBlock*>(GetParent())};
    if (!styleBlockParent) {
        GetParent()->Layout();
        return;
    }

    styleBlockParent->recurseUpdate(repaint);
}

void StyleBlock::collapse(bool collapse) {
    // if (argsInfo.size() == 0) return;
    if (collapse == collapsed) return;

    collapsed = collapse;
    recurseUpdate();

    wxPostEvent(GetEventHandler(), wxCommandEvent(SB_COLLAPSED));
}

void StyleBlock::showPopMenu(wxMouseEvent& evt) {
    wxMenu menu;

    menu.Append(COPY, "Copy");
    menu.Append(CUT, "Cut");
    menu.AppendSeparator();
    menu.Append(COLLAPSE, collapsed ? "Expand" : "Collapse");
    menu.AppendSeparator();
    menu.Append(HELP, "View StyleDoc");

    PopupMenu(&menu, evt.GetPosition());
}

void StyleBlock::moveParamBlocks() {
    helpButton->SetPosition(helpPos);
    // for (auto& argInfo : argsInfo) {
    //     if (argInfo.argBlock) {
    //         argInfo.argBlock->SetPosition({
    //                                           argInfo.drawLoc.x + borderThickness,
    //                                           argInfo.drawLoc.y + borderThickness
    //                                       });
    //     } else if (argInfo.numCtrl) {
    //         auto size{argInfo.numCtrl->GetBestSize()};
    //         argInfo.numCtrl->SetPosition(argInfo.drawLoc);
    //         argInfo.numCtrl->SetSize(size);
    //     }
    // }
}

void StyleBlock::calc() {
    wxPoint drawLocation{edgePadding, edgePadding};
    size.Set(drawLocation.x, drawLocation.y);
    rectSize.Set(-1, -1);

    switch (type & BladeStyles::FLAGMASK) {
        case BladeStyles::BUILTIN:
        case BladeStyles::WRAPPER:
            color = wrapperColor;
            break;
        case BladeStyles::COLOR:
            color = colorColor;
            break;
        case BladeStyles::LAYER:
            color = layerColor;
            break;
        case BladeStyles::FUNCTION:
            color = functionColor;
            break;
        case BladeStyles::TRANSITION:
            color = transitionColor;
            break;
        case BladeStyles::EFFECT:
            color = effectColor;
            break;
        default:
            color = nullptr;
    }

    auto dc{wxClientDC(this)};
    auto headerTextSize{dc.GetTextExtent(name)};
    headerTextPos = drawLocation;
    drawLocation.y += headerTextSize.y;
    if (drawLocation.y > size.y) size.y = drawLocation.y;
    if (drawLocation.x + headerTextSize.x > size.x) size.x = drawLocation.x + headerTextSize.x;
    if (drawLocation.x + headerTextSize.x > rectSize.x) rectSize.x = drawLocation.x + headerTextSize.x;

    auto& params{style->getParams()};
    if (params.size() > 0) {
        drawLocation.y += internalPadding;
        headerBarPos = drawLocation;
        drawLocation.y += internalPadding;
        if (drawLocation.y > size.y) size.y = drawLocation.y;

        drawLocation.x += edgePadding * 2;
        if (drawLocation.x > size.x) size.x = drawLocation.x;
    }

    paramsData.resize(params.size());
    for (size_t i{0}; i < params.size(); i++) {
        using namespace BladeStyles;

        auto param{params.at(i)};
        auto& data{paramsData.at(i)};
        data.colors.clear();

        auto paramType{param->getType()};
        auto paramStyle{static_cast<StyleParam*>(param)->getStyle()};
        if ((paramType & STYLETYPE) && paramStyle) {
            data.rectSize = blockMap.find(paramStyle)->second->GetSize();
        } else {
            data.rectSize.x = -1;
            data.rectSize.y = 20;
        }

        if (paramType & WRAPPER) {
            data.colors.push_back(wrapperColor);
        } if (paramType & BUILTIN) {
            data.colors.push_back(builtinColor);
        } if (paramType & FUNCTION) {
            data.colors.push_back(functionColor);
        } if (paramType & FUNCTION3D) {
            data.colors.push_back(function3DColor);
		} if (paramType & NUMBER) {
            auto numberParam{static_cast<NumberParam*>(param)};
            if (!data.control) data.control.reset(new PCUI::Numeric(this, wxID_ANY, wxEmptyString, wxDefaultSize, 0, -32768, 32768, numberParam->getNum()));
		} if (paramType & BITS) {
            auto bitsParam{static_cast<BitsParam*>(param)};
            if (!data.control) data.control.reset(new PCUI::BitsCtrl(this, wxID_ANY, 16, bitsParam->getBits(), wxEmptyString, wxDefaultSize));
		} if (paramType & BOOL) {
            auto boolParam{static_cast<BoolParam*>(param)};
            if (!data.control) data.control.reset(new PCUI::Bool(this, wxID_ANY, boolParam->getBool()));
		} if (paramType & COLOR) {
            data.colors.push_back(colorColor);
		} if (paramType & LAYER) {
            data.colors.push_back(layerColor);
		} if (paramType & TRANSITION) {
            data.colors.push_back(transitionColor);
        } if (paramType & TIMEFUNC) {
            data.colors.push_back(timeFuncColor);
		} if (paramType & EFFECT) {
            data.colors.push_back(effectColor);
		} if (paramType & LOCKUPTYPE) {
            data.colors.push_back(lockupTypeColor);
		} if (paramType & ARGUMENT) {
            data.colors.push_back(argumentColor);
        }

        data.textPos = drawLocation;
        drawLocation.y += internalPadding;
        auto argTextSize{dc.GetTextExtent(param->name)};
        drawLocation.y += argTextSize.y;
        if (drawLocation.x + argTextSize.x > size.x) size.x = drawLocation.x + argTextSize.x;
        if (drawLocation.x + argTextSize.x > rectSize.x) rectSize.x = drawLocation.x + argTextSize.x;

        data.rectPos = drawLocation;
        drawLocation.y += data.rectSize.y;
        if (drawLocation.x + data.rectSize.x > size.x){
            if (paramStyle && rectSize.x == -1) {
                // Freeze smaller size
                rectSize.x = size.x;
            }

            size.x = drawLocation.x + data.rectSize.x;
        }
        if (data.control && drawLocation.x + data.rectSize.x > rectSize.x) {
            rectSize.x = drawLocation.x + data.rectSize.x;
        }
        if (paramType & BladeStyles::FUNCTION) {
            drawLocation.y += borderThickness;
        }
        drawLocation.y += internalPadding * 2;

        if (paramType & BladeStyles::REFMASK) {
            const auto refNum{(paramType & BladeStyles::REFMASK) >> 16};
            auto refTextSize{dc.GetTextExtent(params.at(refNum - 1)->name)};
            if (drawLocation.x + refTextSize.x + (internalPadding * 2) > size.x) size.x = drawLocation.x + refTextSize.x + (internalPadding * 2);
        }
    }
    if (drawLocation.y > size.y) size.y = drawLocation.y;

    if (rectSize.x == -1) rectSize.x = size.x;
    if (rectSize.y == -1) rectSize.y = size.y;

    auto helpButtonSize{helpButton->GetSize()};
    auto hbNeededWidth{headerTextPos.x + headerTextSize.x + internalPadding + helpButtonSize.x};
    if (hbNeededWidth > size.x) size.x = hbNeededWidth;
    if (hbNeededWidth > rectSize.x) rectSize.x = hbNeededWidth;

    helpPos.x = rectSize.x - helpButtonSize.x;

    if (collapsed) {
        size.y = rectSize.y = headerTextPos.y + headerTextSize.y + internalPadding; 
    }

    rectSize.x += edgePadding;
    rectSize.y += edgePadding;

    // if (argsInfo.size() > 0) helpPos.y = (headerBarPos.y - helpButtonSize.y) / 2;
    // else helpPos.y = (rectSize.y - helpButtonSize.y) / 2;

    if (size.x < rectSize.x) size.x = rectSize.x;
    if (size.y < rectSize.y) size.y = rectSize.y;
    size.x += borderThickness; // For block borders
}

void StyleBlock::render(wxDC& dc) {
    dc.SetBrush(wxBrush(*faceColor));

    if (color) {
        dc.SetPen(wxPen(color->ChangeLightness(130), 1));
        dc.SetBrush(wxBrush(*color));
    }

    dc.SetClippingRegion(0, 0, rectSize.x, rectSize.y);
    dc.DrawRoundedRectangle(0, 0, rectSize.x, rectSize.y, 5);

    dc.DrawText(name, headerTextPos.x, headerTextPos.y);

    if (paramsData.size() > 0) {
        dc.SetPen(*textColor);
        dc.DrawLine(headerBarPos.x, headerBarPos.y, rectSize.x - edgePadding, headerBarPos.y);
    }

    if (collapsed) return;

    auto& params{style->getParams()};
    for (size_t i{0}; i < paramsData.size(); i++) {
        auto param{params.at(i)};
        auto& data{paramsData.at(i)};
        dc.SetTextForeground(*textColor);
        dc.DrawText(param->name, data.textPos.x, data.textPos.y);

        if (!(param->getType() & BladeStyles::STYLETYPE)) continue;

        auto blockBoxSize{data.rectSize};
        blockBoxSize.x = size.x;

        const wxColour* paramColor{data.colors.size() ? data.colors.at(0) : nullptr};
        if (color && paramColor) {
            dc.SetBrush(*paramColor);
            dc.SetPen(color->ChangeLightness(80));
        } else {
            dc.SetBrush(*dimColor);
            dc.SetPen(*dimColor);
        }
        dc.DrawRoundedRectangle(
                                data.rectPos.x,
                                data.rectPos.y,
                                blockBoxSize.x + (2 * borderThickness),
                                blockBoxSize.y + (2 * borderThickness),
                                6);

        if (paramColor) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(paramColor->ChangeLightness(130));
        } else {
            dc.SetPen(*dimColor);
            dc.SetBrush(*dimColor);
        }
        dc.DrawRoundedRectangle(data.rectPos.x + borderThickness, data.rectPos.y + borderThickness, blockBoxSize.x, blockBoxSize.y, 5);


        if (param->getType() & BladeStyles::REFMASK) {
            const auto refNum{(param->getType() & BladeStyles::REFMASK) >> 16};
            const auto yOffset{(data.rectSize.y - dc.GetCharHeight()) / 2};
            dc.SetTextForeground(*dimColor);
            dc.DrawText(params.at(refNum - 1)->name, data.rectPos.x + borderThickness + internalPadding, data.rectPos.y + borderThickness + yOffset);
        }
    }
}

void StyleBlock::paintEvent(wxPaintEvent& evt) {
    Block::paintEvent(evt);
    update(false);
}
