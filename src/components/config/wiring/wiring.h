#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/wiring.h
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

#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include <wx/bitmap.h>

#include <config/types.h>
#include <utils/clone.h>
#include <utils/position.h>
#include <utils/types.h>

#include "../private/export.h"

namespace Config {
struct Config; // Forward declare for later.
} // namespace Config

namespace Config::Wiring {

struct Component;
struct Net;
struct Pad;
struct FullPadID;
struct Wiring;
struct NetSave;
// To disambiguate. These units will be multiplied by a grid size multiplier.
// E.g. So that 1 and 2 are a set grid spacing apart, making movement easier for the user.
using GridUnit = int32;
// Describes the electrical goal of a `Net` end; i.e. Where the `Net` is supposed to go or connect.
using Connection = std::variant<std::monostate, FullPadID>;
using Disconnected = std::monostate;

constexpr auto GRID_MULTIPLIER{10};

constexpr auto HIGHLIGHT_THICKNESS{2};
const wxColour HIGHLIGHT_COLOR{ 0x60, 0xcc, 0xff, 0x5F };

// The ID associated with the default connector area, i.e. That
// in which things are connected directly to the proffieboard itself.
constexpr UID BOARD_CONNECTOR_AREA_UID{0};

enum class PadType {
    // Perhaps unintuitively, GND/BATT_NEG is considered to be
    // emitted from the board and received by components.
    BATT_NEG        = 1 << 0,
    BATT_POS        = 1 << 1,
    
    POW_5V          = 1 << 2,
    POW_3V3         = 1 << 3,
    SD_POWER        = 1 << 4,

    CONTROLLED_NEG  = 1 << 5,
    CONTROLLED_POS  = 1 << 6,
                              
    SPEAKER_NEG     = 1 << 7,
    SPEAKER_POS     = 1 << 8,

    SER_RX          = 1 << 9,
    SER_TX          = 1 << 10,
                              
    I2C_SDA         = 1 << 11,
    I2C_SCL         = 1 << 12,
                              
    I2S_SDATA       = 1 << 13,
    I2S_SCLK        = 1 << 14,
    I2S_FS          = 1 << 15,

    SPI_MOSI        = 1 << 16,
    SPI_MISO        = 1 << 17,
    SPI_CLK         = 1 << 18,

    NPXL_DATA       = 1 << 19,

    GPIO            = 1 << 20,
    BUTTON          = 1 << 21,

    BITBANG_DATA    = 1 << 22,
    BITBANG_CLK     = 1 << 23,
};

enum class ComponentClass {
    NONE = 0,
    PROFFIEBOARD_V1,
    PROFFIEBOARD_V2,
    PROFFIEBOARD_V3,

    WS281X_STRIP,
    SPI_STRIP,
    TRISTAR,
    QUADSTAR,
    STRING_BLADE,
    SINGLE_DIODE,

    RESISTOR,

    MOMENTARY_BUTTON,
    LATCHING_BUTTON,
    TOUCH_BUTTON,

    RFID,

    CUSTOM,

    // The side receiving connections. Probably from the proffieboard.
    // Per set there can only be one.
    CONNECTOR_IN,
    // The side sending out the received connections. Dictates the `connectorID` of
    // a `PadTypeSet`. There may be multiple if the connector has BladeID or Blade Detect enabled.
    CONNECTOR_OUT,
};

struct CONFIG_EXPORT ComponentType {
    ComponentClass compClass{ComponentClass::NONE};
    UID customID{NULL_ID};

    auto operator<=>(const ComponentType&) const = default;
};

template<typename T>
[[nodiscard]] T switchBoard(ComponentClass boardVersion, T pb1Opt, T pb2Opt, T pb3Opt) {
    using namespace Wiring;
    switch (boardVersion) {
        case ComponentClass::PROFFIEBOARD_V1: return pb1Opt;
        case ComponentClass::PROFFIEBOARD_V2: return pb2Opt;
        default:
        case ComponentClass::PROFFIEBOARD_V3: return pb3Opt;
    }
}

// Identifies a component based on `type` and `uid`.
struct CONFIG_EXPORT ComponentID final {
    ComponentType type{ComponentClass::NONE};
    UID uid{NULL_ID};

    auto operator<=>(const ComponentID&) const = default;
};

// Unlike Config::UID, these are intentionally sequential and more managed.
using PadID = uint32;

// Fully-qualified representation of a pad. Valid for net endpoints.
struct CONFIG_EXPORT FullPadID final {
    ComponentID compID;
    PadID padID;

    auto operator<=>(const FullPadID&) const = default;
};

// Wire number in Net
enum WireN : int32 {
    START = std::numeric_limits<std::underlying_type_t<WireN>>::min(),
    END = std::numeric_limits<std::underlying_type_t<WireN>>::max(),
};

// Fully identifies a specific location on a `Net` by way of `WireN` and pos
// Each `WireID` is unique and only one net may be connected to a specific
// `WireID` at once.
struct CONFIG_EXPORT WireID final {
    UID netID{NULL_ID};
    WireN wireN{START};

    auto operator<=>(const WireID&) const = default;
};

} // namespace Config::Wiring
// The `Wiring` namespace splits here to allow definition of template specializations.

namespace HashBits {
inline constexpr uint64 combineHash(uint64 hash1, uint64 hash2) { return hash1 ^ (hash2 << 1); }
} // namespace HashBits

namespace std {
template<>
struct hash<Config::Wiring::ComponentType> {
    uint64 operator()(const Config::Wiring::ComponentType& compType) const noexcept {
        auto classHash{hash<Config::Wiring::ComponentClass>{}(compType.compClass)};
        auto customIDHash{hash<decltype(compType.customID)>{}(compType.customID)};
        return HashBits::combineHash(classHash, customIDHash);
    }
};
template<>
struct hash<Config::Wiring::WireID> {
    uint64 operator()(const Config::Wiring::WireID& fullWireID) const noexcept {
        auto netHash{hash<Config::UID>{}(fullWireID.netID)};
        auto wireHash{hash<Config::Wiring::WireN>{}(fullWireID.wireN)};
        return HashBits::combineHash(netHash, wireHash);
    }
};
template<>
struct hash<Config::Wiring::ComponentID> {
    uint64 operator()(const Config::Wiring::ComponentID& compID) const noexcept {
        auto typeHash{hash<Config::Wiring::ComponentType>{}(compID.type)};
        auto uidHash{hash<Config::UID>{}(compID.uid)};
        return HashBits::combineHash(typeHash, uidHash);
    }
};
template<>
struct hash<Config::Wiring::FullPadID> {
    uint64 operator()(const Config::Wiring::FullPadID& fullPadID) const noexcept {
        auto compIDHash{hash<Config::Wiring::ComponentID>{}(fullPadID.compID)};
        auto padIDHash{hash<Config::Wiring::PadID>{}(fullPadID.padID)};
        return HashBits::combineHash(compIDHash, padIDHash);
    }
};
} // namespace std

namespace Config::Wiring {

// Specifies a set of padTypes and padIDs which a particular pad may
// "emit" or "receive."
//
// A list of all endpoints in a net is collected, then for each endpoint,
// all other endpoints' outputs are combined (effectively an or). 
// This output list is then checked against the given pad's input to determine compatibility.
// 
// In essence, the output list is masked by the valid inputs, and if there's anything
// left unmasked (and thus not allowed by the input), a connection would be invalid.
//
// If an input has `pads` specified, then the `types` are ignored and the masking and
// evaluation happens only for the `pads` to determine compatibility.
//
// For emitting, `pads` should probably have size() == 1 and be set to the 
// emitting pad. (If it's not that's just weird, what are you doing?) 
struct CONFIG_EXPORT PadTypes final {
    // Just convenience functions. Nothing special.
    // Return true or false depending on success.
    bool addType(PadType);
    bool removeType(PadType);
    bool addPad(FullPadID);
    bool removePad(FullPadID);

    std::unordered_set<PadType> types;
    std::unordered_set<FullPadID> pads;

    // For most pads, this will be `NULL_ID`, and will happily merge with
    // any other ID. Otherwise the pad is a connector (or the generation of this set
    // has come into contact with a connector by this point).
    //
    // Two non-null IDs must be equal to allow merge.
    UID connectorID{NULL_ID};

    [[nodiscard]] PadTypes operator&(const PadTypes& rhs) const;
    void operator&=(const PadTypes& other);

    [[nodiscard]] PadTypes operator|(const PadTypes& rhs) const;
    void operator|=(const PadTypes& other);

    [[nodiscard]] bool operator==(const PadTypes&) const = default;

    // Return any values in this set that are not present
    // in the `mask` set.
    //
    // If the mask has `pads.size() != 0`, then the return
    // is guaranteed to have `types.size() == 0`. (Only `pads` may have elements)
    // 
    // Alternatively, if the mask has `pads.size() == 0`, then the return
    // is guaranteed to have `pads.size() == 0`, and the `pads` are ignored. (Only processes `types`)
    //
    // Typical use is masking an emitter conglomerate with
    // an input set. If the returned `PadTypeSet` evaluates to true,
    // and there are leftovers, this set does not conform to the
    // requirements of the mask.
    [[nodiscard]] PadTypes mask(const PadTypes& mask) const;

    [[nodiscard]] bool isEmpty() const;

    // Returns std::nullopt if IDs cannot be mixed (e.g. both are unique and not NULL_ID)
    // The resultant UID otherwise.
    static std::optional<UID> mixIDs(const PadTypes& setA, const PadTypes& setB);
};

struct CONFIG_EXPORT PadTypesSet final {
    PadTypesSet() = default;
    // Will auto fill the `emit` connectorID (if applicable) and `pads` with self.
    PadTypesSet(const Pad&);

    // Combine the `emit` and `receive` sets from `other` with `this`.
    // Does so semantically, or'ing the emitters and and'ing the receivers
    void mix(const PadTypesSet& other);

    // Blind append of both `emit` and `receive` to be used when constructing types.
    void absorb(const PadTypesSet& other);

    [[nodiscard]] bool operator==(const PadTypesSet&) const = default;

    [[nodiscard]] static bool compatible(const PadTypesSet& setA, const PadTypesSet& setB);

    PadTypes emit;
    PadTypes receive;
};

struct CONFIG_EXPORT Wiring final {
    Wiring();
    Wiring(const Wiring&);
    Wiring& operator=(const Wiring&);

    Wiring(Wiring&&) = delete;
    Wiring& operator=(Wiring&&) = delete;

    // Returns true on success, care must be taken to only add a
    // component once!
    bool addComponent(std::shared_ptr<Component>);
    // Create component based on type
    std::shared_ptr<Component> addComponent(ComponentType);
    bool removeComponent(ComponentID);
    [[nodiscard]] vector<std::shared_ptr<const Component>> getComponents() const;
    [[nodiscard]] vector<std::shared_ptr<Component>> getComponents();
    [[nodiscard]] std::shared_ptr<const Component> getComponent(ComponentID) const;
    [[nodiscard]] std::shared_ptr<Component> getComponent(ComponentID);
    [[nodiscard]] vector<std::shared_ptr<const Component>> getComponentsByType(ComponentClass) const;
    [[nodiscard]] vector<std::shared_ptr<Component>> getComponentsByType(ComponentClass);

    // Returns nullptr if the connections are invalid. (e.g. both are disconnected.)
    // Returns nullptr if the connections cannot be added (types conflict or something of that nature)
    std::shared_ptr<Net> addNet(Connection, Connection);
    bool removeNet(UID);
    [[nodiscard]] vector<std::shared_ptr<const Net>> getNets() const;
    [[nodiscard]] vector<std::shared_ptr<Net>> getNets();
    [[nodiscard]] std::shared_ptr<const Net> getNet(UID) const;
    [[nodiscard]] std::shared_ptr<Net> getNet(UID);

    // The proffieboard is *always* defined within a `Wiring`, but the version used
    // can be changed and the ID gotten with these functions.
    bool setProffieboardVersion(ComponentClass pbVersion);
    [[nodiscard]] ComponentID getProffieboardID() const;

    [[nodiscard]] std::set<UID> getNetsConnectedToPad(FullPadID) const;

    // Scary, raw creation.
    [[nodiscard]] std::optional<NetSave> genNetSave(UID) const;
    void loadNetSave(const NetSave&);

private:
    // <Type, <UID, Component>>
    using ComponentMap = std::unordered_map<ComponentClass, Map<Component>>;
    // <UID, Net>
    using NetMap = std::unordered_map<UID, std::shared_ptr<Net>>;
    // <FullID, NetID>
    using PadNetLookupMap = std::unordered_multimap<FullPadID, UID>;

    friend class Component;
    friend class Net;
    ComponentMap mComponents;
    // Whenever a new `Net` is created or a `Net`'s `Connection` is altered,
    // these maps are updated so that querying a specific pad/wire will return all
    // `NetID`s (and thus `Net`s) connected to that pad/wire.
    PadNetLookupMap mPadNetLookup;
    NetMap mNets;

    ComponentID mProffieboardID;
};


// Expressed as "from left, to right".
//
// So by default orientation is left-right/pointing right. (pos x axis)
enum class Rotation {
    // 0 Degrees
    LEFT_RIGHT,
    // 90 Degrees
    DOWN_UP,
    // 180 Degrees
    RIGHT_LEFT,
    // 270 Degrees
    UP_DOWN,
};

enum class Dir {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct CONFIG_EXPORT Pad final {
    // Should only be used internally by component.
    //
    // The effect of an "exclusive" pad is dictated by emitted and received types for which only a single
    // specific connection are valid. It's effectively implicit by the emit/receive system.
    static void create(Component& parent, string name, bool required = false);

    // Human-readable name
    const string name;
    // This pad must be connected for wiring using parent component to be valid
    const bool required;

    [[nodiscard]] PadID getID() const;

    // Do a "shallow" calculation of the pad type, which is typically just the
    // base types, along with factoring in a deep calculation for any pads which 
    // are considered to be interconnected. (e.g. If you `calcTypes()` for a resistor
    // pad/leg, you'll get the simple type of the resistor leg combined by a deep calc of the
    // other leg, since they're connected)
    [[nodiscard]] PadTypesSet calcTypes() const;

    // A "deep" calculation of a pad type takes into account not only just interconnected
    // pads and base types, but also asks for a calculation from all connected nets to get
    // a complete idea of the effective current types.
    //
    // This (or a `Net`'s calculation) should be the entry point for type calc, and really
    // only called once for each type query.
    [[nodiscard]] PadTypesSet calcDeepTypes() const;

    [[nodiscard]] Point getPosition() const;

    // Calculates the direction that is ideal to move in starting from the component
    // to get outside/away from the component as fast as possible.
    [[nodiscard]] Dir calcDirectionBias() const;

    [[nodiscard]] const Component& getParent() const;

private:
    friend class Component;
    Pad(Component& parent, string name, bool required = false);

    // This gets called by `calcTypes()`, it is the Component's responsibility to set
    // a generator for each pad which returns it's base type, handles any dynamic changes if
    // applicable, etc., so that it conforms with the expectation of `calcTypes()`.
    using PadTypeGenerator = std::function<PadTypesSet(const Pad&)>;
    PadTypeGenerator mGenTypes;

    // May be moved by the parent component
    Point mPosition;

    PadID mID;
    Component *mParent{nullptr};
};

// Represents basically anything that can be wired to the proffieboard.
//
// Must be added to a `Wiring` parent before considered valid.
struct CONFIG_EXPORT Component : RequireClone<Component> {
    Component(ComponentClass cClass);
    Component(const Component&);

    Component& operator=(const Component&) = delete;
    Component(Component&&) = delete;
    Component& operator=(Component&&) = delete;

    [[nodiscard]] virtual string getHumanName() const = 0;

    [[nodiscard]] ComponentID getID() const;

    // Not const because this might require recalculation
    [[nodiscard]] wxBitmap getIcon();
    [[nodiscard]] wxBitmap getHighlight(const wxColour& = wxNullColour);

    // Get size in grid units
    // Not const, based on `getIcon()`
    [[nodiscard]] Point getSize();
    // Mainly for internal use
    [[nodiscard]] Point getUnorientedSize();

    [[nodiscard]] vector<std::shared_ptr<const Pad>> getPads() const;
    [[nodiscard]] vector<std::shared_ptr<Pad>> getPads();
    [[nodiscard]] std::shared_ptr<const Pad> getPad(PadID) const;
    [[nodiscard]] std::shared_ptr<Pad> getPad(PadID);
    [[nodiscard]] const Wiring &getParent() const;
    [[nodiscard]] Wiring &getParent();

    // Move component to `newPoint`, shifting wires accordingly
    void move(Point newPoint, bool pruneWires = true);
    void updatePadPositions();

    void pruneConnectedNets();

    [[nodiscard]] Point getPosition() const;

    // Does not break connections!!
    // Given this will surely break routed wires, they should be
    // purged and/or rerouted.
    //
    // This will however update pad positions accordingly.
    void setOrientation(Rotation);
    [[nodiscard]] Rotation getOrientation() const;

    // Convenience function
    [[nodiscard]] std::set<UID> getConnectedNets() const;

protected:
    // NOTE: the `typeGen` cannot capture `this` or anything else
    // by reference.
    bool setPadTypeGenerator(PadID, Pad::PadTypeGenerator typeGen);
    bool setPadPosition(PadID, Point);
    void invalidateIcon();

    // Retrieve the current icon to be used for this component. 
    // The component may choose to generate/draw the bitmap itself
    // (perhaps based on modes) or simply to load an external resource. Perhaps both.
    [[nodiscard]] virtual wxBitmap doGetIcon() const = 0;

    // Allow the component to update its pads' positions via custom implementation.
    virtual void doSetPadPositions() = 0;


private:
    friend class Pad;
    friend class Wiring;

    ComponentID mID;
    Wiring *mParent{nullptr};

    using PadMap = std::map<PadID, std::shared_ptr<Pad>>;
    PadMap mPads;
    Point mPosition;
    Rotation mOrientation{Rotation::LEFT_RIGHT};

    wxColour mHighlightColor{HIGHLIGHT_COLOR};
    wxBitmap mIcon;
    wxBitmap mHighlight;
};

// Describes a `Wiring` set of pathed wires.
//
// If the `Net` has `mWires.size() == 0` during `Wiring` initialization and `mConn` is fully connected, 
// it's considered as "pathing needed" and will be automatically routed.
//
// If `mWires.size() == 0` and `mConn` has one or both connections unconnected, the Net is invalid and should be
// destroyed at earliest convenience.
struct CONFIG_EXPORT Net final : Tracked {
    ~Net();

    // Orientation/direction of BEGIN wire.
    // Wires are in a direction perpendicular to their adjacent peers.
    enum class Direction {
        NONE,
        HORIZONTAL,
        VERTICAL,
    } startDir;
    // Flips a `Direction` such that if it is `HORIZONTAL` it becomes `VERTICAL`.
    // `NONE` remains `NONE`
    static void flipDir(Direction &);

    [[nodiscard]] Direction calcEndDir();

    // In the direction perpendicular to `dir`, the offset from 0
    // When combined with the BEGIN wire `start`, you get the "origin" x,y coord
    GridUnit offset;

    [[nodiscard]] Point getStartPos();
    [[nodiscard]] Point getEndPos();

    // Holds positions in the `dir` direction for where the wire starts and ends.
    // The next and previous wires start from the end or start of the current
    // respectively.
    struct Wire {
        GridUnit start;
        GridUnit end;
    };

    // Generate a list of all pads connected to this net (max 2, obviously)
    [[nodiscard]] std::unordered_set<FullPadID> calculateEndPoints() const;

    [[nodiscard]] PadTypesSet calculateEffectiveNetTypes() const;

    // First is start, second is end
    [[nodiscard]] std::pair<Connection, Connection> getConnections() const;

    // Sets the connection for the given end, performing any necessary relevant
    // functions.
    // WireN may be only `BEGIN` or `END`, otherwise return false.
    // This net and the connection types must be permissive, otherwise return false.
    // If both connections become set to `Disconnected` via this function, the Net is destroyed.
    // If connected to the `Disconnected` end of another `Net`, this net is destroyed and is appended to
    // the other.
    //
    // If connection is invalid return false.
    //
    // See `PadTypeSet` for more info
    bool setConnection(WireN, Connection);

    struct WireInfo {
        Wire wire;
        WireN idx;
        Direction dir;
    };
    // Get the information for a given wire, if it exists.
    [[nodiscard]] std::optional<WireInfo> getWireInfo(WireN);

    using WireMap = std::map<WireN, Wire>;
    [[nodiscard]] WireMap getWires() const;

    // Currently only used by `WiringEdit::Renderer`, info to "dumb"-load.
    struct WireArchive {
        GridUnit offset;
        Direction startDir;
        WireMap wires;
    };
    [[nodiscard]] WireArchive generateWireArchive() const;
    void loadWireArchive(const WireArchive&);

    [[nodiscard]] Wiring& getParent();

    // Create a new wire from either the `BEGIN` or `END` (any other WireN is invalid).
    //
    // The len can be negative, and extends from the previous wire in the perpendicular direction.
    // If `mWires.size() == 0` then mWires[0] will be created starting from the associated `Connection` (mStart or mEnd).
    //
    // A `Direction` can be specified, required when creating the first wire.
    // If `NONE` and `mWires.size() == 0`, the function returns false and does nothing.
    // If `Direction` equals the direction of the previous wire, `extendWire()` will be internally called instead.
    // If `Direction` equals `NONE` and there is a previous wire, the direction will alternate as is expected.
    //
    // If `mWires.size() == 0` and the specified start connection is `Disconnected`, the function returns false and does nothing.
    // Returns false if `mWires.size() != 0` and the specified start connection is connected.
    bool addWire(WireN, int32 len, Direction = Direction::NONE, bool ignoreZeroLength = true);

    // Extend the length of a wire along its direction.
    // Negative values shrink the wire.
    //
    // Return false if WireN DNE or is not `BEGIN`/`END`, (does not accept equivalent!!)
    // (e.g. if wires [0-2] are defined, both `BEGIN` and `0` would be valid and equivalent)
    bool extendWire(WireN, int32 dist, bool prune = true);

    // Move a wire in the perpendicular direction, extending/shrinking the two adjacent
    // wires accordingly.
    //
    // Returns false if WireN is out of range
    bool moveWire(WireN, int32 dist, bool prune = true);

    // Deletes the specified wire, assuming it's valid. If WireN is not `BEGIN` or `END` or equivalent,
    // the `Net` is split. The upper range of this net is destroyed and an identical (albeit partial) net is created.
    //
    // A pointer to a NetID object can be supplied, and it will be filled with the new net ID, if there was one,
    // this net's ID if no split ocurred, and NULL_ID if the net was destroyed.
    bool deleteWire(WireN, UID *newID = nullptr);

    // Runs through all wires and checks if there are any with zero length.
    // If there are, wires are removed, joined, etc., to prune the wire chain.
    //
    // Indexes are updated/invalidated if necessary.
    //
    // A `bool` can be provided, and if it is, it will be set to true if the pruning
    // resulted in the destruction of this net. (false otherwise)
    //
    // A `WireN` can be provided, and if it is, it will "track" a wire through
    // prune such that if its index changes, the updated index will be returned through
    // the same variable.
    void pruneWires(bool *destroyed = nullptr, WireN * = nullptr);

private:
    friend class Wiring;
    Net(Wiring& parent, UID);
    // Essentially used as a deque, but with static indexes.
    // Every new wire chains from the previous, and is in the alternate direction.
    // (e.g. If dir == HORIZONTAL then wires[0] is horizontal. Then wires[-1] and wires[1] are VERTICAL, if they exist.)
    WireMap mWires;

    // Start is what the wire closest to BEGIN is connected to.
    // End is what the wire closest to END is connected to.
    // For lookup start vs. end really doesn't matter...
    // Whenever looking them up and setting them it'll just be whichever
    // first matches, if any.
    Connection mStartConn;
    Connection mEndConn;

    // Called during `setConnection()` if applicable.
    void addConnectionToLookup(Connection);
    void removeConnectionFromLookup(Connection);

    Wiring *mParent{nullptr};
};

struct NetSave {
    std::pair<Connection, Connection> connections;
    Net::WireArchive wires;

    UID netID{NULL_ID};
};


} // namespace Config::Wiring


