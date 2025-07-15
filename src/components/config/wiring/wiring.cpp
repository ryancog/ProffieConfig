#include "wiring.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/wiring.cpp
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

#include <memory>
#include <optional>
#include <set>
#include <variant>

#include <wx/dcmemory.h>
#include <wx/gdicmn.h>

#include <config/config.h>
#include <config/types.h>
#include <utils/clone.h>
#include <utils/image.h>
#include <utils/crypto.h>

#include "components.h"
#include "config/wiring/blades.h"

using namespace Config;
namespace Config::Wiring {

[[nodiscard]] PadTypesSet allPermissiveSet();

template<typename RET_TYPE, typename MAP_TYPE>
vector<std::shared_ptr<RET_TYPE>> getComponentsTemplate(const MAP_TYPE& map) {
    vector<std::shared_ptr<RET_TYPE>> ret;
    for (const auto& [ type, comps ] : map) {
        for (const auto& [ uid, comp ] : comps) {
            ret.push_back(comp);
        }
    }
    return ret;
}

template<typename RET_TYPE, typename MAP_TYPE>
vector<std::shared_ptr<RET_TYPE>> getComponentsByTypeTemplate(const MAP_TYPE& map, ComponentClass type) {
    auto compsIt{map.find(type)};
    if (compsIt == map.end()) return {};

    vector<std::shared_ptr<RET_TYPE>> ret;
    ret.reserve(compsIt->second.size());
    for (const auto& [ id, component ] : compsIt->second) {
        ret.push_back(component);
    }
    return ret;
}

template<typename RET_TYPE, typename MAP_TYPE>
std::shared_ptr<RET_TYPE> getComponentTemplate(const MAP_TYPE& map, ComponentID id) {
    auto compsIt{map.find(id.type.compClass)};
    if (compsIt == map.end()) return nullptr;
    auto compIt{compsIt->second.find(id.uid)};
    if (compIt == compsIt->second.end()) return nullptr;
    return compIt->second;
}

template<typename RET_TYPE, typename MAP_TYPE>
vector<std::shared_ptr<RET_TYPE>> getNetsTemplate(const MAP_TYPE& map) {
    std::vector<std::shared_ptr<RET_TYPE>> ret;
    ret.reserve(map.size());
    for (const auto& [ id, net ] : map) ret.emplace_back(net);
    return ret;
}

template<typename RET_TYPE, typename MAP_TYPE>
std::shared_ptr<RET_TYPE> getNetTemplate(const MAP_TYPE& map, UID netID) {
    auto netIt{map.find(netID)};
    if (netIt == map.end()) return nullptr;
    return netIt->second;
}

template<typename RET_TYPE, typename MAP_TYPE>
vector<std::shared_ptr<RET_TYPE>> getPadsTemplate(const MAP_TYPE& map) {
    vector<std::shared_ptr<RET_TYPE>> ret;
    ret.reserve(map.size());
    for (const auto& [ padID, pad ] : map) {
        ret.push_back(pad);
    }
    return ret;
}
template<typename RET_TYPE, typename MAP_TYPE>
std::shared_ptr<RET_TYPE> getPadTemplate(const MAP_TYPE& map, PadID id) {
    auto padIt{map.find(id)};
    if (padIt == map.end()) return nullptr;
    return padIt->second;
}

} // namespace Config::Wiring

bool Wiring::PadTypes::addType(PadType type) {
    return types.insert(type).second;
}

bool Wiring::PadTypes::removeType(PadType type) {
    auto typeIt{types.find(type)};
    if (typeIt == types.end()) return false;
    types.erase(typeIt);
    return true;
}

bool Wiring::PadTypes::addPad(FullPadID pad) {
    return pads.insert(pad).second;
}

bool Wiring::PadTypes::removePad(FullPadID pad) {
    auto padIt{pads.find(pad)};
    if (padIt == pads.end()) return false;
    pads.erase(padIt);
    return true;
}

Wiring::PadTypes Wiring::PadTypes::operator&(const PadTypes& rhs) const {
    const auto& lhs{*this};

    PadTypes ret;
    auto mixedID{mixIDs(lhs, rhs)};
    if (not mixedID) return {};

    ret.connectorID = mixedID.value();

    for (const auto& type : lhs.types) {
        if (rhs.types.find(type) != rhs.types.end()) ret.types.insert(type);
    }
    for (const auto& pad : lhs.pads) {
        if (rhs.pads.find(pad) != rhs.pads.end()) ret.pads.insert(pad);
    }

    return ret;
}

Wiring::PadTypes Wiring::PadTypes::operator|(const PadTypes& rhs) const {
    const auto& lhs{*this};

    auto mixedID{mixIDs(lhs, rhs)};
    if (not mixedID) return {};

    PadTypes ret;
    ret.connectorID = mixedID.value();

    ret.types.reserve(lhs.types.size() + rhs.types.size());
    for (const auto& type : lhs.types) ret.types.insert(type);
    for (const auto& type : rhs.types) ret.types.insert(type);
    ret.types.rehash(ret.types.size());

    ret.pads.reserve(lhs.pads.size() + rhs.pads.size());
    for (const auto& pad : lhs.pads) ret.pads.insert(pad);
    for (const auto& pad : rhs.pads) ret.pads.insert(pad);
    ret.pads.rehash(ret.pads.size());

    return std::move(ret);
}

Wiring::PadTypes Wiring::PadTypes::mask(const PadTypes& mask) const {
    auto mixedID{mixIDs(*this, mask)};
    if (not mixedID) return {};

    PadTypes ret;
    ret.connectorID = mixedID.value();

    if (not mask.pads.empty()) {
        for (const auto& pad : this->pads) {
            if (mask.pads.find(pad) == mask.pads.end()) ret.pads.insert(pad);
        }
        return ret;
    }

    for (const auto& type : this->types) {
        if (mask.types.find(type) == mask.types.end()) ret.types.insert(type);
    }
    return ret;
}

void Wiring::PadTypes::operator&=(const PadTypes& other) { *this = std::move(*this & other); }
void Wiring::PadTypes::operator|=(const PadTypes& other) { *this = std::move(*this | other); }

bool Wiring::PadTypes::isEmpty() const { 
    return types.size() == 0 and pads.size() == 0;
}

std::optional<UID> Wiring::PadTypes::mixIDs(const PadTypes& setA, const PadTypes& setB) {
    if (setA.connectorID == NULL_ID) return setB.connectorID;
    if (setB.connectorID == NULL_ID) return setA.connectorID;
    if (setA.connectorID != setB.connectorID) return std::nullopt;

    return setA.connectorID;
}

Wiring::PadTypesSet::PadTypesSet(const Pad& pad) {
    switch (pad.getParent().getID().type.compClass) {
    case ComponentClass::CONNECTOR_OUT:
        emit.connectorID = pad.getParent().getID().uid;
        break;
    default: break;
    }

    emit.pads.insert({pad.getParent().getID(), pad.getID()});
}

void Wiring::PadTypesSet::mix(const PadTypesSet& other) {
    emit |= other.emit;
    receive &= other.receive;
}

void Wiring::PadTypesSet::absorb(const PadTypesSet& other) {
    for (auto type : other.emit.types) emit.addType(type);
    for (auto type : other.receive.types) receive.addType(type);
    for (auto pad : other.emit.pads) emit.addPad(pad);
    for (auto pad : other.receive.pads) receive.addPad(pad);
}

bool Wiring::PadTypesSet::compatible(const PadTypesSet& setA, const PadTypesSet& setB) {
    auto maskA{setA.emit.mask(setB.receive)};
    if (not maskA.isEmpty()) return false;
    auto maskB{setB.emit.mask(setA.receive)};
    return maskB.isEmpty();
}

Wiring::Wiring::Wiring() {
    setProffieboardVersion(ComponentClass::PROFFIEBOARD_V3); 
}

Wiring::Wiring::Wiring(const Wiring& other) :
    mPadNetLookup(other.mPadNetLookup),
    mProffieboardID(other.mProffieboardID) {

    for (const auto& [ type, map ] : other.mComponents) {
        for (const auto& [ uid, comp ] : map) {
            auto compClone{std::shared_ptr<Component>(comp->clone())};
            compClone->mParent = this;
            mComponents[type][uid] = std::move(compClone);
        }
    }

    for (const auto& [ id, net ] : other.mNets) {
        auto netClone{std::make_shared<Net>(*net)};
        netClone->mParent = this;
        mNets[id] = std::move(netClone);
    }
}

Wiring::Wiring& Wiring::Wiring::operator=(const Wiring& other) {
    if (this == &other) return *this;

    this->~Wiring();
    new(this) Wiring(other);    

    return *this;
}

bool Wiring::Wiring::addComponent(std::shared_ptr<Component> component) { 
    if (not component) return false;

    auto& typeMap{mComponents[component->mID.type.compClass]};
    if (component->mID.uid == NULL_ID) {
        auto uid{Crypto::genUID(typeMap)};
        component->mID.uid = uid;
    }
    component->mParent = this;
    component->updatePadPositions();
    typeMap.insert({component->mID.uid, std::move(component)});
    return true;
}

std::shared_ptr<Wiring::Component> Wiring::Wiring::addComponent(ComponentType type) {
    std::shared_ptr<Component> ret;
    switch (type.compClass) {
        case ComponentClass::WS281X_STRIP:
            ret = std::make_shared<Components::WS281X>();
            break;
        case ComponentClass::SPI_STRIP:
            ret = std::make_shared<Components::SPI>();
            break;
        case ComponentClass::TRISTAR:
            ret = std::make_shared<Components::TriStar>();
            break;
        case ComponentClass::QUADSTAR:
            ret = std::make_shared<Components::QuadStar>();
            break;
        case ComponentClass::STRING_BLADE:
            ret = std::make_shared<Components::String>();
            break;
        case ComponentClass::SINGLE_DIODE:
            ret = std::make_shared<Components::SingleLED>();
            break;
        case ComponentClass::RESISTOR:
            ret = std::make_shared<Components::Resistor>();
            break;
        case ComponentClass::MOMENTARY_BUTTON:
            ret = std::make_shared<Components::MomentaryButton>();
            break;
        case ComponentClass::LATCHING_BUTTON:
            ret = std::make_shared<Components::LatchingButton>();
            break;
        case ComponentClass::TOUCH_BUTTON:
            ret = std::make_shared<Components::TouchButton>();
            break;
        case ComponentClass::RFID:
            ret = std::make_shared<Components::RFID>();
            break;
        case ComponentClass::CONNECTOR_IN:
        case ComponentClass::CONNECTOR_OUT:
        case ComponentClass::CUSTOM:
            // TODO
            break;
        case ComponentClass::NONE:
        case ComponentClass::PROFFIEBOARD_V1:
        case ComponentClass::PROFFIEBOARD_V2:
        case ComponentClass::PROFFIEBOARD_V3:
            return nullptr;
    }

    addComponent(ret);
    return ret;
}

bool Wiring::Wiring::removeComponent(ComponentID id) {
    auto comp{getComponent(id)};
    if (not comp) return false;
    if (id == mProffieboardID) return false;

    for (const auto& pad : comp->getPads()) {
        FullPadID fullPadID{ id, pad->getID() };
        auto netsConnectedToPad{getNetsConnectedToPad(fullPadID)};
        for (const auto netID : netsConnectedToPad) removeNet(netID);
    }

    auto& typeMap{mComponents[id.type.compClass]};
    if (typeMap.size() == 1) mComponents.erase(mComponents.find(id.type.compClass));
    else typeMap.erase(typeMap.find(id.uid));

    return true;
}


vector<std::shared_ptr<const Wiring::Component>> Wiring::Wiring::getComponents() const { 
    return getComponentsTemplate<const Component>(mComponents);
}
vector<std::shared_ptr<Wiring::Component>> Wiring::Wiring::getComponents() {
    return getComponentsTemplate<Component>(mComponents);
}

std::shared_ptr<const Wiring::Component> Wiring::Wiring::getComponent(ComponentID id) const {
    return getComponentTemplate<const Component>(mComponents, id);
}
std::shared_ptr<Wiring::Component> Wiring::Wiring::getComponent(ComponentID id) {
    return getComponentTemplate<Component>(mComponents, id);
}

vector<std::shared_ptr<const Wiring::Component>> Wiring::Wiring::getComponentsByType(ComponentClass type) const {
    return getComponentsByTypeTemplate<const Component>(mComponents, type);
}
vector<std::shared_ptr<Wiring::Component>> Wiring::Wiring::getComponentsByType(ComponentClass type) {
    return getComponentsByTypeTemplate<Component>(mComponents, type);
}

std::shared_ptr<Wiring::Net> Wiring::Wiring::addNet(Connection start, Connection end) {
    if (std::holds_alternative<Disconnected>(start) and std::holds_alternative<Disconnected>(end)) return nullptr;

    auto id{Crypto::genUID(mNets)};
    auto newNet{std::shared_ptr<Net>(new Net(*this, id))};

    // Must be added prior in case it's needed during setConn
    auto netIt{mNets.emplace(id, newNet).first};
    if (not newNet->setConnection(WireN::START, start) or not newNet->setConnection(WireN::END, end)) {
        mNets.erase(netIt);
        return nullptr;
    }

    return newNet;
}

bool Wiring::Wiring::removeNet(UID id) {
    auto netIt{mNets.find(id)};
    if (netIt == mNets.end()) return false;

    netIt->second->setConnection(WireN::START, Disconnected());
    netIt->second->setConnection(WireN::END, Disconnected());
    mNets.erase(netIt);
    return true;
}

vector<std::shared_ptr<const Wiring::Net>> Wiring::Wiring::getNets() const { return getNetsTemplate<const Net>(mNets); }
vector<std::shared_ptr<Wiring::Net>> Wiring::Wiring::getNets() { return getNetsTemplate<Net>(mNets); }

std::shared_ptr<const Wiring::Net> Wiring::Wiring::getNet(UID id) const { return getNetTemplate<const Net>(mNets, id); }
std::shared_ptr<Wiring::Net> Wiring::Wiring::getNet(UID id) { return getNetTemplate<Net>(mNets, id); }

bool Wiring::Wiring::setProffieboardVersion(ComponentClass pbVersion) { 
    if (pbVersion == mProffieboardID.type.compClass) return true;

    std::shared_ptr<Component> newBoard;
    switch (pbVersion) {
        case ComponentClass::PROFFIEBOARD_V1:
            newBoard = std::make_shared<Components::ProffieBoardV1>();
            break;
        case ComponentClass::PROFFIEBOARD_V2:
            newBoard = std::make_shared<Components::ProffieBoardV2>();
            break;
        case ComponentClass::PROFFIEBOARD_V3:
            newBoard = std::make_shared<Components::ProffieBoardV3>();
            break;
        default: return false;
    }
    addComponent(newBoard);
    auto oldBoard{getComponent(mProffieboardID)};

    mProffieboardID = newBoard->getID();
    if (oldBoard) { 
        // TODO: Attempt to remap pads.

        if (oldBoard->mID.type != newBoard->mID.type) {
            mComponents.erase(mComponents.find(oldBoard->mID.type.compClass));
        }
    }
    return true;
}

Wiring::ComponentID Wiring::Wiring::getProffieboardID() const { return mProffieboardID; }

std::set<UID> Wiring::Wiring::getNetsConnectedToPad(FullPadID id) const {
    std::set<UID> ret;
    auto range{mPadNetLookup.equal_range(id)};
    for (auto it{range.first}; it != range.second; it++) ret.insert(it->second);
    return ret;
}

std::optional<Wiring::NetSave> Wiring::Wiring::genNetSave(UID netID) const {
    auto net{getNet(netID)};
    if (not net) return std::nullopt;

    NetSave ret;
    ret.netID = netID;
    ret.connections = net->getConnections();
    ret.wires = net->generateWireArchive();

    return std::move(ret);
}

void Wiring::Wiring::loadNetSave(const NetSave& netSave) {
    auto newNet{std::shared_ptr<Net>(new Net(*this, netSave.netID))};

    // Must be added prior in case it's needed during setConn
    auto netIt{mNets.emplace(netSave.netID, newNet).first};
    newNet->setConnection(WireN::START, netSave.connections.first);
    newNet->setConnection(WireN::END, netSave.connections.second);
    newNet->loadWireArchive(netSave.wires);
}

void Wiring::Pad::create(Component& parent, string name, bool required) {
    auto pad{std::shared_ptr<Pad>(new Pad(parent, std::move(name), required))};
    parent.mPads.emplace(pad->mID, std::move(pad));
}

Wiring::Pad::Pad(Component& parent, string name, bool required) :
    name(std::move(name)), 
    required(required), 
    mParent(&parent), 
    mID(parent.mPads.size() ? parent.mPads.rbegin()->first + 1 : 0) {}

Wiring::PadID Wiring::Pad::getID() const { return mID; }

Wiring::PadTypesSet Wiring::Pad::calcTypes() const { 
    /* Recursion Control */
    /**/ static bool topLevel{true};
    /**/ static std::unordered_set<FullPadID> calcedPads;
    /**/ if (calcedPads.find({mParent->getID(), mID}) != calcedPads.end()) return allPermissiveSet();
    /**/ bool isTopLevel{topLevel};
    /**/ topLevel = false;
    /**/ calcedPads.insert({mParent->getID(), mID});
    /* Recursion Control */

    auto ret{mGenTypes ? mGenTypes(*this) : PadTypesSet{*this}};

    /* Recursion Control */
    /**/ if (isTopLevel) { // Recursion is done.
    /**/     calcedPads.clear();
    /**/     topLevel = true;
    /**/ }
    /* Recursion Control */
    return ret;
}

Wiring::PadTypesSet Wiring::Pad::calcDeepTypes() const { 
    auto types{calcTypes()};

    for (auto netID : mParent->mParent->getNetsConnectedToPad({mParent->getID(), mID})) {
        auto net{mParent->mParent->getNet(netID)};
        types.mix(net->calculateEffectiveNetTypes());
    }

    return types;
}

Point Wiring::Pad::getPosition() const { return mPosition; }

const Wiring::Component& Wiring::Pad::getParent() const{ return *mParent; }

Wiring::Dir Wiring::Pad::calcDirectionBias() const {
    auto compSize{mParent->getSize()};

    auto padTopDist{mPosition.y};
    auto padBottomDist{compSize.y - mPosition.y};
    auto padLeftDist{mPosition.x};
    auto padRightDist{compSize.x - mPosition.x};

    auto minDist{std::min(std::min(padLeftDist, padRightDist), std::min(padTopDist, padBottomDist))};
    if (minDist == padTopDist) return Dir::UP;
    if (minDist == padBottomDist) return Dir::DOWN;
    if (minDist == padLeftDist) return Dir::LEFT;
    if (minDist == padRightDist) return Dir::RIGHT;
    return {};
}

Wiring::Component::Component(ComponentClass cClass) : mID({ cClass }) {}

Wiring::Component::Component(const Component& other) : 
    mID(other.mID),
    mParent(other.mParent),
    mPosition(other.mPosition), 
    mOrientation(other.mOrientation),
    mIcon(other.mIcon),
    mHighlight(other.mHighlight) {

    mPads.clear();
    for (const auto& [ padID, pad ] : other.mPads) {
        auto padClone{std::make_shared<Pad>(*pad)};
        padClone->mParent = this;
        mPads[padID] = std::move(padClone);
    }
}

bool Wiring::Component::setPadTypeGenerator(PadID id, Pad::PadTypeGenerator typeGen) {
    auto pad{getPad(id)};
    if (not pad) return false;

    pad->mGenTypes = std::move(typeGen);
    return true;
}

bool Wiring::Component::setPadPosition(PadID id, Point pos) {
    auto pad{getPad(id)};
    if (not pad) return false;

    pad->mPosition = pos;
    return true;
}

void Wiring::Component::invalidateIcon() { 
    mIcon = wxNullBitmap;
    mHighlight = wxNullBitmap;
}

wxBitmap Wiring::Component::getIcon() {
    if (mIcon.IsOk()) return mIcon;

    auto rawIcon{doGetIcon()};
    switch (mOrientation) {
        case Rotation::LEFT_RIGHT:
            mIcon = rawIcon;
            break;
        case Rotation::DOWN_UP:
            mIcon = rawIcon.ConvertToImage().Rotate90(true);
            mIcon.SetScaleFactor(Image::getDPIScaleFactor());
            break;
        case Rotation::RIGHT_LEFT:
            mIcon = rawIcon.ConvertToImage().Rotate180();
            mIcon.SetScaleFactor(Image::getDPIScaleFactor());
            break;
        case Rotation::UP_DOWN:
            mIcon = rawIcon.ConvertToImage().Rotate90(false);
            mIcon.SetScaleFactor(Image::getDPIScaleFactor());
            break;
    }

    return mIcon;
}

// TODO: It'd be nice if this were faster for larger components (where currently there's a lot of pixel-by-pixel checks),
// but I'm not sure there's a good way to do that. It'll probably be more optimizing order or methodology rather than
// actually changing the fundamental logic.
wxBitmap Wiring::Component::getHighlight(const wxColour& color) {
    if (
            mHighlight.IsOk() and
            (color == wxNullColour or
             color == mHighlightColor)
       ) return mHighlight;

    auto icon{getIcon()};
    if (color != wxNullColour) mHighlightColor = color;

    const auto scaleFactor{static_cast<int32>(icon.GetScaleFactor())};
    const auto scaledBorderThickness{HIGHLIGHT_THICKNESS * scaleFactor};
    mHighlight.Create(scaleFactor * (icon.GetDIPSize() + Point{HIGHLIGHT_THICKNESS * 2, HIGHLIGHT_THICKNESS * 2}), 32);
    mHighlight.UseAlpha();
    auto bmpImage{icon.ConvertToImage()};

    auto bmpImageSize{icon.GetSize()};
    vector<vector<bool>> xHits{static_cast<uint64>(bmpImageSize.x)};
    for (auto& col : xHits) col.resize(static_cast<uint64>(bmpImageSize.y));

    auto colorWithAlpha{[](const wxColour& color, uint8 alpha) -> wxColour {
        return { color.Red(), color.Green(), color.Blue(), static_cast<uint8>(float64(color.Alpha()) / std::numeric_limits<uint8>::max() * alpha) };
    }};
    auto getPixelColor{[](wxDC& bmpDC, Point pos) -> wxColour {
#       ifdef __WXMAC__
        // TODO: Figure out a good way to do this for macOS.
        return { 0, 0, 0, 0 };
#       else
        wxColour ret;
        bmpDC.GetPixel(pos, &ret);
        return ret;
#       endif
    }};

    {
        wxMemoryDC bmpDC{mHighlight};
        bmpDC.SetBrush(mHighlightColor);

        for (auto yIdx{0}; yIdx < bmpImageSize.y; ++yIdx) {
            int32 start{-1};
            uint8 startAlpha{};
            int32 end{};
            uint8 endAlpha{};
            for (auto xIdx{0}; xIdx < bmpImageSize.x; ++xIdx) {
                auto alpha{bmpImage.GetAlpha(xIdx, yIdx)};
                if (alpha and start == -1) {
                    start = xIdx;
                    startAlpha = alpha;
                } else if (alpha and start != -1) {
                    end = xIdx;
                    endAlpha = alpha;
                }
            }
            if (start == -1) continue;

            xHits[start][yIdx] = true;
            xHits[end][yIdx] = true;
            auto bmpStart{start + scaledBorderThickness};
            auto bmpEnd{end + scaledBorderThickness};

            bmpDC.SetPen(colorWithAlpha(mHighlightColor, startAlpha));
            bmpDC.DrawPoint(bmpStart - scaledBorderThickness, yIdx + scaledBorderThickness);
            bmpDC.SetPen(colorWithAlpha(mHighlightColor, endAlpha));
            bmpDC.DrawPoint(bmpEnd + scaledBorderThickness, yIdx + scaledBorderThickness);

            bmpDC.SetPen(mHighlightColor); 
            bmpDC.DrawLine(bmpStart - scaledBorderThickness + 1, yIdx + scaledBorderThickness, bmpEnd + scaledBorderThickness - 1, yIdx + scaledBorderThickness);
        }

        for (auto xIdx{0}; xIdx < bmpImageSize.x; ++xIdx) {
            int32 start{-1};
            uint8 startAlpha{};
            int32 end{};
            uint8 endAlpha{};
            for (auto yIdx{0}; yIdx < bmpImageSize.y; ++yIdx) {
                auto alpha{bmpImage.GetAlpha(xIdx, yIdx)};
                if (alpha and start == -1) {
                    start = yIdx;
                    startAlpha = alpha;
                } else if (alpha and start != -1) {
                    end = yIdx;
                    endAlpha = alpha;
                }
            }
            if (start == -1) continue;

            auto bmpStart{start + scaledBorderThickness};
            auto bmpEnd{end + scaledBorderThickness};

            if (xHits[xIdx][start]) {
                bmpDC.SetPen(wxNullPen);
                bmpDC.SetLogicalFunction(wxCLEAR);
                bmpDC.DrawCircle(xIdx + scaledBorderThickness, bmpStart, scaledBorderThickness - 1);
                bmpDC.SetLogicalFunction(wxCOPY);
                bmpDC.DrawCircle(xIdx + scaledBorderThickness, bmpStart, scaledBorderThickness - 1);
            } else {
                auto curColor{getPixelColor(bmpDC, { xIdx + scaledBorderThickness, bmpStart - scaledBorderThickness })};
                auto candidateColor{colorWithAlpha(mHighlightColor, startAlpha)};
                if (curColor.Alpha() < candidateColor.Alpha()) {
                    bmpDC.SetPen(candidateColor);
                    bmpDC.DrawPoint(xIdx + scaledBorderThickness, bmpStart - scaledBorderThickness);
                }
            }

            if (xHits[xIdx][end]) {
                bmpDC.SetPen(wxNullPen);
                bmpDC.SetLogicalFunction(wxCLEAR);
                bmpDC.DrawCircle(xIdx + scaledBorderThickness, bmpEnd, scaledBorderThickness - 1);
                bmpDC.SetLogicalFunction(wxCOPY);
                bmpDC.DrawCircle(xIdx + scaledBorderThickness, bmpEnd, scaledBorderThickness - 1);
            } else {
                auto curColor{getPixelColor(bmpDC, { xIdx + scaledBorderThickness, bmpEnd + scaledBorderThickness })};
                auto candidateColor{colorWithAlpha(mHighlightColor, endAlpha)};
                if (curColor.Alpha() < candidateColor.Alpha()) {
                    bmpDC.SetPen(candidateColor);
                    bmpDC.DrawPoint(xIdx + scaledBorderThickness, bmpEnd + scaledBorderThickness);
                }
            }

            bmpDC.SetPen(mHighlightColor); 
            bmpDC.SetLogicalFunction(wxCLEAR);
            bmpDC.DrawLine(xIdx + scaledBorderThickness, bmpStart - scaledBorderThickness + 1, xIdx + scaledBorderThickness, bmpEnd + scaledBorderThickness - 1);
            bmpDC.SetLogicalFunction(wxCOPY);
            bmpDC.DrawLine(xIdx + scaledBorderThickness, bmpStart - scaledBorderThickness + 1, xIdx + scaledBorderThickness, bmpEnd + scaledBorderThickness - 1);
        }
    }

    mHighlight.SetScaleFactor(scaleFactor);
    return mHighlight;
}

Point Wiring::Component::getSize() { 
    // Sucks if the implementing component didn't give us an icon with an evenly-divisible size.
    //
    // That's not this function's problem. (Or a `Component` problem in general)
    return getIcon().GetDIPSize() / GRID_MULTIPLIER; 
}

Point Wiring::Component::getUnorientedSize() {
    Point ret;
    switch (mOrientation) {
        default: 
            ret = getSize();
            break;
        case Rotation::UP_DOWN:
        case Rotation::DOWN_UP:
            auto size{getSize()};
            ret.x = size.y;
            ret.y = size.x;
            break;
    }
    return ret;
}

Point Wiring::Component::getPosition() const { return mPosition; }

void Wiring::Component::setOrientation(Rotation orient) {
    mOrientation = orient;
    invalidateIcon();
    updatePadPositions();
}

Wiring::Rotation Wiring::Component::getOrientation() const { return mOrientation; }

std::set<UID> Wiring::Component::getConnectedNets() const {
    std::set<UID> ret;
    for (const auto& pad : getPads()) {
        ret.merge(mParent->getNetsConnectedToPad({ mID, pad->getID() }));
    }
    return std::move(ret);
}

vector<std::shared_ptr<const Wiring::Pad>> Wiring::Component::getPads() const { return getPadsTemplate<const Pad>(mPads); }
vector<std::shared_ptr<Wiring::Pad>> Wiring::Component::getPads() { return getPadsTemplate<Pad>(mPads); }

std::shared_ptr<const Wiring::Pad> Wiring::Component::getPad(PadID id) const { return getPadTemplate<const Pad>(mPads, id); }
std::shared_ptr<Wiring::Pad> Wiring::Component::getPad(PadID id) { return getPadTemplate<Pad>(mPads, id); }

Wiring::ComponentID Wiring::Component::getID() const { return mID; }

const Wiring::Wiring& Wiring::Component::getParent() const { return *mParent; }
Wiring::Wiring& Wiring::Component::getParent() { return *mParent; }

void Wiring::Component::move(Point newPoint, bool pruneWires) {
    vector<std::pair<std::shared_ptr<Net>, WireN>> wires;

    for (const auto& [ padID, pad ] : mPads) {
        for (auto netID : mParent->getNetsConnectedToPad({mID, padID})) {
            auto net{mParent->getNet(netID)};
            auto [ startConn, endConn ]{net->getConnections()};
            if (std::holds_alternative<FullPadID>(startConn) and std::get<FullPadID>(startConn).compID == mID) {
                wires.emplace_back(net, WireN::START);
            }
            if (std::holds_alternative<FullPadID>(endConn) and std::get<FullPadID>(endConn).compID == mID) {
                wires.emplace_back(net, WireN::END);
            }
        }
    }

    auto moveDelta{newPoint - mPosition};
    for (const auto& [ net, wireN ] : wires) {
        auto wireInfo{net->getWireInfo(wireN)};
        if (net->getWires().size() == 1) {
            net->addWire(wireN, moveDelta.x, Net::Direction::HORIZONTAL);
            net->addWire(wireN, moveDelta.y, Net::Direction::VERTICAL);
            continue;
        }
        switch (wireInfo->dir) {
            case Net::Direction::HORIZONTAL:
                net->moveWire(wireN, moveDelta.y, pruneWires);
                net->extendWire(wireN, moveDelta.x, pruneWires);
                break;
            case Net::Direction::VERTICAL:
                net->moveWire(wireN, moveDelta.x, pruneWires);
                net->extendWire(wireN, moveDelta.y, pruneWires);
                break;
            default: break;
        }
    }
    mPosition = newPoint;
}

void Wiring::Component::updatePadPositions() {
    doSetPadPositions();
    if (mOrientation == Rotation::LEFT_RIGHT) return;
    auto size{getUnorientedSize()};

    for (const auto& pad : getPads()) {
        auto padPos{pad->getPosition()};
        switch (mOrientation) {
            case Rotation::DOWN_UP:
                pad->mPosition = {size.y - padPos.y, padPos.x};
                break;
            case Rotation::RIGHT_LEFT:
                pad->mPosition = {size.x - padPos.x, size.y - padPos.y};
                break;
            case Rotation::UP_DOWN:
                pad->mPosition = {padPos.y, size.x - padPos.x};
                break;
            default: break;
        }
    }
}

void Wiring::Component::pruneConnectedNets() {
    for (const auto& [ padID, pad ] : mPads) {
        for (auto netID : mParent->getNetsConnectedToPad({mID, padID})) {
            auto net{mParent->getNet(netID)};
            net->pruneWires();
        }
    }

}

Wiring::Net::Net(Wiring& parent, UID id) : Tracked(id), mParent(&parent) {}

Wiring::Net::~Net() {
    removeConnectionFromLookup(mStartConn);
    removeConnectionFromLookup(mEndConn);
}

std::unordered_set<Wiring::FullPadID> Wiring::Net::calculateEndPoints() const {
    std::unordered_set<FullPadID> ret;
    if (std::holds_alternative<FullPadID>(mStartConn)) ret.insert(std::get<FullPadID>(mStartConn));
    if (std::holds_alternative<FullPadID>(mEndConn)) ret.insert(std::get<FullPadID>(mEndConn));

    return std::move(ret);
}

Wiring::PadTypesSet Wiring::Net::calculateEffectiveNetTypes() const {
    PadTypesSet ret{allPermissiveSet()};

    auto endPoints{calculateEndPoints()};
    std::unordered_set<FullPadID> checkedEndPoints{};
    std::unordered_set<UID> checkedNets{};
    for (auto endPointIt{endPoints.begin()}; endPointIt != endPoints.end(); ++endPointIt) {
        if (checkedEndPoints.find(*endPointIt) != checkedEndPoints.end()) continue;
        checkedEndPoints.insert(*endPointIt);

        auto netIDs{mParent->getNetsConnectedToPad(*endPointIt)};
        for (const auto& netID : netIDs) {
            if (checkedNets.find(netID) != checkedNets.end()) continue;
            checkedNets.insert(netID);
            endPoints.merge(calculateEndPoints());
            // Yes, this will be incremented, and yes, 
            // this is probably redundant if there's multiple nets, but that's fine.
            endPointIt = endPoints.begin();
        }
    }

    for (const auto& padID : endPoints) {
        auto padTypes{mParent->getComponent(padID.compID)->getPad(padID.padID)->calcTypes()};
        ret.mix(padTypes);
    }

    return std::move(ret);
}

void Wiring::Net::flipDir(Direction& dir) {
    if (dir == Direction::NONE) return;

    if (dir == Direction::HORIZONTAL) dir = Direction::VERTICAL;
    else dir = Direction::HORIZONTAL;
}

Wiring::Net::Direction Wiring::Net::calcEndDir() {
    auto ret{startDir};
    if (mWires.size() % 2 == 0) flipDir(ret);
    return ret;
}

Point Wiring::Net::getStartPos() {
    auto wireStart{mWires.begin()->second.start};
    return startDir == Direction::HORIZONTAL ? 
        Point{ wireStart, offset } : 
        Point{ offset, wireStart };
}

Point Wiring::Net::getEndPos() {
    auto pos{getStartPos()};
    auto traceDir{startDir};
    for (const auto& [ wireN, wire ] : mWires) {
        switch (traceDir) {
            case Direction::HORIZONTAL:
                pos.x = wire.end;
                break;
            case Direction::VERTICAL:
                pos.y = wire.end;
                break;
            default: break;
        }
        flipDir(traceDir);
    }
    return pos;
}

std::pair<Wiring::Connection, Wiring::Connection> Wiring::Net::getConnections() const { return { mStartConn, mEndConn }; }

bool Wiring::Net::setConnection(WireN wireN, Connection conn) {
    Connection *oldConn{nullptr};
    WireMap::iterator wireIt;
    if (mWires.size()) {
        if (wireN == mWires.begin()->first) wireN = WireN::START;
        else if (wireN == std::prev(mWires.end())->first) wireN = WireN::END;
    }
    if (wireN == WireN::START) {
        oldConn = &mStartConn;
        if (mWires.size()) wireIt = mWires.begin();
    } else if (wireN == WireN::END) {
        oldConn = &mEndConn;
        if (mWires.size()) wireIt = std::prev(mWires.end());
    } else return false;

    if (*oldConn == conn) return true;
    auto netTypesSet{calculateEffectiveNetTypes()};
    if (std::holds_alternative<FullPadID>(conn)) {
        auto fullPadID{std::get<FullPadID>(conn)};
        // Check if comp exists
        auto comp{mParent->getComponent(fullPadID.compID)};
        if (not comp) return false;
        auto pad{comp->getPad(fullPadID.padID)};
        if (not pad) return false;

        // Check if types permit
        auto padTypesSet{pad->calcDeepTypes()};
        if (not PadTypesSet::compatible(padTypesSet, netTypesSet)) return false;
    } 

    removeConnectionFromLookup(*oldConn);
    addConnectionToLookup(conn);

    (*oldConn) = conn;
    return true;
}

std::optional<Wiring::Net::WireInfo> Wiring::Net::getWireInfo(WireN wireN) {
    if (mWires.size() == 0) return std::nullopt;
    if (wireN == WireN::START) wireN = mWires.begin()->first;
    else if (wireN == WireN::END) wireN = mWires.rbegin()->first;

    auto wireIt{mWires.find(wireN)};
    if (wireIt == mWires.end()) return std::nullopt;

    auto retDir{startDir};
    if ((mWires.begin()->first - wireIt->first) % 2 != 0) flipDir(retDir);
    return WireInfo{ wireIt->second, wireN, retDir };
}

Wiring::Net::WireArchive Wiring::Net::generateWireArchive() const {
    WireArchive ret;
    ret.offset = offset;
    ret.startDir = startDir;
    ret.wires = getWires();
    return ret;
}

void Wiring::Net::loadWireArchive(const WireArchive& archive) {
    offset = archive.offset;
    startDir = archive.startDir;
    mWires = archive.wires;
}

Wiring::Net::WireMap Wiring::Net::getWires() const { return mWires; }

Wiring::Wiring& Wiring::Net::getParent() { return *mParent; }

bool Wiring::Net::addWire(WireN wireN, int32 len, Direction wireDir, bool ignoreZeroLength) {
    if (ignoreZeroLength and len == 0) return true;

    if (mWires.size() == 0) {
        if (wireDir == Direction::NONE) return false;
        FullPadID padID{};
        if (wireN == START) { // These are flipped to stay consistent with the rest of the logic.
                              // For example, you'd expect multiple calls using `START` to prepend.
            if (not std::holds_alternative<FullPadID>(mEndConn)) return false;
            padID = std::get<FullPadID>(mEndConn);
        } else if (wireN == END) {
            if (not std::holds_alternative<FullPadID>(mStartConn)) return false;
            padID = std::get<FullPadID>(mStartConn);
        } else return false;

        auto component{mParent->getComponent(padID.compID)};
        auto pad{component->getPad(padID.padID)};
        auto padPos{component->getPosition() + pad->getPosition()};
        startDir = wireDir;
        switch (startDir) {
            case Direction::HORIZONTAL:
                offset = padPos.y;
                mWires.emplace(WireN(0), Wire{padPos.x, padPos.x + len});
                break;
            case Direction::VERTICAL:
                offset = padPos.x;
                mWires.emplace(WireN(0), Wire{padPos.y, padPos.y + len});
                break;
            default: break;
        }
        return true;
    }

    if (wireN == START) {
        wireN = WireN(mWires.begin()->first - 1);
        // The (perpendicular) direction this wire should be going to be considered "new"
        auto perpDir{startDir}; 
        flipDir(perpDir);

        if (wireDir == Direction::NONE) wireDir = perpDir;
        else if (wireDir != perpDir) return extendWire(START, len);

        int32 endPos{mWires.size() == 1 ? offset : /* > 2 */ mWires.find(WireN(wireN + 2))->second.start};
        int32 newOffset{mWires.find(WireN(wireN + 1))->second.start};
        mWires.emplace(wireN, Wire{endPos + len, endPos});
        startDir = wireDir;
        offset = newOffset;
    }
    else if (wireN == END) {
        wireN = WireN(mWires.rbegin()->first + 1);
        auto perpDir{this->startDir}; 
        if (std::abs(wireN - mWires.begin()->first) % 2 == 1) flipDir(perpDir);

        if (wireDir == Direction::NONE) wireDir = perpDir;
        else if (wireDir != perpDir) return extendWire(END, len);

        int32 startPos{mWires.size() == 1 ? offset : /* > 2 */ mWires.find(WireN(wireN - 2))->second.end};
        mWires.emplace(wireN, Wire{startPos, startPos + len});
    } else return false;

    return true;
}

bool Wiring::Net::extendWire(WireN wireN, int32 dist, bool prune) {
    if (mWires.size() == 0) return false;
    if (dist == 0) return true;

    if (wireN == WireN::START) {
        mWires.begin()->second.start += dist;
    } else if (wireN == WireN::END) {
        mWires.rbegin()->second.end += dist;
    } else return false;

    if (prune) pruneWires();
    return true;
}

bool Wiring::Net::moveWire(WireN wireN, int32 dist, bool prune) {
    if (mWires.size() == 0) return false;
    if (dist == 0) return true;

    auto wireRangeStart{mWires.begin()->first};
    auto wireRangeEnd{mWires.rbegin()->first};
    if (wireN == START) wireN = wireRangeStart;
    else if (wireN == END) wireN = wireRangeEnd;
    if (wireN < wireRangeStart or wireN > wireRangeEnd) return false;

    auto nextWireIt{mWires.find(WireN(wireN + 1))};
    auto prevWireIt{mWires.find(WireN(wireN - 1))};

    if (wireN == wireRangeStart) offset += dist;

    if (nextWireIt != mWires.end()) nextWireIt->second.start += dist;
    if (prevWireIt != mWires.end()) prevWireIt->second.end += dist;
    
    if (prune) pruneWires();
    return true;
}

bool Wiring::Net::deleteWire(WireN wireN, UID *newID) {
    if (newID) *newID = getUID();
    if (mWires.size() == 0) return false;

    auto wireRangeStart{mWires.begin()->first};
    auto wireRangeEnd{mWires.rbegin()->first};
    if (wireN == START) wireN = wireRangeStart;
    else if (wireN == END) wireN = wireRangeEnd;

    if (wireN < wireRangeStart or wireN > wireRangeEnd) return false;

    auto wireIt{mWires.find(wireN)};
    // Store for later if we need to know where it was.
    auto erasedWire{wireIt->second};
    mWires.erase(wireIt);

    if (wireN == wireRangeStart) setConnection(WireN::START, Disconnected());
    if (wireN == wireRangeEnd) setConnection(WireN::END, Disconnected());

    if (std::holds_alternative<Disconnected>(mStartConn) and std::holds_alternative<Disconnected>(mEndConn)) {
        mParent->removeNet(getUID());
        if (newID) *newID = NULL_ID;
        return true;
    }

    // If erasing towards the end it's easy, we can just delete things. (and worst case we end up empty)
    // If erasing from the front, we need to keep track of how we're moving to update the offset and dir.
    if (std::holds_alternative<Disconnected>(mEndConn)) {
        for (int32 idx{wireN + 1}; idx <= wireRangeEnd; idx++) {
            mWires.erase(mWires.find(WireN(idx)));
        }
    } 
    if (std::holds_alternative<Disconnected>(mStartConn)) {
        for (int32 idx{wireRangeStart}; idx < wireN; idx++) {
            auto wireIt{mWires.find(WireN(idx))};
            mWires.erase(wireIt);
        }

        if ((wireN + 1) % 2) flipDir(startDir);
        offset = erasedWire.end;
    }

    if (mWires.size() == 0) {
        // Purge ourselves now that we're empty.
        mParent->removeNet(getUID());
        if (newID) *newID = NULL_ID;
        return true;
    }

    wireRangeStart = mWires.begin()->first;
    wireRangeEnd = mWires.rbegin()->first;
    // If the wireN is still within the range, things must be connected and we need to split.
    if (wireN > wireRangeStart and wireN < wireRangeEnd) {
        auto newNet{mParent->addNet(Disconnected(), mEndConn)};
        if (newID) *newID = newNet->getUID();
        setConnection(WireN::END, Disconnected());
        
        vector<Wire> newNetWires;
        for (auto idx{wireN + 1}; idx <= wireRangeEnd; idx++) {
            auto wireIt{mWires.find(WireN(idx))};
            newNetWires.push_back(wireIt->second);
            mWires.erase(wireIt);
        }

        // Assume there was something else that stopped the creation of the net,
        // this isn't an error.
        if (not newNet) return true;

        newNet->mWires.clear();
        Direction newDir{this->startDir};
        if (std::abs(wireN + 1 - mWires.begin()->first) % 2 == 1) flipDir(newDir);
        newNet->startDir = newDir;
        newNet->offset = erasedWire.end;
        int32 idx{0};
        for (auto wireIt{newNetWires.begin()}; wireIt != newNetWires.end(); wireIt++) {
            newNet->mWires.emplace(WireN(idx), *wireIt);
            idx++;
        }
    }

    return true;
}

void Wiring::Net::addConnectionToLookup(Connection conn) {
    if (std::holds_alternative<Disconnected>(conn)) return;

    if (std::holds_alternative<FullPadID>(conn)) {
        mParent->mPadNetLookup.insert({std::get<FullPadID>(conn), getUID()});
    } 
}

void Wiring::Net::removeConnectionFromLookup(Connection conn) {
    if (std::holds_alternative<Disconnected>(conn)) return;

    if (std::holds_alternative<FullPadID>(conn)) {
        auto padConnRange{mParent->mPadNetLookup.equal_range(std::get<FullPadID>(conn))};
        for (auto padIt{padConnRange.first}; padIt != padConnRange.second; padIt++) {
            if (padIt->second == getUID()) {
                mParent->mPadNetLookup.erase(padIt);
                return;
            }
        }
    } 
}

void Wiring::Net::pruneWires(bool *destroyed, WireN *tracker) {
    if (destroyed) *destroyed = false;
    if (mWires.size() == 0) return;

    auto firstWireIt{mWires.begin()};
    while (firstWireIt->second.start == firstWireIt->second.end) {
        if (
                (std::holds_alternative<FullPadID>(mStartConn) or
                 std::holds_alternative<FullPadID>(mEndConn)) and
                mWires.size() == 1
           ) break;

        offset = firstWireIt->second.end;
        flipDir(startDir);
        firstWireIt = mWires.erase(firstWireIt);
    }

    // If we ended up purging all our wires, setup ourselves for destruction.
    if (mWires.size() == 0) {
        mParent->removeNet(getUID());
        if (destroyed) *destroyed = true;
        return;
    }

    // If we're here, at least the first wire must exist and have some substance.
    for (auto wireIt{std::next(mWires.begin())}; wireIt != mWires.end(); wireIt++) {
        if (wireIt->second.start == wireIt->second.end) {
            auto nextWireIt{mWires.erase(wireIt)};
            if (nextWireIt == mWires.end()) break;
            // If there is a next wire, it would've been parallel to the previous one,
            // and thus it needs to be removed and appended to the previous one.
            auto nextWire{nextWireIt->second};
            auto prevWireIt{std::prev(nextWireIt)};
            mWires.erase(nextWireIt);
            prevWireIt->second.end = nextWire.end;
            // Update the tracker index so that it's ready to be shifted
            // during cleanup.
            if (tracker) *tracker = prevWireIt->first;
            // Do this so that on next (if there is one), wireIt++ will advance to nextWire.
            wireIt = prevWireIt;
        }
    }

    // Update wire indexes
    auto wiresAfterPrune{mWires};
    mWires.clear();
    auto wireIdx{0};
    for (const auto& [ preWireN, wire ] : wiresAfterPrune) {
        auto wireN{WireN(wireIdx++)};

        if (tracker and preWireN == *tracker) {
            *tracker = wireN;
            // Prevent future updates
            tracker = nullptr;
        }
        mWires[wireN] = wire;
    }
}

[[nodiscard]] Wiring::PadTypesSet Wiring::allPermissiveSet() {
    PadTypesSet ret;
    ret.receive.types = {
        PadType::BATT_NEG,
        PadType::BATT_POS,
        PadType::POW_5V,
        PadType::POW_3V3,
        PadType::SD_POWER,
        PadType::CONTROLLED_NEG,
        PadType::CONTROLLED_POS,
        PadType::SPEAKER_NEG,
        PadType::SPEAKER_POS,
        PadType::SER_RX,
        PadType::SER_TX,
        PadType::I2C_SDA,
        PadType::I2C_SCL,
        PadType::I2S_SDATA,
        PadType::I2S_SCLK,
        PadType::I2S_FS,
        PadType::SPI_MOSI,
        PadType::SPI_MISO,
        PadType::SPI_CLK,
        PadType::NPXL_DATA,
        PadType::GPIO,
        PadType::BUTTON,
        PadType::BITBANG_DATA,
        PadType::BITBANG_CLK,
    };
    return std::move(ret);
}
